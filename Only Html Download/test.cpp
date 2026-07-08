#include "Downloader.h"
#include <iostream>
#include <string>

int main()
{
    try
    {
        Downloader downloader;

        std::string html =
            downloader.download("http://127.0.0.1:9222/json");

        std::cout << "HTML Size = " << html.size() << '\n';
        std::cout << html.substr(0, 500) << '\n';
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
}