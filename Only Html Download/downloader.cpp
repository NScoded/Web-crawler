#include "downloader.h"
#include<iostream>
#include <curl/curl.h>
#include <stdexcept>

static size_t WriteCallback(void* contents,
                            size_t size,
                            size_t nmemb,
                            void* userp)
{
    size_t totalSize = size * nmemb;

    std::string* html = static_cast<std::string*>(userp);

    html->append(static_cast<char*>(contents), totalSize);

    return totalSize;
}
std::string Downloader::download(const std::string& url)
{
    CURL* curl = curl_easy_init();

    if(curl == nullptr)
        throw std::runtime_error("Failed to initialize CURL");

    std::cout << "CURL initialized\n";

    std::string html;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode result = curl_easy_perform(curl);

    std::cout << "Result = " << result << '\n';

    if(result != CURLE_OK)
    {
        std::cout << curl_easy_strerror(result) << '\n';
        curl_easy_cleanup(curl);
        throw std::runtime_error(curl_easy_strerror(result));
    }

    std::cout << "Downloaded " << html.size() << " bytes\n";

    curl_easy_cleanup(curl);

    return html;
}