// CDPScraper.h
//
// Ye class Chrome DevTools Protocol (CDP) use karke kisi bhi website ka
// JS-rendered (fully loaded) HTML nikal ke deti hai.
//
// Ye 3 kaam karti hai:
//   1. Headless Chrome ko launch karna        (constructor mein)
//   2. Kisi bhi link ka rendered HTML nikalna  (getHtml() function)
//   3. Chrome ko band karna                    (destructor mein, apne aap)
//
// Simple usage:
//
//   CDPScraper scraper;                          // Chrome start ho gaya
//   string html = scraper.getHtml("https://example.com");
//   // ... html use karo ...
//                                                 // scope khatam -> Chrome band

#pragma once

#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXNetSystem.h>
#include <nlohmann/json.hpp>

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

class CDPScraper {
public:
    // chromeBinary = chrome.exe ka path
    // port         = chrome debugging port (default 9222 hi rehne do)
    // extraWaitSeconds = load hone ke baad kitna extra ruko (SPA sites ke liye)
    CDPScraper(
        std::string chromeBinary =
#ifdef _WIN32
            "C:/Program Files/Google/Chrome/Application/chrome.exe",
#else
            "google-chrome",
#endif
        std::string port = "9222",
        int extraWaitSeconds = 3
    );

    // Destructor -> yahan Chrome ko band kiya jata hai
    ~CDPScraper();

    // Class ko copy karna allow nahi (kyunki isके andar ek live Chrome process hai)
    CDPScraper(const CDPScraper&) = delete;
    CDPScraper& operator=(const CDPScraper&) = delete;

    // ===== Main function =====
    // Link do, rendered HTML string mein wapas milega.
    // Jitni baar chaho call karo, Chrome dobara launch nahi hota, wahi reuse hota hai.
    std::string getHtml(const std::string& url);

private:
    // ---- Ye chhota helper class Chrome se WebSocket ke through baat karta hai ----
    // (CDP protocol commands bhejna aur unke response/events sunna)
    class CDPClient {
    public:
        explicit CDPClient(const std::string& wsUrl);

        void connect();                                    // websocket se connect karo
        int send(const std::string& method,
                 const nlohmann::json& params = nlohmann::json::object()); // command bhejo
        nlohmann::json waitForResult(int id, int timeoutMs = 30000);       // command ka jawab lo
        nlohmann::json waitForEvent(const std::string& method, int timeoutMs = 30000); // event ka wait karo
        void close();                                       // websocket band karo

    private:
        void onMessage(const std::string& text);            // aane wale har message ko handle karo

        ix::WebSocket ws_;
        std::atomic<int> msgId_{0};
        std::mutex mtx_;
        std::condition_variable cv_;
        bool connected_ = false;
        std::map<int, nlohmann::json> results_;      // command-id -> uska jawab
        std::map<std::string, nlohmann::json> events_; // event-name -> uska data
    };

    // ---- Chrome process manage karne wale helper functions ----
    void launchChrome();                       // headless chrome start karo
    void killChrome();                         // chrome band karo
    bool waitForChromeEndpoint(int retries = 30) const; // chrome ready hone ka wait karo

    // ---- Simple HTTP helpers (chrome ke debugging API se baat karne ke liye) ----
    static std::string httpGet(const std::string& url);
    static std::string httpPut(const std::string& url);
    static std::string httpDelete(const std::string& url);

    std::string chromeBin_;      // chrome.exe ka path
    std::string port_;           // debugging port
    int extraWaitSeconds_;       // extra wait time
    std::string profileDir_;     // is chrome instance ka temporary profile folder
    bool chromeRunning_ = false; // chrome chal raha hai ya nahi

#ifdef _WIN32
    PROCESS_INFORMATION pi_{};   // windows process handle
#else
    long chromePid_ = -1;        // linux/mac process id
#endif
};
