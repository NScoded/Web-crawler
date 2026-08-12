#include <winsock2.h>
#include <ws2tcpip.h>
#include "../include/SearchEngine.h"
#include<string>
#include<iostream>
#include <cctype>
#include <limits>
#include <nlohmann/json.hpp>
using namespace std;

namespace {

string decodeUrlComponent(const string& value) {
    string decoded;
    decoded.reserve(value.size());

    for (size_t index = 0; index < value.size(); index++) {
        if (value[index] == '+') {
            decoded += ' ';
        } else if (value[index] == '%' && index + 2 < value.size()) {
            const string hex = value.substr(index + 1, 2);
            char* end = nullptr;
            const long character = strtol(hex.c_str(), &end, 16);
            if (end != hex.c_str() && *end == '\0') {
                decoded += static_cast<char>(character);
                index += 2;
            } else {
                decoded += value[index];
            }
        } else {
            decoded += value[index];
        }
    }

    return decoded;
}

void sendAll(SOCKET client, const string& response) {
    size_t sent = 0;
    while (sent < response.size()) {
        const int bytes = send(client, response.data() + sent,
                               static_cast<int>(response.size() - sent), 0);
        if (bytes == SOCKET_ERROR || bytes == 0) return;
        sent += static_cast<size_t>(bytes);
    }
}

void sendJson(SOCKET client, int status, const nlohmann::json& body) {
    const string content = body.dump();
    const string statusText = status == 200 ? "OK" : "Bad Request";
    const string response =
        "HTTP/1.1 " + to_string(status) + " " + statusText + "\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "Content-Length: " + to_string(content.size()) + "\r\n\r\n" + content;
    sendAll(client, response);
}

int runSearchApi() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Could not start the local search API.\n";
        return 1;
    }

    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

    if (bind(server, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR ||
        listen(server, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Could not use http://127.0.0.1:8080. It may already be running.\n";
        closesocket(server);
        WSACleanup();
        return 1;
    }

    SearchEngine search;
    cout << "Search API running at http://127.0.0.1:8080/api/search?q=car\n";
    cout << "Keep this window open while using index.html.\n";

    while (true) {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;

        char buffer[8192]{};
        const int bytesRead = recv(client, buffer, sizeof(buffer) - 1, 0);
        const string request = bytesRead > 0 ? string(buffer, bytesRead) : "";
        const size_t lineEnd = request.find("\r\n");
        const string requestLine = request.substr(0, lineEnd);
        const size_t queryStart = requestLine.find("/api/search?q=");

        if (queryStart == string::npos) {
            sendJson(client, 400, {{"error", "Use /api/search?q=your+search"}});
        } else {
            const size_t valueStart = queryStart + string("/api/search?q=").size();
            const size_t valueEnd = requestLine.find(' ', valueStart);
            string query = decodeUrlComponent(requestLine.substr(valueStart, valueEnd - valueStart));

            if (query.empty()) {
                sendJson(client, 400, {{"error", "Enter a search query."}});
            } else {
                DynamicArray<DynamicArray<string>> results = search.keyword(query);
                nlohmann::json response;
                response["results"] = nlohmann::json::array();
                for (int index = 0; index < results.size(); index++) {
                    response["results"].push_back({
                        {"title", results[index][0]},
                        {"url", results[index][0]},
                        {"description", results[index][1]}
                    });
                }
                sendJson(client, 200, response);
            }
        }
        closesocket(client);
    }
}

} // namespace

DynamicArray<DynamicArray<string>> SearchEngine::keyword(string &words){
    DynamicArray<DynamicArray<string>>ans;
    DynamicArray<DynamicArray<string>>linksByWord;
    DynamicArray<string>queryWords;
    stringstream ks(words);
    string word;

    while(getline(ks,word,' ')){
        word=indexer.normalizeWord(word);
        if(word.empty()) continue;

        queryWords.push_back(word);
        linksByWord.push_back(page.getIndexer(word));
    }

    if(queryWords.size() == 0) return ans;

    // Return one result for each query word in turn: word 1, word 2, ...,
    // then start again at word 1 until every word's result list is exhausted.
    Set<string>visited;
    DynamicArray<int>nextLink(queryWords.size(), 0);
    bool hasMoreResults = true;

    while(hasMoreResults){
        hasMoreResults = false;

        for(int wordIndex=0; wordIndex<queryWords.size(); wordIndex++){
            DynamicArray<string>& links = linksByWord[wordIndex];

            while(nextLink[wordIndex] < links.size() &&
                  visited.exists(links[nextLink[wordIndex]])){
                nextLink[wordIndex]++;
            }

            if(nextLink[wordIndex] >= links.size()) continue;

            hasMoreResults = true;
            string link = links[nextLink[wordIndex]++];
            string html=page.getHtml(link);
            string target=html;
            visited.insert(link);
            for (char &ch : html)ch = tolower(ch);
            size_t wordpos,leftpos,rightpos;
        
            wordpos=html.find(queryWords[wordIndex]);
            
            if(wordpos==string::npos){
                continue;
            }
            leftpos = html.rfind('.', wordpos);
            rightpos = html.find('.', wordpos);
            string description="";
            if(leftpos==string::npos || rightpos-leftpos>150){
                size_t remaining=150;
                if(html.size()-wordpos<150)remaining=html.size()-wordpos;
                description=html.substr(wordpos,remaining);
                size_t index=wordpos+remaining;
                while(index<html.size() && html[index]!=' '){
                    description=description+html[index];
                    index++;
                }
                description=description+"...";
            }
            else{
                description = target.substr(leftpos+1, rightpos - leftpos);
            }
            DynamicArray<string>row;
            row.push_back(link);
            row.push_back(description);
            ans.push_back(row);
        }
    }

    return ans;
}

int main(int argc, char* argv[]){
    if (argc > 1 && string(argv[1]) == "--server") {
        return runSearchApi();
    }

    cout<<"Main started! \n";
    SearchEngine search;
    DynamicArray<DynamicArray<string>>result;
    string input;
    while(true){
        cout<<"------------------------------------------------\n";
        cout<<"search >> ";
        getline(cin, input);
        cout<<"------------------------------------------------\n";

        if(input.empty()){
            continue;
        }

        if(input=="0")break;

        result=search.keyword(input);
        for(int i=0;i<result.size();i++){
            cout<<"Result: "<<i<<endl;
            cout<<"Link: "<<result[i][0]<<endl;
            cout<<"description: "<<result[i][1]<<endl;
            cout<<"--------------------------------------------------------------------\n";
            
        }
        cout<<"\n \n \n";
    }
}
