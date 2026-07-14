#include "STL/DynamicArray.h"
#include<string>
using namespace std;

class HtmlParser{
    private:
    DynamicArray<string>links;
    public:
    size_t parseHttp(const string &html, size_t start);
    size_t parseHref(const string &html, size_t start);
    DynamicArray<string>parseHtml(const string &html);

};
