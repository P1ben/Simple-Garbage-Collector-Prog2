#ifndef GARBAGE_COLLECTOR_H
#define GARBAGE_COLLECTOR_H

#include <iostream>

// Ha true az értéke, akkor a garbage collector műveletei kiírásra kerülnek.
#define DEBUG_MODE false


/// Absztrakt osztály, lehetővé teszi a különböző ReferenceCount objektumokból
/// heterogén kollekció létrehozását. Az összes tagfüggvénye virtuális.
class ReferenceCounter{
public:
    virtual const void* GetPointer() const = 0;
    virtual size_t GetReferenceCount() const = 0;
    virtual void operator ++ (int) = 0;
    virtual void operator -- (int) = 0;
    virtual ~ReferenceCounter(){};
};

/// Referencia számláló osztály. Feladata egy adott pointerre mutató referenciák számának nyilvántartása.
template<class T>
class ReferenceCount : public ReferenceCounter{
private:

    /// A tárolt pointer.
    T* pointer;

    /// Tömbre, vagy változóra mutat?
    bool isArray;

    /// A pointerre mutató referenciák száma.
    size_t reference_count;

public:
    /// Konstruktor
    /// Létrehoz egy új ReferenceCount objektumot.
    /// @param p A tárolandó pointer. (default: nullptr)
    /// @param isArr Tömbre, vagy változóra mutat az adott pointer? (default: false)
    ReferenceCount(T* p = nullptr, bool isArr = false) : pointer(p), isArray(isArr), reference_count(1){};

    /// Getter függvény a referenciák számának lekérdezéséhez.
    /// @return size_t A tárolt pointerre mutató referenciák száma.
    size_t GetReferenceCount() const override {return this->reference_count;};

    /// Getter függvény a tárolt pointer lekérdezéséhez.
    /// @return const void* A tárolt pointer.
    const void* GetPointer() const override {return this->pointer;};

    /// Hozzáad egyet a referenciák számához.
    void operator ++ (int) override {this->reference_count++;};

    /// Elvesz egyet a referenciák számából.
    void operator -- (int) override {this->reference_count--;};

    /// Destruktor
    /// Törli az objektumot, illetve felszabadítja a tárolt pointer által mutatott memóriaterületet.
    ~ReferenceCount(){
        if(this->isArray){
            delete[] pointer;
            if(DEBUG_MODE) std::cout << "|| Array destroyed | Pointer: " << pointer <<"\n";
        }
        else
        {
            delete   pointer;
            if(DEBUG_MODE) std::cout << "|| Variable destroyed | Pointer: " << pointer << "\n";
        }
    }
};

/// Magát a szemétgyűjtőt tartalmazó osztály. Az osztály minden változója és tagfüggvénye statikus.
/// Függvényeit csak az okos pointerek (SPointer) érik el.
class GarbageCollector{
private:

    /// A létrehozott ReferenceCount objektumokat tartalmazó heterogén kollekció.
    static ReferenceCounter** array;

    /// A heterogén kollekció mérete.
    static size_t count;

    /// Lefuttat egy keresést a ReferenceCount objektumokat tartalmazó heterogén kollekcióban.
    /// @param pointer A keresett pointer.
    /// @return A pointert tartalmazó ReferenceCounter objektumra mutató pointer. Amennyiben a keresett pointer nem található a függvény visszatérési értéke nullptr.
    static ReferenceCounter* SearchArray(const void* pointer);

    /// Bővíti a heterogén kollekciót, és hozzáad egy új elemet.
    /// @param ref Az eltárolandó pointer.
    /// @param isArray Tömbre, vagy változóra mutat a pointer?
    /// @exception const char* Az új objektumnak szükséges memóriaterület lefoglalása sikertelen. (A pointer által mutatott memóriaterület felszabadításra kerül.)
    template<typename T>
    static void Append(T* ref, bool isArray){
        ReferenceCounter** uj;
        try{
            uj = new ReferenceCounter*[count + 1];
        } catch(...) {
            delete ref;
            throw "GC: Hiba tortent a tomb novelesekor. A memoriaterulet, melyre a pointer mutatott felszabaditasra kerult.\n";
        }
        for(size_t i = 0; i < count; i++)
            uj[i] = array[i];

        try{
            uj[count++] = new ReferenceCount<T>(ref, isArray);
        } catch(...) {
            delete[] uj;
            delete ref;
            throw "GC: Hiba tortent az uj ReferenceCount objektum letrehozasakor. A memoriaterulet, melyre a pointer mutatott felszabaditasra kerult.\n";
        }
        delete[] array;
        array = uj;
    }

    /// Megkeresi a tömbben az adott pointerhez tartozó ReferenceCount objektumot, és hozzáad egyet a referenciák számához.
    /// Amennyiben a pointer nem volt még eltárolva a tömbben, hozzáadja a tömbhöz új elemként.
    /// @param ref A pointer, aminek a referenciáinak számához hozzá szeretnénk adni.
    /// @param isArray A megadott pointer tömbre, vagy változóra mutat-e.
    template<class T>
    static void Add(T* ref, bool isArray){
        if(ref == nullptr) return;
        //std::cout << "GC count: " << count << "\n";
        ReferenceCounter* res = SearchArray((void*)ref);
        if(res == nullptr)
            Append(ref, isArray);
        else
        {
            (*res)++;
            if(DEBUG_MODE) std::cout << "|| Referenciak szama novelve:    Pointer: " << res->GetPointer() << " Value: (" << res->GetReferenceCount() - 1 << " -> " << res->GetReferenceCount() << ")\n";
        }
    }

    /// Kitöröl a referenciák számát nyilvántartó tömbből egy elemet.
    /// Amennyiben a törlés után a tömb hossza 0 lesz, felszabadítja a tömböt.
    /// @param ref A tömbből kitörlendő elemre mutató pointer.
    static void Remove(ReferenceCounter* ref);

    /// Megkeresi a tömbben az adott pointerhez tartozó ReferenceCount objektumot, és kivon egyet a referenciák számából.
    /// Amennyiben a referenciák száma 0-ra csökken, kitőrli a tömbből az objektumot.
    /// @param ref A pointer, aminek a referenciáinak számához hozzá szeretnénk adni.
    /// @param isArray A megadott pointer tömbre, vagy változóra mutat-e.
    static void Subtract(const void* ref);


    template<class T>
    friend class SPointer;
};

/// Okos pointer, mely eltárol egy dinamikusan foglalt változóra mutató pointert.
/// Amennyiben a tárolt pointerre nincs több referencia, felszabadítja.
template <class T>
class SPointer
{
private:
    T *pointer;
    bool isArray;
public:
    /// Konstruktor
    /// Alapértékekre állítja a létrehozott objektum belső változóit.
    SPointer() : pointer(nullptr), isArray(false){};

    /// Konstruktor
    /// Egy megadott pointerre állítja a tárolt pointert.
    /// @param p A tárolandó pointer.
    SPointer(T *p) : pointer(p), isArray(false){
        if(DEBUG_MODE) std::cout << "|| Variable created | Pointer: " << p << "\n";
        GarbageCollector::Add(pointer, isArray);
    };

    /// Másoló konstruktor
    SPointer(const SPointer& cop) : pointer(cop.pointer), isArray(cop.isArray){
        if(DEBUG_MODE) std::cout << "|| Variable copied | Pointer: " << pointer << "\n";
        GarbageCollector::Add(pointer, isArray);
    };

    /// Értékadó operátor
    /// A régi pointerre mutató referenciák számát csökkenti.
    /// Az új pointerre mutató referenciák számát növeli.
    /// @param p A tárolandó pointert tartalmazó okos pointer objektum.
    SPointer& operator = (SPointer& p) {
        if(DEBUG_MODE) std::cout << "|| Address copy: deleted: " << pointer << " New: " << p << "\n";
        GarbageCollector::Subtract(pointer);
        pointer = p;
        GarbageCollector::Add(p.pointer, isArray);
        return *this;
    }

    /// Értékadó operátor
    /// A régi pointerre mutató referenciák számát csökkenti.
    /// Az új pointerre mutató referenciák számát növeli.
    /// @param p A tárolandó pointer.
    SPointer& operator = (T* p) {
        if(DEBUG_MODE) std::cout << "|| Address copy: deleted: " << pointer << " New: " << p << "\n";
        GarbageCollector::Subtract(pointer);
        pointer = p;
        GarbageCollector::Add(p, isArray);
        return *this;
    }

    /// Kompatibilitást garantáló operátor
    operator T* () {return pointer;};

    /// Kompatibilitást garantáló operátor
    operator T* () const {return pointer;};

    /// Nyíl operátor
    T* operator -> (){return pointer;};

    /// Destruktor
    /// Csökkenti a tárolt pointerre mutató referenciák számát.
    ~SPointer() {GarbageCollector::Subtract(pointer);};
};

/// Okos pointer, mely eltárol egy dinamikusan foglalt tömbre mutató pointert.
/// Amennyiben a tárolt pointerre nincs több referencia, felszabadítja.
template <class T>
class SPointer<T[]>
{
private:
    T* pointer;
    bool isArray;
public:

    /// Konstruktor
    /// Alapértékekre állítja a létrehozott objektum belső változóit.
    SPointer() : pointer(nullptr), isArray(true){};

    /// Konstruktor
    /// Egy megadott pointerre állítja a tárolt pointert.
    /// @param p A tárolandó pointer.
    SPointer(T *p) : pointer(p), isArray(true){
        if(DEBUG_MODE) std::cout << "|| Array created | Pointer: " << pointer << "\n";
        GarbageCollector::Add(pointer, isArray);
    };

    /// Másoló konstruktor
    SPointer(const SPointer<T>& cop) : pointer(cop.pointer), isArray(cop.isArray){
        GarbageCollector::Add(pointer, isArray);
    };

    /// Értékadó operátor
    /// A régi pointerre mutató referenciák számát csökkenti.
    /// Az új pointerre mutató referenciák számát növeli.
    /// @param p A tárolandó pointert tartalmazó okos pointer objektum.
    SPointer& operator = (SPointer& p){
        if(DEBUG_MODE) std::cout << "|| Address copy: deleted: " << pointer << " New: " << p << "\n";
        GarbageCollector::Subtract(pointer);
        pointer = p;
        GarbageCollector::Add(p.pointer, isArray);
        return *this;
    }

    /// Értékadó operátor
    /// A régi pointerre mutató referenciák számát csökkenti.
    /// Az új pointerre mutató referenciák számát növeli.
    /// @param p A tárolandó pointer.
    SPointer& operator = (T* p){
        if(DEBUG_MODE) std::cout << "|| Address copy: deleted: " << pointer << " New: " << p << "\n";
        GarbageCollector::Subtract(pointer);
        pointer = p;
        GarbageCollector::Add(p, isArray);
        return *this;
    }

    /// Kompatibilitást garantáló operátor
    operator T* () {return pointer;};

    /// Kompatibilitást garantáló operátor
    operator T* () const {return pointer;};

    /// Nyíl operátor
    T* operator -> (){return pointer;};

    /// Destruktor
    /// Csökkenti a tárolt pointerre mutató referenciák számát.
    ~SPointer() {GarbageCollector::Subtract(pointer);};
};

#endif
