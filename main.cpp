#include"include/Queue.h"

using namespace std;


class stud{
    int n;

    public:
    bool operator==(const stud &other ){
        return this->n==other.n;
    }
};
int main(){
    Queue<int>queue;
    queue.push(1);
    queue.push(2);
    queue.push(3);
    
}