#include<bits/stdc++.h>
#include "include/Set.h"
using namespace std;

int main(){
  string s="Hello my name is rohan how are you";
  Set<string>set;
  string search="Hello ";
  stringstream ss(s);
  int mismatch,before,after;
  string word;
  int wordsize=0;
  while(ss>>word){
    wordsize++;
    set.insert(word);
  }
  before=set.size();
  cout<<"before: "<<before<<endl;
  stringstream sw(search);
  int searchword=0;
  while(sw>>word){
    searchword++;
    set.insert(word);
  }
  after=set.size();
  cout<<"after: "<<after<<endl;
  mismatch=after-before;
  cout<<(searchword-mismatch)*100/wordsize;



  

}