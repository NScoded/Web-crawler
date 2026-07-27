#include"../include/HtmlParser.h"
#include"../include/Set.h"
#include<iostream>
using namespace std;


size_t HtmlParser ::parseHttp(const string &html,size_t start){
    size_t index=start;
    while(index<html.size()){
        char c=html[index];
        if(c=='\'' || c=='"' || c==')' || c==';' || c=='<'|| c==' '){
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
                if(d=='"' || d=='\'' || d==')' || d==';'|| d=='<'|| d==' '){
                    string url = html.substr(first,index-first);
                    if (!url.empty()) {
                        string lowerUrl = url;
                        for (char &ch : lowerUrl)
                            ch = (char)tolower((unsigned char)ch);
                        if (lowerUrl.rfind("mailto:", 0) != 0 &&
                            lowerUrl.rfind("javascript:", 0) != 0 &&
                            lowerUrl.rfind("data:", 0) != 0 &&
                            lowerUrl.rfind("tel:", 0) != 0 &&
                            lowerUrl[0] != '#')
                        {
                            links.push_back(url);
                        }
                    }
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

DynamicArray<string> HtmlParser::parseLinks(const string &html){
    size_t index=0;
    links.clear();
    while(index+4<=html.size()){
        if(html.compare(index,4,"href")==0){
            index=parseHref(html,index+4);
            continue;
        }
        if((index + 7 <= html.size() && html.compare(index,7,"http://") == 0) ||
           (index + 8 <= html.size() && html.compare(index,8,"https://") == 0)){
            index=parseHttp(html,index);
            continue;
        }
        index++;
    }

    return links;
}

string HtmlParser:: parseContent(const string& html)
{
    string content;
    string tag;

    bool insideTag = false;
    bool skipContent = false;

    
    Set<string> skipTags;

    skipTags.insert("script");
    skipTags.insert("style");
    skipTags.insert("head");
    skipTags.insert("noscript");
    skipTags.insert("meta");
    skipTags.insert("link");
    skipTags.insert("title");

    for (size_t i = 0; i < html.size(); i++)
    {
        if (html[i] == '<')
        {
            insideTag = true;
            tag.clear();

            i++;

            bool closing = false;

            if (i < html.size() && html[i] == '/')
            {
                closing = true;
                i++;
            }

            while (i < html.size() &&
                   html[i] != '>' &&
                   !isspace((unsigned char)html[i]))
            {
                tag += (char)tolower((unsigned char)html[i]);
                i++;
            }

            while (i < html.size() && html[i] != '>')
                i++;

            if (skipTags.exists(tag))
            {
                if (closing)
                    skipContent = false;
                else
                    skipContent = true;
            }

            insideTag = false;

            if (!content.empty() && content.back() != ' ')
                content += ' ';

            continue;
        }

        if (insideTag || skipContent)
            continue;

        // '\r' '\t' '\n' ' '
        // negative value nhi dalni chahiye
        if (isspace((unsigned char)html[i]))
        {
            if (!content.empty() && content.back() != ' ')
                content += ' ';
        }
        else
        {
            content += html[i];
        }
    }

    return content;
}