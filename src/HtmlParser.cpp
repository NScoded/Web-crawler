#include"../include/HtmlParser.h"
#include<iostream>
using namespace std;


size_t HtmlParser ::parseHttp(const string &html,size_t start){
    size_t index=start;
    while(index<html.size()){
        char c=html[index];
        if(c=='\'' || c=='"' || c==')' || c==';'){
            links.push_back(html.substr(start,index-start));
            return index;
        }
        index++;
    }
    
    return html.size();
}


size_t HtmlParser::parseHref(const string &html,size_t start){
    size_t index=start;

    while(index<html.size()){
        char c=html[index];
        if(c==' '){
            index++;
            continue;
        }
        if(c=='='){
            index++;
            continue;
        }
        if(c=='"' || c=='\''){
            index++;
            size_t first=index;
            while(index<html.size()){
                char d=html[index];
                if(d=='"' || d=='\'' || d==')' || d==';'){
                    links.push_back(html.substr(first,index-first));
                    return index;
                }
                index++;
            }
            return html.size();
        }
        index++;
    }

    return html.size();
}

DynamicArray<string> HtmlParser::parseHtml(const string &html){
    size_t index=0;
    links.clear();
    while(index+4<=html.size()){
        if(html.compare(index,4,"href")==0){
            index=parseHref(html,index+4);
            continue;
        }
        if(html.compare(index,4,"http")==0){
            index=parseHttp(html,index);
            continue;
        }
        index++;
    }

    return links;
}
