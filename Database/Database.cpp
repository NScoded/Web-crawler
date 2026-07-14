#include <iostream>
#include <string>
#include <mysql.h>

using namespace std;


class Database{
    public:
    MYSQL *conn;
    Database(){
        conn = mysql_init(nullptr);

    if (conn == nullptr)
    {
        cout << "mysql_init() failed" << endl;
        
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
    }

    cout << "Database Connected Successfully" << endl;

    }

    bool putRecord(string & url,string & html,int depth,string lastCrawl){
        string query =
            "INSERT INTO pages(url, depth, html, last_crawl) VALUES('"
            + url + "',"
            + to_string(depth)
            + ",'"
            + html
            + "','"
            + lastCrawl
            + "')";
        if (mysql_query(conn, query.c_str())){
            cout << "Insert Error : " << mysql_error(conn) << endl;
            return false;
        }
        return true;
    }
    bool get(const string &url,int &depth,string &html,string &lastCrawl){
        string query ="SELECT depth, html, last_crawl FROM pages WHERE url='"+ url + "' LIMIT 1";
        if (mysql_query(conn, query.c_str())){
            cout << "Select Error : " << mysql_error(conn) << endl;
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

    string getHtml(string &url){
        string html,last;
        int depth;
        get(url,depth,html,last);
        return html;
    }

    int getDepth(string &url){
        string html,last;
        int depth;
        get(url,depth,html,last);
        return depth;
    }
    
    string getLastCrawl(string &url){
        string html,last;
        int depth;
        get(url,depth,html,last);
        return last;
    }
    ~Database(){
        if(conn)
            mysql_close(conn);
    }

};

int main(){
    Database d;
    
}