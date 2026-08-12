#include "../include/Crawler.h"

using namespace std;

Crawler::Crawler(){
    depth=2;
    normalizer.seedLink="empty";
}

void Crawler:: Continue(){
    
    string url;
    int deep;
    int id;
    int max;
    pages.getLastFrontier(url,deep,id,max);

    if(url=="null"){
        cout<<"Frontier is already empty\n";
        return ;
    }
        frontier.backup();
        normalizer.seedLink=url;
        depth=max;
        cout<<"Continuing from last url: "<<url<<"\nDepth: "<<deep<<endl;
        string html=fetch.getHtml(url);
        loop(url,html,id);
        cout<<"Finished\n";
}

void Crawler::crawl(std::string seed, int deep)
{
    depth = deep;

    if (normalizer.isrelative(seed))
    {
        cout << "Source link is invalid or relative" << endl;
        return;
    }

    
    
    // Fetch seed page
    string seedHtml = fetch.getHtml(seed);
    seedHtml=htmlparser.parseContent(seedHtml);
    
   
    
    

    // Insert only into seeds table
    size_t seedId = frontier.putSeed(seed, seedHtml, deep);

    if (seedId == 0) {
        cout << "Seed it invalid\n";
        return;
    }


    loop(seed,seedHtml,seedId);
    
}


void Crawler::loop(string seed,string seedHtml,int seedId){

    size_t count=0;
    size_t duplicateFrontier=0;

    while (!frontier.empty()){
        cout << "-------------------------------------------------------------\n";
        cout << "Next Url : " << frontier.getLink() << endl;
        cout << "Depth    : " << frontier.getDepth() << endl;
        cout << "Count    : " << count << endl;

        string link = frontier.getLink();
        int linkDepth = frontier.getDepth();
        normalizer.seedLink=link;
        string title="";
        string authority="";
        frontier.pop();
        if (frontier.empty()){
            if(pages.clearFrontier())cout<<"Frontier Empty\n";
        }

        

        try
        {
            if (linkDepth > depth)
            {
                cout << "Depth Reached!\n";
                continue;
            }
///////////////////////////////////////////////////////////////////////////
            normalizer.normalize(link,authority);
            cout<<"Authority: "<<authority<<endl;
            if (link.empty())
                continue;

            if (visited.exists(link))
            {
                cout << "Already Visited : " << link << endl;
                duplicateFrontier++;
                continue;
            }

            string html;

            // Seed page ka HTML pehle hi fetch ho chuka hai
            if (link == seed)
                html = seedHtml;
            else
                html = fetch.getHtml(link);

            visited.insert(link);
/////////////////////////////////////////////////////////////////////////////////////
            DynamicArray<string> links = htmlparser.parseLinks(html,title);
            cout<<"Title: "<<title<<endl;
            
            html=htmlparser.parseContent(html);

            for (int i = 0; i < links.size(); i++)
            {
                ///////////////////////////////////////////////////////////
                normalizer.normalize(links[i],authority);

                if (!links[i].empty() && !visited.exists(links[i]))
                {
                    // Database will store unique links
                    frontier.put(links[i], linkDepth + 1,depth,seedId);
                }
            }

            // Don't insert Seed page into pages table 
            if (linkDepth != 0)
            {
                // Databse not stores duplicate pages
                pages.storePage(link, html,authority,title, linkDepth,seedId);
            }
            

            cout << "Total links : " << links.size() << endl;
        }
        catch (const std::exception &e)
        {
            cout << "Failed : " << link << endl;
            cout << e.what() << endl;
        }

        count++;
    }
    cout<<"------------------------------------------------------------\n";
    cout<<"Result: \n";
    cout<<"------------------------------------------------------------\n";
    cout<<"Total website visited: "<<count<<endl;
    cout<<"Total duplicate url in pages table : "<<pages.duplicatePages<<endl;
    cout<<"Total duplicate url in frontier : "<<duplicateFrontier<<endl;
    cout<<"Total pages fetch by curl : "<<fetch.curlURL<<endl;
    cout<<"Total pages fetch by CDP : "<<fetch.CDPURL<<endl;
}

int main(){
    cout << "Main Started\n";

    Crawler c;
    Indexer indexer;
    
    

    cout << "\nCrawler Created\n";

    string input;
    int depth,size;

    while(true){

        char choice;
        cout<<"--------------------------------------------------\n";
        cout<<"1. Continue crawler?\n2. Start from a seed link?\n3. Start Indexer\n5. Exit\n";
        cin>>choice;
        cout<<"--------------------------------------------------\n";
        if(choice=='1'){
            c.Continue();
    
        }
        else if(choice =='2'){
            int size;
            cout<<"Enter number of seed links: ";
            cin>>size;
            DynamicArray<string>seed;
            DynamicArray<int>dept;
            for(int i=0;i<size;i++){
                cout<<"Enter a Seed link: ";
                cin>>input;
                cout<<"Enter MaxDepth: ";
                cin>>depth;
                seed.push_back(input);
                dept.push_back(depth);
            }
            for(int i=0;i<size;i++){
                c.crawl(seed[i],dept[i]);
            }
            cout<<"Finished\n";
        }
        else if(choice =='3'){
            indexer.buildIndex();

        }
        else if(choice =='5'){
            cout<<"Crawler exit...";
            break;
            
        }
        else{
            cout<<"You entered an invalid choice \n";
        }

    }

    
    
    
    

}