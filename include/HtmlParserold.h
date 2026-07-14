#include "STL/DynamicArray.h"
#include<string>
using namespace std;

class HtmlParser{
    private:
    DynamicArray<string>links;
    public:
    size_t parseHttp(string &part,size_t index);
    size_t parseHref(string &part,size_t index);
    DynamicArray<string>parseHtml(string html);

};

