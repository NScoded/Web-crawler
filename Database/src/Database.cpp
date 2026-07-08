#include "../include/Database.h"

#include <iostream>

Database::Database()
{
    db = nullptr;
}

Database::~Database()
{
    close();
}

// Connect to database
bool Database::connect(const std::string& filename)
{
    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK)
    {
        std::cerr << "Connection Failed : "
                  << sqlite3_errmsg(db)
                  << std::endl;

        return false;
    }

    std::cout << "Connected Successfully\n";
    return true;
}

// Create table
bool Database::createTable()
{
    const char* sql = R"(

        CREATE TABLE IF NOT EXISTS pages
        (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            url TEXT NOT NULL,
            html TEXT NOT NULL
        );

    )";

    char* error = nullptr;

    if (sqlite3_exec(db, sql, nullptr, nullptr, &error) != SQLITE_OK)
    {
        std::cerr << error << std::endl;
        sqlite3_free(error);
        return false;
    }

    std::cout << "Table Created\n";
    return true;
}

// Insert page
bool Database::insertPage(const std::string& url,
                          const std::string& html)
{
    const char* sql =
        "INSERT INTO pages(url, html) VALUES(?, ?);";

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db,
                           sql,
                           -1,
                           &stmt,
                           nullptr) != SQLITE_OK)
    {
        std::cerr << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt,
                      1,
                      url.c_str(),
                      -1,
                      SQLITE_TRANSIENT);

    sqlite3_bind_text(stmt,
                      2,
                      html.c_str(),
                      -1,
                      SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << sqlite3_errmsg(db) << std::endl;

        sqlite3_finalize(stmt);

        return false;
    }

    sqlite3_finalize(stmt);

    std::cout << "Inserted Successfully\n";

    return true;
}

// Close database
void Database::close()
{
    if (db != nullptr)
    {
        sqlite3_close(db);
        db = nullptr;
    }
}