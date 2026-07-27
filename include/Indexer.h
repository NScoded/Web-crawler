#pragma once

#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "PageStorage.h"
#include "STL/HashMap.h"
#include "Set.h"

class Indexer
{
private:
    PageStorage storage;

    struct Url{
        std::string link;
        int freq;
    };
    std::string stemWord(std::string word);

    bool endsWith(const std::string& word,
                  const std::string& suffix);

    bool isVowel(char ch);

    bool containsVowel(const std::string& word);

    bool isDoubleConsonant(const std::string& word);
    Set<std::string>stopword;
    // Parse HTML and build frequency map
    void countWords(const std::string& html, std::string & url);

    
    
    // Convert text into lowercase words
    std::string normalizeWord( std::string& word);
    
    // Store one page's index into database
    void storeIndex();
        
        public:
        Indexer();
        ~Indexer();
        HashMap<std::string, DynamicArray<Url>> frequency;

    // Build index for all pages
    void buildIndex();
};