#include"../../include/STL/LinkedList.h"
#include"../../include/STL/LinkedList.h"
#include"../../include/STL/MyHash.h"
#include"../../include/STL/HashMap.h"

//
// Pair Constructor
//

template<typename K, typename V>
HashMap<K,V>::Pair::Pair(K k, V v){
    key=k;
    value=v;
}
    
//
// HashMap Constructor
//
template<typename K, typename V>
HashMap<K,V>::HashMap(int capacity){
    bucketCount=capacity;
    elementCount=0;
    for(int i=0;i<bucketCount;i++){
        buckets.push_back(LinkedList<Pair>());
    }
}

//
// Hash Function
//
template<typename K, typename V>
size_t HashMap<K,V>::hash(const K& key) const {

    MyHash<K> hasher;
    return hasher(key) % bucketCount;
}

//
// Insert
//
template<typename K, typename V>
void HashMap<K,V>::insert(const K& key, const V& value)
{
    size_t index = hash(key);

    LinkedList<Pair>& bucket = buckets[index];

    typename LinkedList<Pair>::Node* curr = bucket.head;

    while(curr)
    {
        // operator == should be defined for key type
        if(curr->data.key == key)
        {
            curr->data.value = value;
            return;
        }

        curr = curr->next;
    }
    // same key nahi mili to last me new pair append kardo
    bucket.append(Pair(key, value));
    elementCount++;


    if(loadFactor() > 0.75f)
        rehash();
}

//
// Exists
//
template<typename K, typename V>
bool HashMap<K,V>::exists(const K& key) const
{
    size_t index = hash(key);

    const LinkedList<Pair>& bucket = buckets[index];

    typename LinkedList<Pair>::Node* curr = bucket.head;

    while(curr)
    {
        if(curr->data.key == key)
            return true;

        curr = curr->next;
    }

    return false;
}

//
// Get
//
template<typename K, typename V>
V& HashMap<K,V>::get(const K& key)
{
    size_t index = hash(key);

    LinkedList<Pair>& bucket = buckets[index];

    typename LinkedList<Pair>::Node* curr = bucket.head;

    while(curr)
    {
        if(curr->data.key == key)
            return curr->data.value;

        curr = curr->next;
    }

    throw std::runtime_error("Key not found");
}

//
// Remove
//
template<typename K, typename V>
bool HashMap<K,V>::remove(const K& key)
{
    size_t index = hash(key);

    LinkedList<Pair>& bucket = buckets[index];

    typename LinkedList<Pair>::Node* curr = bucket.head;

    while(curr)
    {
        if(curr->data.key == key)
        {
            bucket.remove(curr->data);
            elementCount--;
            return true;
        }

        curr = curr->next;
    }

    return false;
}

//
// Size
//
template<typename K, typename V>
int HashMap<K,V>::size() const
{
    return elementCount;
}

//
// Capacity
//
template<typename K, typename V>
int HashMap<K,V>::capacity() const
{
    return bucketCount;
}

//
// Load Factor
//
template<typename K, typename V>
float HashMap<K,V>::loadFactor() const
{
    return static_cast<float>(elementCount) / bucketCount;
}


//
// Clear
//
template<typename K, typename V>
void HashMap<K,V>::clear()
{
    for(int i = 0; i < bucketCount; i++)
    {
        buckets[i].clear();
    }

    elementCount = 0;
}

//
// Rehash
//
template<typename K, typename V>
void HashMap<K,V>::rehash()
{
    DynamicArray<LinkedList<Pair>> oldBuckets = buckets;

    int oldBucketCount = bucketCount;

    bucketCount *= 2;

    buckets.clear();

    for(int i = 0; i < bucketCount; i++)
    {
        buckets.push_back(LinkedList<Pair>());
    }

    elementCount = 0;

    for(int i = 0; i < oldBucketCount; i++)
    {
        typename LinkedList<Pair>::Node* curr =
            oldBuckets[i].head;

        while(curr)
        {
            insert(
                curr->data.key,
                curr->data.value
            );

            curr = curr->next;
        }
    }
}