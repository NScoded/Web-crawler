#include "../include/STL/LinkedList.h"
#include <iostream>



//Node Constructor
template<typename T>
LinkedList<T>::Node::Node(T val)
    : data(val), next(nullptr)
{
}


// LinkedList Constructor
template<typename T> LinkedList<T> :: LinkedList(){
    head=nullptr;
    tail=nullptr;
    size=0;
}

// LinkedList Copy Conststructor
template<typename T>LinkedList<T> :: LinkedList(const LinkedList&others){
    head=nullptr;
    tail=nullptr;
    size=0;
    Node*cur=others.head;
    
    while(cur){
        this->append(cur->data);
        cur=cur->next;
    }
}

// Copy Assignement Constructor
template<typename T>
LinkedList<T>& LinkedList<T>::operator=(const LinkedList& others){

    if(this == &others){
        return *this;
    }

    clear();

    Node* temp = others.head;

    while(temp){
        append(temp->data);
        temp = temp->next;
    }

    return *this;
}



// LinkedList Destructor
template<typename T> LinkedList<T> :: ~LinkedList(){
    Node* temp=head;
    while(head){
        head=head->next;
        delete temp;
        temp=head;
    }
    head=nullptr;
    tail=nullptr;
    size=0;
};


// operator ==
template<typename T>
bool LinkedList<T>::operator==(const LinkedList<T>& other) const {

    if(size != other.size)
        return false;

    Node* a = head;
    Node* b = other.head;

    while(a && b) {

        if(a->data != b->data)
            return false;

        a = a->next;
        b = b->next;
    }

    return true;
}

// Append
template<typename T>void 
LinkedList<T> :: append(T val){
    Node* node = new Node(val);
        if(head==nullptr){
            head=node;
            tail=node;
        }
        else{
            tail->next=node;
            tail=node;
        }
        size++;

};

// pop()
template<typename T> void 
LinkedList<T> :: pop(){
    if(size==0){
        throw std::underflow_error("Linkedlist is empty");
    }
    Node*temp=head;
    size--;
    if(head->next==nullptr){
        head=nullptr;
        tail=nullptr;
        delete temp;
        return;
    }

    while(temp->next!=tail){
        temp=temp->next;
    }
    temp->next=nullptr;
    delete tail;
    tail=temp;

};

//InsertHead
template <typename T> void 
LinkedList<T> :: insertHead(T val){
    Node*node=new Node(val);
    size++;
    if(head==nullptr){
        head=node;
        tail=head;
        return;
    }
    node->next=head;
    head=node;
};

// insert at random pos
template<typename T> 
// pos between 1 to size
void LinkedList<T> :: insert(int pos,T val){

    // pos>size || pos<1
    if(size+1<pos || pos<0){ 
        throw std::out_of_range("Index out of range");
    }
    // Last insertion
    if(size==pos){
        append(val);
        return;
    }
    
    // Insert at head (or Head is nullptr)
    if(pos==0 ){
        insertHead(val);
        return;
    }

    // Insert in Between Head and Tail
    Node * node= new Node(val);
    size++;
    Node*prev=head;
    for(int i=0;i<pos-1;i++){
        prev=prev->next;
    }
    node->next=prev->next;
    prev->next=node;

};



//length()
template<typename T>
int LinkedList<T> :: length(){
    return size;
}


//exists()
template<typename T> bool
LinkedList<T>::exists(T val){
    Node*temp=head;
    while(temp){
        if(temp->data==val)return true;
        temp=temp->next;
    }
    return false;
}

//reverse()
template <typename T> void
LinkedList<T>::reverse(){

    Node* prev=nullptr;
    Node* cur=head;
    tail=head;

    while(cur){
        Node*next=cur->next;
        cur->next=prev;
        prev=cur;
        cur=next;
    }
    head=prev;

    

}

// remove by value
template<typename T>void
LinkedList<T>::remove(int index){
    if(index>=size || index<0) throw std::out_of_range("Out of Range");

    if(index==size-1){
        pop();
        return ;

    }
    
    if(index==0){
        Node *temp=head;
        head=head->next;
        if(head==nullptr){
            tail=nullptr;
        }
        delete temp;
        size--;
        return;
    }
    Node*prev=head;
    Node*nxt=head;
    int i=0;
    
    while(i<index){
        prev=nxt;
        nxt=nxt->next;
        i++;
    }

    if(nxt==nullptr){
        std::cout<<"Data not Found!";
        return;
    }
    
    size--;
    
    prev->next=nxt->next;
    delete nxt;
    
}


// clear
template<typename T> void LinkedList<T>:: clear(){
    
    while(head){
        Node*temp=head;
        head=head->next;
        delete temp;
    }
    head=nullptr;
    tail=nullptr;
    size=0;
}

