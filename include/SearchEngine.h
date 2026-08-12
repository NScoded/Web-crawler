#pragma once
#include "PageStorage.h"
#include "Indexer.h"
#include "STL/DynamicArray.h"
#include <string>

class SearchEngine{
    private:
    PageStorage page;
    Indexer indexer;

    public:

    //DynamicArray<DynamicArray<std::string>>keyword(DynamicArray<std::string> &word);
    DynamicArray<DynamicArray<std::string>>keyword(std::string &word);
    


};