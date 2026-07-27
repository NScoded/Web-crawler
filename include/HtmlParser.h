#pragma once

#include "STL/DynamicArray.h"
#include <string>


class HtmlParser{
    private:
    
    DynamicArray<std::string>links;
    public:
    size_t parseHttp(const std::string &html, size_t start);
    size_t parseHref(const std::string &html, size_t start);
    DynamicArray<std::string>parseLinks(const std::string &html);
    std::string parseContent(const std::string& html);

};
