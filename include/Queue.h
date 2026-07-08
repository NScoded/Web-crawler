# pragma once

#include<string>
#include "STL/LinkedList.h"
using namespace std;

template<typename T>
class Queue{

    private:
    LinkedList<T>queue;
    public:
    
    T pop();
    int size();
    T front();
    bool empty();
    void push(T val);
    

};

#include"../src/Queue.tpp"