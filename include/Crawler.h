#pragma once
#include "Frontier.h"
#include "Normalizer.h"
#include "../CDP/CDPScraper.h"
#include "HtmlParser.h"
#include "PageStorage.h"
#include "Indexer.h"



class Crawler{
    private:
    int depth;
    void loop(std::string url,std::string seedHtml, int seedId);
    Set<std::string>visited;
    Frontier frontier;
    Normalizer normalizer;
    CDPScraper fetch;
    HtmlParser htmlparser;
    PageStorage pages;
    

    public:
    Crawler();
    void Continue();
    void crawl(std::string seed,int deep);


};