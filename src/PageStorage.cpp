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
    const char *query = "DELETE FROM frontier WHERE url = ? AND depth = ?";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (stmt == nullptr)
    {
        std::cout << "Statement initialization failed" << std::endl;
        return false;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        std::cout << "Prepare Error : " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }

    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));

    unsigned long urlLength = url.length();

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)url.c_str();
    bind[0].buffer_length = urlLength;
    bind[0].length = &urlLength;

    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &depth;

    if (mysql_stmt_bind_param(stmt, bind))
    {
        std::cout << "Bind Error : " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }

    if (mysql_stmt_execute(stmt))
    {
        std::cout << "Error deleting from frontier: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }

    mysql_stmt_close(stmt);
    return true;
}

//storePage
bool PageStorage::storePage(std::string &url,
                            std::string &html,
                            std::string &authority,
                            std::string &title,
                            int depth,
                            int id){
    const char *query =
        "INSERT INTO pages(url, depth, html, seed_id, authority, title) VALUES(?, ?, ?, ?, ?, ?)";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);

    if (stmt == nullptr)
    {
        cout << "Statement initialization failed" << endl;
        return false;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        duplicatePages++;
        cout << "Duplicate pages found: " << duplicatePages;
        mysql_stmt_close(stmt);
        return false;
    }

    MYSQL_BIND bind[6];
    memset(bind, 0, sizeof(bind));

    unsigned long urlLength = url.length();
    unsigned long htmlLength = html.length();
    unsigned long authorityLength = authority.length();
    unsigned long titleLength = title.length();

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

    // authority
    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (char *)authority.c_str();
    bind[4].buffer_length = authorityLength;
    bind[4].length = &authorityLength;

    // title
    bind[5].buffer_type = MYSQL_TYPE_STRING;
    bind[5].buffer = (char *)title.c_str();
    bind[5].buffer_length = titleLength;
    bind[5].length = &titleLength;

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
    string query = "SELECT url,depth,seed_id,max_depth FROM frontier ORDER BY id DESC LIMIT 1";

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
        std::cout << "Error clearing frontier: "
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
                             const std::string &url){
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

DynamicArray<string> PageStorage:: getIndexer(string &word){
    DynamicArray<string> links;

    string query = "SELECT url FROM indexer WHERE word='" + word + "';";

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error in getting word from indexer: "
             << mysql_error(conn) << endl;
        return links;
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (result == nullptr)
        return links;

    MYSQL_ROW row = mysql_fetch_row(result);

    if (row == nullptr)
    {
        mysql_free_result(result);
        return links;
    }

    string url = row[0];

    while (true)
    {
        size_t pos = url.find(',');

        if (pos == string::npos)
        {
            links.push_back(url);
            break;
        }

        links.push_back(url.substr(0, pos));
        url = url.substr(pos + 1);
    }

    mysql_free_result(result);

    return links;
}

std::string PageStorage::getTitle(int id)
{
    std::string title;

    const char *query = "SELECT title FROM pages WHERE id = ?";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);

    if (stmt == nullptr)
        return title;

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        mysql_stmt_close(stmt);
        return title;
    }

    MYSQL_BIND param[1];
    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = &id;

    mysql_stmt_bind_param(stmt, param);

    char buffer[1024];
    unsigned long length;
    bool isNull = false;

    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));

    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = buffer;
    result[0].buffer_length = sizeof(buffer);
    result[0].length = &length;
    result[0].is_null = &isNull;

    mysql_stmt_bind_result(stmt, result);

    if (mysql_stmt_execute(stmt) == 0 &&
        mysql_stmt_fetch(stmt) == 0 &&
        !isNull)
    {
        title.assign(buffer, length);
    }

    mysql_stmt_close(stmt);

    return title;
}

std::string PageStorage::getAuthority(int id)
{
    std::string authority;

    const char *query = "SELECT authority FROM pages WHERE id = ?";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);

    if (stmt == nullptr)
        return authority;

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        mysql_stmt_close(stmt);
        return authority;
    }

    MYSQL_BIND param[1];
    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = &id;

    mysql_stmt_bind_param(stmt, param);

    char buffer[1024];
    unsigned long length;
    bool isNull = false;

    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));

    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = buffer;
    result[0].buffer_length = sizeof(buffer);
    result[0].length = &length;
    result[0].is_null = &isNull;

    mysql_stmt_bind_result(stmt, result);

    if (mysql_stmt_execute(stmt) == 0 &&
        mysql_stmt_fetch(stmt) == 0 &&
        !isNull)
    {
        authority.assign(buffer, length);
    }

    mysql_stmt_close(stmt);

    return authority;
}