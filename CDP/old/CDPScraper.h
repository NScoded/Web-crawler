// CDPScraper.h
//
// Ye class Chrome DevTools Protocol (CDP) use karke kisi bhi website ka
// JS-rendered (fully loaded) HTML nikal ke deti hai.
//
// UPDATED VERSION - purani version se ye cheezein badli hain (speed ke liye):
//   1. Har getHtml() call par naya tab + naya websocket handshake NAHI hota.
//      Ab ek chhota "tab pool" banta hai constructor mein hi (default 4 tabs),
//      aur getHtml() unhi tabs ko baar baar reuse karta hai. Isse per-page
//      overhead (tab create + websocket handshake) khatam ho jata hai.
//   2. Fixed "sleep(extraWaitSeconds)" har page ke baad hataya gaya hai.
//      Ab hum Chrome ke Network events sunte hain aur jab tak network
//      "idle" (koi naya request pending nahi) nahi ho jata tab tak wait
//      karte hain - lekin ek max cap ke andar. Static/simple pages bahut
//      jaldi return honge, sirf heavy JS pages hi zyada wait karengi.
//   3. Images/fonts/media requests CDP se hi block kar diye jaate hain
//      (Network.setBlockedURLs), kyunki humein sirf HTML/DOM chahiye -
//      isse page load time bahut kam ho jata hai.
//   4. Pool me multiple tabs hone ki wajah se getHtml() ko alag alag
//      threads se bhi call kiya ja sakta hai -> parallel crawling possible.
//
// Simple usage (single page):
//
//   CDPScraper scraper;                          // Chrome start ho gaya (pool bhi)
//   string html = scraper.getHtml("https://example.com");
//
// Parallel usage (multiple threads):
//
//   CDPScraper scraper(chromePath, "9222", 3 /*maxWaitSeconds*/, 4 /*tabs*/);
//   // ab 4 threads ek saath scraper.getHtml(url) call kar sakte hain

#pragma once

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#endif

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>

#include "../include/STL/DynamicArray.h"

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

class CDPScraper {
public:
    // chromeBinary      = chrome.exe ka path
    // port              = chrome debugging port (default 9222 hi rehne do)
    // maxWaitSeconds    = network idle ka wait karne ki upper limit (worst case cap)
    // numTabs           = kitne tabs pool me rakhne hain (parallel fetch ke liye)
    // blockImagesEtc    = images/fonts/media block karke fetch fast karna hai ya nahi
    CDPScraper(
        std::string chromeBinary =
#ifdef _WIN32
            "C:/Program Files/Google/Chrome/Application/chrome.exe",
#else
            "google-chrome",
#endif
        std::string port = "9222",
        int maxWaitSeconds = 3,
        int numTabs = 4,
        bool blockImagesEtc = true
    );

    // Close all tabs
    
    ~CDPScraper();

    // Class ko copy karna allow nahi (kyunki iske andar live Chrome process/tabs hain)
    CDPScraper(const CDPScraper&) = delete;
    CDPScraper& operator=(const CDPScraper&) = delete;

    // start point - mulitple thread pools
    std::string getHtml(const std::string& url);

private:
    
    class CDPClient {
    public:
        explicit CDPClient(const std::string& wsUrl);

        void connect();
        int send(const std::string& method,
                 const nlohmann::json& params = nlohmann::json::object());
        nlohmann::json waitForResult(int id, int timeoutMs = 30000);
        nlohmann::json waitForEvent(const std::string& method, int timeoutMs = 30000);

        // Naya: network requests ke khatam hone ka wait (blind sleep ki jagah).
        // idleMs      = kitni der tak koi naya pending request na ho to "idle" maan lo
        // maxWaitMs   = worst case kitni der tak hi wait karna hai (safety cap)
        void waitForNetworkIdle(int idleMs, int maxWaitMs);

        void resetNetworkCounters();
        void close();

    private:
        void onMessage(const std::string& text);

        ix::WebSocket ws_;
        std::atomic<int> msgId_{0};
        std::mutex mtx_;
        std::condition_variable cv_;
        bool connected_ = false;
        std::map<int, nlohmann::json> results_;
        std::map<std::string, nlohmann::json> events_;

        // pending network requests ka live counter (idle-detection ke liye)
        int pendingRequests_ = 0;
    };

    // Ek tab + uska CDP client
    struct Tab {
        std::string tabId;
        std::string wsUrl;
        std::unique_ptr<CDPClient> client;
    };

    // ---- Chrome process manage karne wale helper functions ----
    void launchChrome();
    void killChrome();
    bool waitForChromeEndpoint(int retries = 30) const;

    // ---- Ek naya tab kholo, websocket se connect karo, resource-blocking on karo ----
    // Note: Tab* (raw, heap-owned) return karta hai, unique_ptr nahi -
    // DynamicArray<T> sirf copyable T rakh sakta hai (move-only types jaise
    // unique_ptr nahi), isliye pool khud hi Tab* ka owner hai.
    Tab* createTab();
    void closeTabRemote(const std::string& tabId); // Chrome side se tab band karo

    // ---- Pool se ek free tab lo / wapas do ----
    Tab* acquireTab();
    void releaseTab(Tab* tab);

    // ---- Simple HTTP helpers (chrome ke debugging API se baat karne ke liye) ----
    static std::string httpGet(const std::string& url);
    static std::string httpPut(const std::string& url);
    static std::string httpDelete(const std::string& url);

    std::string chromeBin_;
    std::string port_;
    int maxWaitSeconds_;
    int numTabs_;
    bool blockImagesEtc_;
    std::string profileDir_;
    bool chromeRunning_ = false;

    // Tab pool
    // std::vector ki jagah apni DynamicArray - pool_ Tab* (raw, heap-owned)
    // rakhta hai; destructor mein sab explicitly delete kiye jaate hain.
    std::mutex poolMutex_;
    std::condition_variable poolCv_;
    DynamicArray<Tab*> pool_;

#ifdef _WIN32
    PROCESS_INFORMATION pi_{};
#else
    long chromePid_ = -1;
#endif
};
