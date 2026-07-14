// main.cpp
// Example: using the reusable CDPScraper class from a separate cpp file.
//
// run:
// ./cdp_scraper.exe

#include "CDPScraper.h"

#include <fstream>
#include <iostream>

int main() {
    try {
        // Chrome ek hi baar launch hoga (constructor mein).
        CDPScraper scraper("C:/Program Files/Google/Chrome/Application/chrome.exe", "9222", 3);

        // Pehla link
        std::string html1 = scraper.getHtml("https://en.wikipedia.org/wiki/Computer");
        std::cout<<html1;
        std::ofstream out1("../pages/output.html", std::ios::binary);
        out1 << html1;
        out1.close();
        std::cout << "Saved page 1 (" << html1.size() << " bytes)\n";

        // Chahe jitni baar getHtml() call karo, same Chrome instance reuse hota hai.
        std::string html2 = scraper.getHtml("https://example.com");
        std::cout << "Fetched page 2 (" << html2.size() << " bytes)\n";

        // scraper ke destructor mein Chrome apne aap band ho jayega.
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
