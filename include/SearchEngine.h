#pragma once
#include "PageStorage.h"
#include "DynamicArray.h"
#include <string>

class SearchEngine{
    private:    
    DynamicArray<DynamicArray<std::string>>keyword(DynamicArray<std::string> &word);
    DynamicArray<DynamicArray<std::string>>keyword(std::string &word);
    


};