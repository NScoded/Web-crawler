#include "../include/Crawler.h"

Crawler::Crawler(){
    depth=2;
    normalizer.seedLink="empty";
}


void Crawler::crawl(string seed,int deep=0){

    if(normalizer.isrelative(seed)){
        cout<<"Source link is invalid or relative"<<endl;
        return;
    }
    normalizer.seedLink = seed;

    frontier.put(seed, deep);

    while(!frontier.empty()){
        cout<<"Next Url :"<<frontier.getLink()<<"-> "<<frontier.getDepth()<<endl;

        string link = frontier.getLink();
        int linkDepth = frontier.getDepth();
        frontier.pop();

        try{
            if(linkDepth > depth) continue;

            normalizer.normalize(link);
            if(link.empty()) continue;

           

            if(visited.exists(link)){
                cout << "Already visited\n" << link << endl;
                continue;
            }

           
            string html = fetch.getHtml(link);
            visited.insert(link);
           

            DynamicArray<string> links = htmlparser.parseHtml(html);
            for(int i = 0; i < links.size(); i++){
                normalizer.normalize(links[i]);
                if(!links[i].empty() && !visited.exists(links[i])){
                    frontier.put(links[i], linkDepth + 1);
                    
                }
            }
            cout<<"Total links: "<<links.size()<<endl;
        }
        catch(const std::exception& e){
            std::cout << e.what() << std::endl;
        }
    }
}

int main(){
    Crawler c;
    c.crawl("https://google.com", 0);

}