#pragma once
#include <string>
#include"STL/DynamicArray.h"
#include"Frontier.h"



class Normalizer{
    private:
    const std::string DOMAIN_FILE = "src/ignore/domain.txt";
    const std::string EXTENSION_FILE = "src/ignore/extension.txt";
    std::string read(std::string page);
    void To_lower(std::string& link);
    void removeFragment(std::string& link);
    void normalizePath(std::string &source);
    void relativeURL(std::string &source);
    Frontier frontier;
    
    
    
    public:
    Set<std::string>ignoreExtension;
    Set<std::string>ignoreDomain;
    bool isrelative(std::string &source);
    std::string seedLink;
    Normalizer();
    void normalize(std::string & link,std::string & authority);
    DynamicArray<std::string> normalize(DynamicArray<std::string> & link);

};