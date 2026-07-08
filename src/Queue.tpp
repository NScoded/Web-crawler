#include "../include/Queue.h"

// Push()
template<typename T>
void Queue<T>::push(T val){
    queue.append(val);
}

// Pop()
template<typename T>
T Queue<T>::pop(){
    if(queue.length()<1) throw std::out_of_range("Queue is already empty");
    T val=queue.head->data;
    queue.remove(0);
    return val;

}

// size()
template<typename T>
int Queue<T>::size(){
    
    return queue.length();
}

// empty()
template<typename T>
bool Queue<T>::empty(){
    if(queue.length()==0)return true;
    return false;
}


// front()
template<typename T>
T  Queue<T>::front(){
    if(empty()){
        throw std::out_of_range("Queue is empty");
    }
    return queue.head->data;
}