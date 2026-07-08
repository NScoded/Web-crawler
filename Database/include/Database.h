#pragma once

#include <sqlite3.h>
#include <string>

class Database
{
private:

    sqlite3* db;

public:

    Database();

    ~Database();

    bool connect(const std::string& filename);

    bool createTable();

    bool insertPage(
        const std::string& url,
        const std::string& html
    );

    void close();
};