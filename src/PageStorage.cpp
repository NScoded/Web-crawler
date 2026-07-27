#include "../include/PageStorage.h"
#include <cstring>
#include <iostream>

using namespace std;

// PageStorage()
PageStorage::PageStorage(){
    conn = mysql_init(nullptr);

    if (conn == nullptr) {
        cout << "mysql_init() failed" << endl;
        return;
    }

    if (!mysql_real_connect(
            conn,
            "localhost",
            "root",
            "Nilesh@mysql2005",
            "webcrawler",
            3306,
            nullptr,
            0))
    {
        cout << "Connection Failed" << endl;
        cout << mysql_error(conn) << endl;
        mysql_close(conn);
        conn = nullptr;
        return;
    }

    cout << "Database Connected Successfully" << endl;
}

// putFrontier()
bool PageStorage::putFrontier(string url, int depth, int max, int Id)
{
    const char *query =
        "INSERT INTO frontier(url, depth, max_depth, seed_id) VALUES(?, ?, ?, ?)";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);

    if (stmt == nullptr)
    {
        cout << "Statement initialization failed" << endl;
        return false;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        cout << "Prepare Error : "
             << mysql_stmt_error(stmt) << endl;

        mysql_stmt_close(stmt);
        return false;
    }

    MYSQL_BIND bind[4];
    memset(bind, 0, sizeof(bind));

    unsigned long urlLength = url.length();

    // url
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)url.c_str();
    bind[0].buffer_length = urlLength;
    bind[0].length = &urlLength;

    // depth
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &depth;

    // max_depth
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = &max;

    // seed_id
    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer = &Id;

    if (mysql_stmt_bind_param(stmt, bind))
    {
        cout << "Bind Error : "
             << mysql_stmt_error(stmt) << endl;

        mysql_stmt_close(stmt);
        return false;
    }

    if (mysql_stmt_execute(stmt))
    {
        cout << "Execute Error : "
             << mysql_stmt_error(stmt) << endl;

        mysql_stmt_close(stmt);
        return false;
    }

    mysql_stmt_close(stmt);

    return true;
}

// getFrontier()
void PageStorage::getFrontier(string &url,int &depth){
    string query="select url,depth from frontier order by id desc LIMIT 1";
    if(mysql_query(conn,query.c_str())){
        cout<<"Error in getting Last Url: "<<mysql_error(conn)<<endl;
    }
    MYSQL_RES *result = mysql_store_result(conn);
    if (result == nullptr){
         url = "null";
        depth = -1;
        mysql_free_result(result);
        return;
    }
        

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == nullptr) {
        url = "null";
        depth = -1;
        mysql_free_result(result);
        return ;
    }

    url=row[0];
    depth=stoi(row[1]);
    mysql_free_result(result);
    return ;
}

// deleteFrontier
bool PageStorage::deleteFrontier(string url, int depth)
{
    std::string query =
        "DELETE FROM frontier where url = '" +
        url + "' and depth = " + std::to_string(depth) + " ;";

    if (mysql_query(conn, query.c_str()))
    {
        std::cout << "Error inserting into frontier: "
                  << mysql_error(conn) << std::endl;
        return false;
    }

    return true;
}

//storePage
bool PageStorage::storePage(std::string &url,
                            std::string &html,
                            int depth,
                            int id)
{
    const char *query =
        "INSERT INTO pages(url, depth, html, seed_id) VALUES(?, ?, ?, ?)";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);

    if (stmt == nullptr)
    {
        cout << "Statement initialization failed" << endl;
        return false;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        duplicatePages++;
        cout<<"Duplicate pages found: "<<duplicatePages;
        mysql_stmt_close(stmt);
        return false;
    }

    MYSQL_BIND bind[4];
    memset(bind, 0, sizeof(bind));

    unsigned long urlLength = url.length();
    unsigned long htmlLength = html.length();

    // url
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)url.c_str();
    bind[0].buffer_length = urlLength;
    bind[0].length = &urlLength;

    // depth
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &depth;

    // html
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (char *)html.c_str();
    bind[2].buffer_length = htmlLength;
    bind[2].length = &htmlLength;

    // seed_id
    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer = &id;

    if (mysql_stmt_bind_param(stmt, bind))
    {
        cout << "Bind Error : "
             << mysql_stmt_error(stmt) << endl;

        mysql_stmt_close(stmt);
        return false;
    }

    if (mysql_stmt_execute(stmt))
    {
        cout << "Execute Error : "
             << mysql_stmt_error(stmt) << endl;

        mysql_stmt_close(stmt);
        return false;
    }

    mysql_stmt_close(stmt);

    return true;
}

//getLastFrontier
string PageStorage::getLastFrontier(string& url,int& deep,int& id,int&max)
{
    string query = "SELECT url,depth,seed_id,max_depth FROM frontier ORDER BY id LIMIT 1";

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error in getting Last Url: "
             << mysql_error(conn) << endl;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (result == nullptr){

        url="null";
        return url;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if (row == nullptr)
    {
        mysql_free_result(result);
        url="null";
        return url;
    }

    url = row[0];
    deep = stoi(row[1]);
    id = stoi(row[2]);
    max = stoi(row[3]);

    mysql_free_result(result);

    return url;
}

//getPage()
bool PageStorage::getPage(const string &url,
                          int &depth,
                          string &html,
                          string &lastCrawl)
{
    string query =
        "SELECT depth, html, last_crawl FROM pages WHERE url='" +
        url +
        "' LIMIT 1";

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Select Error : "
             << mysql_error(conn) << endl;
        return false;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (result == nullptr)
        return false;

    MYSQL_ROW row = mysql_fetch_row(result);

    if (row == nullptr)
    {
        mysql_free_result(result);
        return false;
    }

    depth = row[0] ? stoi(row[0]) : 0;
    html = row[1] ? row[1] : "";
    lastCrawl = row[2] ? row[2] : "";

    mysql_free_result(result);

    return true;
}

bool PageStorage:: clearFrontier(){
    string query="TRUNCATE FRONTIER;";
    if (mysql_query(conn, query.c_str()))
    {
        std::cout << "Error inserting into frontier: "
                  << mysql_error(conn) << std::endl;
        return false;
    }

    return true;

}

string PageStorage::getHtml(string &url)
{
    string html;
    string last;
    int depth;

    if (!getPage(url, depth, html, last))
        return "";

    return html;
}

int PageStorage::getDepth(string &url)
{
    string html, last;
    int depth = -1;

    if (!getPage(url, depth, html, last))
        return -1;

    return depth;
}

string PageStorage::getLastCrawl(string &url)
{
    string html;
    string last;
    int depth;

    if (!getPage(url, depth, html, last))
        return "";

    return last;
}

size_t PageStorage::putSeeds(std::string &link,
                             std::string &html,
                             int &depth,
                             int &max)
{
    const char *query =
        "INSERT INTO seeds(url, max_depth, html, depth) VALUES(?, ?, ?, 0)";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);

    if (stmt == nullptr)
    {
        cout << "Statement initialization failed\n";
        return 0;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        cout << "Prepare Error : "
             << mysql_stmt_error(stmt) << endl;

        mysql_stmt_close(stmt);
        return 0;
    }

    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));

    unsigned long urlLength = link.length();
    unsigned long htmlLength = html.length();

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)link.c_str();
    bind[0].buffer_length = urlLength;
    bind[0].length = &urlLength;

    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &max;

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (char *)html.c_str();
    bind[2].buffer_length = htmlLength;
    bind[2].length = &htmlLength;

    if (mysql_stmt_bind_param(stmt, bind))
    {
        cout << "Bind Error : "
             << mysql_stmt_error(stmt) << endl;

        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_execute(stmt))
    {
        cout << "Execute Error : "
             << mysql_stmt_error(stmt) << endl;
        cout<< "Error hint: You have already crawl this link as a seed url\n";
        mysql_stmt_close(stmt);
        return 0;
    }

    // Retrieve the auto-generated ID
    size_t seedId = static_cast<size_t>(mysql_stmt_insert_id(stmt));

    mysql_stmt_close(stmt);

    return seedId;
}

PageStorage::~PageStorage()
{
    if (conn)
        mysql_close(conn);
}

std::string PageStorage::getHtml(int id)
{
    std::string html="";

    const char *query = "SELECT html FROM pages WHERE id = ?";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
        return html;

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        mysql_stmt_close(stmt);
        return html;
    }

    MYSQL_BIND param[1];
    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = &id;
    param[0].is_unsigned = false;

    if (mysql_stmt_bind_param(stmt, param))
    {
        mysql_stmt_close(stmt);
        return html;
    }

    if (mysql_stmt_execute(stmt))
    {
        mysql_stmt_close(stmt);
        return html;
    }

    char buffer[65536];
    unsigned long length = 0;
    bool isNull = false;
    bool error = false;

    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));

    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = buffer;
    result[0].buffer_length = sizeof(buffer);
    result[0].length = &length;
    result[0].is_null = &isNull;
    result[0].error = &error;

    if (mysql_stmt_bind_result(stmt, result))
    {
        mysql_stmt_close(stmt);
        return html;
    }

    int status = mysql_stmt_fetch(stmt);

    if (status == 0 && !isNull)
    {
        html.assign(buffer, length);
    }

    mysql_stmt_close(stmt);

    return html;
}

int PageStorage::getMaxPageId()
{
    const char *query = "SELECT MAX(id) FROM pages";

    if (mysql_query(conn, query))
    {
        std::cerr << "MySQL Query Error: "
                  << mysql_error(conn) << std::endl;
        return 0;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (result == nullptr)
    {
        std::cerr << "Failed to fetch result: "
                  << mysql_error(conn) << std::endl;
        return 0;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    int maxId = 0;

    if (row && row[0] != nullptr)
    {
        maxId = std::stoi(row[0]);
    }

    mysql_free_result(result);

    return maxId;
}

std::string PageStorage::getUrl(int id)
{
    std::string url;

    std::string query =
        "SELECT url FROM pages WHERE id = " + std::to_string(id);

    if (mysql_query(conn, query.c_str()))
    {
        std::cerr << "MySQL Query Error: "
                  << mysql_error(conn) << std::endl;
        return "";
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (result == nullptr)
    {
        std::cerr << "MySQL Store Result Error: "
                  << mysql_error(conn) << std::endl;
        return "";
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if (row != nullptr && row[0] != nullptr)
    {
        url = row[0];
    }

    mysql_free_result(result);

    return url;
}

bool PageStorage::putIndexer(const std::string &word,
                             int maxFrequency,
                             const std::string &url)
{
    const char *query =
        "INSERT INTO indexer(word, max_frequency, url) "
        "VALUES(?, ?, ?)";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
        return false;

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        mysql_stmt_close(stmt);
        return false;
    }

    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));

    // word
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void *)word.c_str();
    unsigned long wordLength = word.length();
    bind[0].length = &wordLength;

    // max_frequency
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (void *)&maxFrequency;

    // url
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void *)url.c_str();
    unsigned long urlLength = url.length();
    bind[2].length = &urlLength;

    if (mysql_stmt_bind_param(stmt, bind))
    {
        mysql_stmt_close(stmt);
        return false;
    }

    bool success = (mysql_stmt_execute(stmt) == 0);

    mysql_stmt_close(stmt);
    return success;
}