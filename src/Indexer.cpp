#include "../include/Indexer.h"
#include <cctype>
#include <iostream>
#include <sstream>

using namespace std;


Indexer::Indexer(){
    string ignore="a,an,the,is,am,are,was,were,be,been,being,do,does,did,have,has,had,can,could,will,would,shall,should,may,might,must,i,me,my,mine,we,our,ours,you,your,yours,he,him,his,she,her,hers,it,its,they,them,their,theirs,this,that,these,those,who,whom,whose,what,which,when,where,why,how,and,or,but,if,then,else,than,because,while,although,though,of,to,in,on,at,by,for,with,from,into,onto,over,under,between,through,during,before,after,above,below,up,down,out,off,about,against,as,such,so,very,too,also,only,just,even,still,again,once,here,there,now,yet,all,any,both,each,few,more,most,other,some,no,nor,not,own,same,s,t,d,ll,m,o,re,ve,y";
    std::stringstream ss(ignore);
    
    std::string word;

    while (getline(ss, word, ','))
    {
        stopword.insert(word);
    }
    
}

Indexer::~Indexer()
{
}

void Indexer::buildIndex()
{
    int maxId = storage.getMaxPageId();

    std::cout << "Building Index..." << std::endl;
    std::cout << "Total Pages : " << maxId << std::endl;

    for (int id = 1; id <= maxId; id++)
    {
        string html = storage.getHtml(id);
        string url = storage.getUrl(id);

        if (html.empty())
            continue;

        
        

        countWords(html,url);

        //storeIndex(id, frequency);

        std::cout << "Indexed Page : " << id << std::endl;
    }
    
    DynamicArray<string>array=frequency.getkeys();
    for(int i=0;i<array.size();i++){
        int freq=frequency.get(array[i])[0].freq;
        cout<<"key: "<<array[i]<<" -> "<<freq<<endl;

    }



    std::cout << "Indexing Completed." << std::endl;
    
    std::cout << "Storing "<<frequency.size()<<" words in database..." << std::endl;

    storeIndex();
}

std::string Indexer::normalizeWord(std::string& word)
{
    std::string normalized;

    for (char ch : word){
        unsigned char c = static_cast<unsigned char>(ch);

        // alphabets aur digits
        if (std::isalnum(c)){
            normalized += std::tolower(c);
        }
    }
    
    if(stopword.exists(word))return "";
    //normalized = stemWord(normalized);
    return normalized;
}

void Indexer:: countWords(const std::string& html,string & link){
     stringstream ss(html);

     HashMap<string,int>pageFrequency;

     string word;
     while(ss>>word){
        word = normalizeWord(word);
        if(word.empty())continue;
        if(pageFrequency.exists(word)){
            int size=pageFrequency.get(word);
            size++;
            pageFrequency.insert(word,size);
            continue;
        }
        pageFrequency.insert(word,1);
    }

    DynamicArray<string>keys=pageFrequency.getkeys();
    for(int i=0;i<keys.size();i++){
        if(!frequency.exists(keys[i])){
            Url url;
            url.freq=pageFrequency.get(keys[i]);
            url.link=link;
            DynamicArray<Url>u;
            u.push_back(url);
            frequency.insert(keys[i],u);
            
        }
        else{
            DynamicArray<Url>rank=frequency.get(keys[i]);
            int pagekey=pageFrequency.get(keys[i]);
            bool inserted=false;
            Url greater;
            greater.freq=pagekey;
            greater.link=link;

            for(int j=0;j<rank.size();j++){
                if(pagekey > rank[j].freq){
                    rank.insert(j,greater);
                    if(rank.size()>5){
                        rank.pop_back();
                    }
                    inserted=true;
                    break;
                }

            }
            if(!inserted){
                if(rank.size()<5){
                    rank.push_back(greater);
                }
            }
            frequency.insert(keys[i],rank);
        }

    }
    
}

bool Indexer::endsWith(const std::string& word,
                       const std::string& suffix)
{
    if (word.length() < suffix.length())
        return false;

    return word.compare(word.length() - suffix.length(),
                        suffix.length(),
                        suffix) == 0;
}

bool Indexer::isVowel(char ch)
{
    ch = std::tolower(ch);
    return ch == 'a' || ch == 'e' || ch == 'i' ||
           ch == 'o' || ch == 'u';
}

bool Indexer::containsVowel(const std::string& word)
{
    for(char ch : word)
    {
        if(isVowel(ch))
            return true;
    }

    return false;
}

bool Indexer::isDoubleConsonant(const std::string& word)
{
    if(word.length() < 2)
        return false;

    char a = word[word.length()-1];
    char b = word[word.length()-2];

    return a == b && !isVowel(a);
}

std::string Indexer::stemWord(std::string word)
{
    if(word.length() <= 2)
        return word;

    // studies -> study
    if(endsWith(word,"ies") && word.length() > 4)
    {
        word.erase(word.length()-3);
        word += "y";
        return word;
    }

    // boxes -> box
    // watches -> watch
    if(endsWith(word,"es") && word.length() > 3)
    {
        word.erase(word.length()-2);
        return word;
    }

    // booked -> book
    // played -> play
    if(endsWith(word,"ed"))
    {
        std::string stem = word.substr(0,word.length()-2);

        if(containsVowel(stem))
        {
            word = stem;

            if(isDoubleConsonant(word))
                word.pop_back();

            return word;
        }
    }

    // racing -> race
    // smelling -> smell
    // running -> run
    if(endsWith(word,"ing"))
    {
        std::string stem = word.substr(0,word.length()-3);

        if(containsVowel(stem))
        {
            word = stem;

            if(isDoubleConsonant(word))
            {
                word.pop_back();
            }
            else if(!word.empty() &&
                    word.back()=='c')
            {
                word+='e';
            }

            return word;
        }
    }

    // cars -> car
    // books -> book
    if(endsWith(word,"s") &&
       !endsWith(word,"ss") &&
       word.length()>3)
    {
        word.pop_back();
    }

    return word;
}

void Indexer::storeIndex()
{
    DynamicArray<std::string> keys = frequency.getkeys();

    for (int i = 0; i < keys.size(); i++)
    {
        std::string url;

        DynamicArray<Url> rank = frequency.get(keys[i]);

        for (int j = 0; j < rank.size(); j++)
        {
            if (j > 0)
                url += ",";

            url += rank[j].link;
        }

        storage.putIndexer(keys[i], rank[0].freq, url);
    }
    cout<<"Storing complete\n";
}


