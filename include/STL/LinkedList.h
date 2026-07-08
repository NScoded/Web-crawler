#pragma once

template<typename T>class LinkedList{

    template<typename>
    friend struct MyHash;

    template<typename,typename>
    friend class HashMap;
    private:
    struct Node{
        T data;
        Node*next;
        Node(T val);
        
    };
    
    
    public:
    
    // 10 methods
    Node*head=nullptr;
    Node*tail=nullptr;
    int size;
    LinkedList(); //Constructor
    ~LinkedList(); // Destructor
    LinkedList(const LinkedList& other); // Copy Constructor
    LinkedList& operator=(const LinkedList& other); // Handling assignment operator
    bool operator==(const LinkedList<T>& other) const;
    void insertHead(T value); //Insert value at front
    void remove(int index); // Delete value by index
    void insert(int pos, T value); //Insert value at given position
    void append(T value); // Add element at last
    void pop();// Delete last element
    void reverse(); // Reverse the LinkedList
    bool exists(T value); //returns true of value exists otherwise returns false 
    int length(); //return size of the linkedlist
    void clear();// Delete all elements


};

#include "../../src/LinkedList/LinkedList.tpp"
