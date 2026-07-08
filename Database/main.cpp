#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>

#include <mysql_driver.h>
#include <mysql_connection.h>

#include <cppconn/prepared_statement.h>

using namespace std;

int main()
{
    try
    {
        auto driver =
            sql::mysql::get_mysql_driver_instance();

        unique_ptr<sql::Connection> conn(
            driver->connect(
                "tcp://127.0.0.1:3306",
                "root",
                "YOUR_PASSWORD"
            )
        );

        conn->setSchema("crawler_db");

        ifstream file("page.html");

        stringstream ss;

        ss << file.rdbuf();

        string html = ss.str();

        auto pstmt =
            unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement(
                    "INSERT INTO pages(url,html_content) VALUES(?,?)"
                )
            );

        pstmt->setString(
            1,
            "https://example.com"
        );

        pstmt->setString(
            2,
            html
        );

        pstmt->execute();

        cout << "Inserted Successfully\n";
    }

    catch(sql::SQLException &e)
    {
        cout << e.what() << endl;
    }
}