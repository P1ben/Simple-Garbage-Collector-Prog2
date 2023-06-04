#include "garbage_collector.h"
#include "memtrace.h"

ReferenceCounter** GarbageCollector::array = NULL;
size_t GarbageCollector::count = 0;

ReferenceCounter* GarbageCollector::SearchArray(const void* pointer){
    for(size_t i = 0; i < count; i++)
        if((*array[i]).GetPointer() == pointer)
            return array[i];
    return nullptr;
}

void GarbageCollector::Remove(ReferenceCounter* ref){
    delete ref;
    if(count == 1){
        delete[] array;
        array = NULL;
        count = 0;
        return;
    }
    ReferenceCounter** uj = new ReferenceCounter*[count - 1];
    for(size_t i = 0, j = 0; i < count; i++)
        if(array[i] != ref)
            uj[j++] = array[i];
    delete[] array;
    array = uj;
    count--;
}

void GarbageCollector::Subtract(const void* ref){
    if(ref == nullptr) return;
    ReferenceCounter* res =  SearchArray(ref);
    if(res == nullptr)
        throw "GC: Pointer nem talalhato!";
    else
    {
        if(DEBUG_MODE) std::cout << "|| Referenciak szama csokkentve: Pointer: " << res->GetPointer() << " Value: (" << res->GetReferenceCount() << " -> " << res->GetReferenceCount() - 1 << ")\n";
        //std::cout << "Reference count: " << res->GetReferenceCount() << "\n";
        if(res->GetReferenceCount() > 1)
            (*res)--;
        else
            Remove(res);
    }
}
