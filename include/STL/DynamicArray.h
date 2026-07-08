#pragma once

template<typename T> class DynamicArray{

    private:
    void resize();
    void destroyAndFree(T * arr,int count);
    
    public:
    int len;
    int cap;
    T * arr ;
    DynamicArray();// Constructor
    DynamicArray(int cap,T val);// Constructor with capacity and value
    DynamicArray(const DynamicArray& other); //Copy Constructor
    DynamicArray& operator=(const DynamicArray& other); //Copy Assignment Constructor
    T& operator [](int index); // operator [] with =
    bool operator==(const DynamicArray &others)const;
    const T& operator [](int index) const; // const operator []
    ~DynamicArray(); //Destructor
    void push_back(const T& value); //Add value at last
    void insert(int index, const T& value); //Add value at given index
    void remove(int index);// Delete value by index
    void pop_back(); // Delete last element
    int size() const; // return number of elements inserted
    int capacity()const;// returns total capacity of array
    void clear(); // remove all elements
};

#include "../../src/DynamicArray/DynamicArray.tpp"