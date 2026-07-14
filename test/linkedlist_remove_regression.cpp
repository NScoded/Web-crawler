#include "../include/STL/LinkedList.h"
#include <iostream>

int main() {
    LinkedList<int> list;
    list.append(10);
    list.append(20);
    list.append(30);

    list.remove(0);

    if (list.length() != 2) {
        std::cerr << "length mismatch: expected 2, got " << list.length() << "\n";
        return 1;
    }

    if (list.head == nullptr || list.head->data != 20) {
        std::cerr << "head pointer corrupted after remove(0)\n";
        return 2;
    }

    std::cout << "regression test passed\n";
    return 0;
}
