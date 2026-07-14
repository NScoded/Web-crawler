#include "../include/Normalizer.h"
#include<string>
#include<sstream>
#include<fstream>



// To_lower()
void Normalizer::To_lower(string &link){
    for(char &c:link){
        if(c>= 'A' && c<='Z'){
            c=c-'A'+'a';
        }
    }
    
};

// read()
string Normalizer::read(string page){
    ifstream file(page);

    if(!file.is_open()){
        cout<<"File not found"<<endl;
        return "File not found";
    }
    

    string txt;

    while (getline(file, txt)) {

        if (!txt.empty() && txt.back() == ',')
            txt.pop_back();

    
        if(page==DOMAIN_FILE){

            ignoreDomain.insert(txt);
        }
        else{
            ignoreExtension.insert(txt);
        }

    }

    file.close();
    return txt;



}

// Normalizer()
Normalizer::Normalizer(){
    read(DOMAIN_FILE);
    read(EXTENSION_FILE);
    
}


// removeFragment()
void Normalizer::removeFragment(string &source){
    size_t pos=source.find('#');
    if(pos==string::npos) return ;
    source=source.substr(0,pos);    
}

// normalizePath()
void Normalizer::normalizePath(string &source){
    DynamicArray<string>content;
    string token;
    stringstream ss(source);
    while(getline(ss,token,'/')){
        if(token.empty() || token==".")continue;

        if(token==".."){
            if(content.size()!=0){
                content.pop_back();
            }
            continue;
        }
        content.push_back(token);
    }
    if(content.size()){
        string extension=content[content.size()-1];
        size_t pos=extension.find('.');
        if(pos!=string::npos){
            extension=extension.substr(pos);
            if(ignoreExtension.exists(extension)){
               
                source="";
                return ;
            }
            
        }
    }
    string result="/";
    for(int i=0;i<content.size();i++){
        result=result+ content[i]+'/';
    }
    source=result;
}

// isrelative()
bool Normalizer::isrelative(string & source){
    size_t schemePos=source.find("://");
    if(schemePos==string::npos){
        return true;
    }
    return false;

}

//string normalize()
void Normalizer::normalize(string &source){
    To_lower(source);

    // relative url hai
    if(isrelative(source)){
        relativeURL(source); 
        if (source.empty())
        return ; 
    }
    
    removeFragment(source);
    if (source.empty()) return ;
    

    
    size_t schemePos=source.find("://"); 
    
    
    string scheme = source.substr(0,schemePos);
    string remaining = source.substr(schemePos+3);

    size_t slashPos=remaining.find('/');
    if(slashPos==string::npos){
        return;
    }

    string authority=remaining.substr(0,slashPos);
    string path =remaining.substr(slashPos);

    size_t dotPos=authority.find(':');
    if(dotPos!=string::npos){
        string port=authority.substr(dotPos+1);
        string host=authority.substr(0,dotPos);

        if((port=="80" && scheme=="http")||(port=="443" && scheme == "https")){
            authority=host;
        }
    }

    // ignoreDomains
    size_t checkpos=authority.find("www.");
    string checkauth=authority;
    if(checkpos!=string::npos){
        checkauth=authority.substr(4);
    }
    if(ignoreDomain.exists(checkauth)){
        source="";
        return ;
    }
    
    normalizePath(path);

    
    source= scheme + "://" + authority + path;
}

// relative ()
void Normalizer::relativeURL(string & source){
    if(seedLink=="empty"){
        cout<<"Seed link is not valid or relative";
        return ;
    }
    if (source.rfind("mailto:", 0) == 0 ||
    source.rfind("tel:", 0) == 0 ||
    source.rfind("javascript:", 0) == 0 ||
    source.rfind("data:", 0) == 0){
        source = "";
        return;
    }

    if (!source.empty() && source[0] == '/')
        source.erase(0, 1);

    source = seedLink +"/"+ source;
    
   

}

// DynamicArray<string> normalizer()
DynamicArray<string> Normalizer::normalize(DynamicArray<string> &source){
    DynamicArray<string>normalisedlink;

    for(int i=0;i<source.size();i++){

        To_lower(source[i]);
    
        removeFragment(source[i]);
    
        size_t schemePos=source[i].find("://"); 
        
        if(schemePos==string::npos){
            continue;
        }
    
        string scheme = source[i].substr(0,schemePos);
        string remaining = source[i].substr(schemePos+3);
    
        size_t slashPos=remaining.find('/');
        if(slashPos==string::npos){
            continue;
        }
    
        string authority=remaining.substr(0,slashPos);
        string path =remaining.substr(slashPos);
    
        size_t dotPos=authority.find(':');
        if(dotPos!=string::npos){
            string port=authority.substr(dotPos+1);
            string host=authority.substr(0,dotPos);
    
            if((port=="80" && scheme=="http")||(port=="443" && scheme == "https")){
                authority=host;
            }
        }
        normalizePath(path);

        normalisedlink.push_back(scheme + "://" + authority + path);
    
    }
    return normalisedlink;
}


