#include "include/Database.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main()
{
    Database database;

    if (!database.connect("crawler.db"))
        return 1;

    if (!database.createTable())
        return 1;

    // Open HTML file
    std::ifstream file("../page.html");

    if (!file)
    {
        std::cout << "File not found!\n";
        return 1;
    }

    // Read complete file
    std::stringstream ss;
    ss << file.rdbuf();

    std::string html = ss.str();

    std::cout << "HTML Length : " << html.length() << '\n';

    if (database.insertPage("https://nileshsahu.in", html))
    {
        std::cout << "Insert Successful\n";
    }
    else
    {
        std::cout << "Insert Failed\n";
    }

    database.close();

    return 0;
}