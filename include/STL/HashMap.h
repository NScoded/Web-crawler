#pragma once

#include "DynamicArray.h"
#include "LinkedList.h"
#include "MyHash.h"


#include <stdexcept>

template<typename K, typename V>
class HashMap {

    private:
    struct Pair {
        K key;
        V value;
        Pair(K k, V v);
        bool operator==(const Pair& other) const {
            return key == other.key;
        }
    };
    DynamicArray<LinkedList<Pair>> buckets;
    int bucketCount;
    int elementCount;
    size_t hash(const K& key) const;
    void rehash();

    public:
    HashMap(int capacity = 16);
    void insert(const K& key, const V& value);
    bool remove(const K& key);
    bool exists(const K& key) const;
    V& get(const K& key);
    int size() const;
    int capacity() const;
    float loadFactor() const;
    void clear();
};

#include "../../src/HashMap/HashMap.tpp"