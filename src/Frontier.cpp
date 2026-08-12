#include "../include/Frontier.h"
#include "../include/STL/DynamicArray.h"
#include<string>
#include<sstream>

using namespace std;

static void sanitizeUrl(std::string &link)
{
    for (char &ch : link)
    {
        if (ch == '\\')
            ch = '/';
    }
}

// put(link and depth)
void Frontier::put(string & link,int deep,int max,int Id){
    sanitizeUrl(link);
    URL url;
    url.link=link;
    url.depth=deep;
    pages.putFrontier(link,deep,max,Id);
    queue.push(url);
}
// put(link,depth and maxDepth)
size_t Frontier::putSeed(string & link,string & html,int max,int deep){
    sanitizeUrl(link);
    size_t seedId = pages.putSeeds(link,html,deep,max);
    if (seedId == 0) {
        return 0;
    }



    URL url;
    url.link = link;
    url.depth = deep;
    queue.push(url);
    return seedId;
}

// pop()
Frontier :: URL Frontier::pop(){
    URL url=queue.front();
    string link=url.link;
    int depth=url.depth;
    pages.deleteFrontier(link,depth);
    return queue.pop();

}

void Frontier::backup()
{
    string link = "null";
    int depth = 0;
    cout<<"Backup called\n";
    cout<<"Taking Backeup from Pages\n";

    while (true)
    {
        pages.getFrontier(link, depth);

        if (link == "null"){

            cout<<"Break\n";
                break;
        }

        sanitizeUrl(link);
        URL url;
        url.link = link;
        url.depth = depth;
        queue.push(url);
        pages.deleteFrontier(link, depth);
    }
}

// getLink()
string Frontier::getLink(){
    URL url;
    url=queue.front();
    return url.link;
}
// getLink()
size_t Frontier::getSize(){
    return queue.size();
}

// empty()
bool Frontier::empty(){
    return queue.empty();
}

// getDepth()
int Frontier::getDepth(){
    URL url;
    url=queue.front();
    return url.depth;
}

// getDate()
string Frontier:: getDate() {

    time_t now = time(NULL);
    tm *t = localtime(&now);

    char date[11];

    sprintf(date, "%04d-%02d-%02d",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday);

    return date;
}




// exists()


// int main(){
//     Frontier f;
//     string s="HTTP://Example.COM:80/a/./b/../c//index.html#abc";
//     cout<<"Input: "<<s<<endl;
//     cout<<"Output: "<<f.normalizer(s);
// }