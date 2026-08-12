#pragma once
#include "STL/DynamicArray.h"
#include <string>
#include <mysql.h>

class PageStorage
{
private:

public:
MYSQL *conn;
    PageStorage();
    ~PageStorage();
    size_t duplicatePages=0;
    
    // Pages Table
    bool storePage(std::string &url,
                   std::string &html,
                   std::string &authority,
                   std::string &title,
                   int depth,
                   int id);

    bool getPage(const std::string &url,
                 int &depth,
                 std::string &html,
                 std::string &lastCrawl);

    std::string getHtml(std::string &url);
    std::string getHtml(int id);
    std::string getTitle(int id);
    std::string getAuthority(int id);
    int getMaxPageId();  
    std::string getUrl(int id);

    int getDepth(std::string &url);

    std::string getLastCrawl(std::string &url);

    std::string getLastFrontier(std::string &url,int &deep,int& id,int &max);

    // Frontier Table
    bool putFrontier(std::string url,
                     int depth,int max,int Id);

    bool deleteFrontier(std::string url,
                        int depth);

    bool clearFrontier();

    void getFrontier(std::string &url,
                     int &depth);

    // Seeds Table
    size_t putSeeds(std::string &url,
                 std::string &html,
                 int &depth,
                 int &max);
    
    // Indexer Table
    bool putIndexer(const std::string &word,
                int maxFrequency,
                const std::string &url);
    
    DynamicArray<std::string> getIndexer(std::string &word);
};
