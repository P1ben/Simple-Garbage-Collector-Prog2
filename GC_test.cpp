#include <iostream>
#include <sstream>

#include "garbage_collector.h"


/// Két egészt hasonlít össze.
/// @param label A teszt címeként kiírandó szöveg.
/// @param expexted A várt érték.
/// @param got A kapott érték.
void test_value(const char* label, int expected, int got){
    std::cout << label << std::endl;
    std::cout << "  Expected: " << expected << std::endl;
    std::cout << "  Got: " << got << std::endl;
    if(expected == got)
        std::cout << "SUCCESS\n";
    else
    {
        std::cout << "FAILED\n";
    }
    std::cout << std::endl;
}

/// Int típusú változót tároló láncolt lista elem.
struct Node{
    int value;
    SPointer<Node> next;

    Node(int inp = 0) : value(inp), next(nullptr){};
};

/// Hozzáad egy elemet a láncolt listához.
/// @param begin A láncolt lista első eleme.
/// @param inp A hozzáadandó változó.
void AppendNode(SPointer<Node> begin, int inp = 0){
    //std::cout << inp << ": " << begin << "\n";
    while(begin->next != nullptr){
        begin = begin->next;
    }
    begin->next = new Node(inp);
}

/// Kitörli az utolsó elemet a láncolt listából.
/// @param begin A láncolt lista első eleme.
void DeleteNode(SPointer<Node> begin){
    SPointer<Node> before = begin;
    while(begin->next != nullptr){
        before = begin;
        begin = begin->next;
    }
    before->next = nullptr;
}

/// Sorban kiírja a láncolt lista elemeit.
/// @param begin A láncolt lista első eleme.
void NodeKiir(SPointer<Node> begin){
    while(begin->next != nullptr){
        std::cout << begin->value << " -> ";
        begin = begin->next;
    }
    std::cout << begin->value << "\n";
}

/// Értékadás teszt
void Test1(){
    std::cout << "--- ERTEKADAS TESZT ---\n";
    {
        const SPointer<int> a = new int;
        *a = 10;

        test_value("-*a erteke ertekadas utan-", 10, *a);

        SPointer<int> b = a;
        test_value("-*b erteke ertekadas utan-", 10, *b);

    }
    std::cout << "--- ERTEKADAS TESZT VEGE ---\n\n";
}

/// Mátrix teszt
void Test2(){
    std::cout << "--- MATRIX TESZT ---\n";
    {
        SPointer<SPointer<int[]>[]> matrix = new SPointer<int[]>[10];
        for(int i = 0; i < 10; i++){
            matrix[i] = new int[10];
        }
        for(int i = 0; i < 10; i++){
            for(int j = 0; j < 10; j++){
                matrix[i][j] = i*10 + j;
            }
        }
        for(int i = 0; i < 10; i++){
            for(int j = 0; j < 10; j++){
                if(i < 1) std::cout << "[0" <<  matrix[i][j] << "]";
                else std::cout << "[" <<  matrix[i][j] << "]";
            }
            std::cout << "\n";
        }
        test_value("\n-matrix[2][3] erteke-", 23, matrix[2][3]);
        test_value("-matrix[9][9] erteke-", 99, matrix[9][9]);

    }
    std::cout << "--- MATRIX TESZT VEGE ---\n\n";
}

/// Láncolt lista teszt
void Test3(){
    std::cout << "--- LINKEDLIST TESZT ---\n";
    {
        std::cout << "Lancolt lista letrehozasa 5 elemmel:\n";
        SPointer<Node> first = new Node(0);
        for(int i = 1; i < 5; i++)
            AppendNode(first, i);

        NodeKiir(first);
        test_value("\n-Lancolt lista 2. elemenek erteke-", 1, first->next->value);

        std::cout <<"\n\n2 db elem torlese:\n";

        DeleteNode(first);
        DeleteNode(first);


        NodeKiir(first);
        test_value("\n-Lancolt lista 2. elemenek erteke torles utan-", 1, first->next->value);
    }
    std::cout << "--- LINKEDLIST TESZT VEGE ---\n\n";
}
int main()
{
    char choice = 'a';
    while (choice != '4')
    {
        std::cout << "Melyik tesztet szeretned futtatni? (1 - 4)\n";
        std::cout << "  1: Ertekadas teszt\n";
        std::cout << "  2: Lancolt lista teszt\n";
        std::cout << "  3: Matrix teszt\n\n";
        std::cout << "  4: Kilepes\n\n> ";
        std::cin >> choice;

        switch (choice)
        {
        case '1':
            Test1();
            break;
        case '2':
            Test3();
            break;
        case '3':
            Test2();
            break;
        }
        std::cout << std::endl;
    }
    return 0;
}
