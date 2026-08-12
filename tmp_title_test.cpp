#include <iostream>
#include <string>
#include "include/HtmlParser.h"
int main(){
    std::string html = "<html><head><title>AWS Academy | Training and Certification | AWS</title></head><body><div>Some content</div></body></html>";
    std::string title;
    HtmlParser parser;
    parser.parseLinks(html, title);
    std::cout << title << std::endl;
    return 0;
}
