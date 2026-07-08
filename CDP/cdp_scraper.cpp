// cdp_scraper.cpp
//
// Full-page (JS-rendered) website scraper using Chrome DevTools Protocol (CDP).
// Launches headless Chrome, connects over WebSocket, navigates to the target
// URL, waits for the page (including JS) to finish loading, then dumps the
// fully rendered document.documentElement.outerHTML to a file.
//
// Build: see CMakeLists.txt in the same folder.
//
// Usage:
//   ./cdp_scraper <url> [output_file] [extra_wait_seconds] [chrome_binary]
//
// Example:
//   ./cdp_scraper https://example.com out.html 3 google-chrome
// run:
// ./cdp_scraper.exe "https://codequotient.com" output.html 3 "C:/Program Files/Google/Chrome/Application/chrome.exe"


#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXNetSystem.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif

using json = nlohmann::json;

// ---------- tiny libcurl HTTP GET helper ----------
static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

static std::string httpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl error: " << curl_easy_strerror(res) << "\n";
        }
        curl_easy_cleanup(curl);
    }
    return response;
}

static std::string httpPut(const std::string& url)
{
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            std::cerr << curl_easy_strerror(res) << std::endl;

        curl_easy_cleanup(curl);
    }

    return response;
}

// ---------- CDP client over WebSocket ----------
class CDPClient {
public:
    explicit CDPClient(const std::string& wsUrl) {
        ws_.setUrl(wsUrl);
        ws_.disableAutomaticReconnection();

        ws_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
            if (msg->type == ix::WebSocketMessageType::Message) {
                onMessage(msg->str);
            } else if (msg->type == ix::WebSocketMessageType::Open) {
                std::lock_guard<std::mutex> lk(mtx_);
                connected_ = true;
                cv_.notify_all();
            } else if (msg->type == ix::WebSocketMessageType::Error) {
                std::cerr << "WebSocket error: " << msg->errorInfo.reason << "\n";
            }
        });
    }

    void connect() {
        ws_.start();
        std::unique_lock<std::mutex> lk(mtx_);
        if (!cv_.wait_for(lk, std::chrono::seconds(10), [this] { return connected_; })) {
            throw std::runtime_error("Could not connect to Chrome DevTools WebSocket");
        }
    }

    // Send a CDP command, returns the request id.
    int send(const std::string& method, const json& params = json::object()) {
        int id = ++msgId_;
        json cmd = {{"id", id}, {"method", method}, {"params", params}};
        ws_.send(cmd.dump());
        return id;
    }

    // Block until the response for `id` arrives (or timeout).
    json waitForResult(int id, int timeoutMs = 30000) {
        std::unique_lock<std::mutex> lk(mtx_);
        bool ok = cv_.wait_for(lk, std::chrono::milliseconds(timeoutMs),
                                [this, id] { return results_.count(id) > 0; });
        if (!ok) throw std::runtime_error("Timeout waiting for CDP response id=" + std::to_string(id));
        json r = results_[id];
        results_.erase(id);
        return r;
    }

    // Block until a CDP *event* (not a command reply) with this method name arrives.
    json waitForEvent(const std::string& method, int timeoutMs = 30000) {
        std::unique_lock<std::mutex> lk(mtx_);
        bool ok = cv_.wait_for(lk, std::chrono::milliseconds(timeoutMs),
                                [this, &method] { return events_.count(method) > 0; });
        if (!ok) throw std::runtime_error("Timeout waiting for event " + method);
        json e = events_[method];
        events_.erase(method);
        return e;
    }

    void close() { ws_.stop(); }

private:
    void onMessage(const std::string& text) {
        try {
            json j = json::parse(text);
            std::lock_guard<std::mutex> lk(mtx_);
            if (j.contains("id")) {
                results_[j["id"].get<int>()] = j.contains("result") ? j["result"] : j;
            } else if (j.contains("method")) {
                events_[j["method"].get<std::string>()] = j.contains("params") ? j["params"] : json::object();
            }
            cv_.notify_all();
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse CDP message: " << e.what() << "\n";
        }
    }

    ix::WebSocket ws_;
    std::atomic<int> msgId_{0};
    std::mutex mtx_;
    std::condition_variable cv_;
    bool connected_ = false;
    std::map<int, json> results_;
    std::map<std::string, json> events_;
};

// ---------- helpers ----------
static bool waitForChromeEndpoint(const std::string& port, int retries = 30) {
    for (int i = 0; i < retries; ++i) {
        std::string resp = httpGet("http://localhost:" + port + "/json/version");
        if (!resp.empty()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    return false;
}

int main(int argc, char** argv) {
    

    std::string targetUrl = "https://codequotient.com";
    std::string outFile = "../pages/output.html";
    int extraWaitSeconds = 3;

    std::string chromeBin =
        "C:/Program Files/Google/Chrome/Application/chrome.exe";
    std::string port        = "9222";

    ix::initNetSystem();

    // 1) Launch headless Chrome with a throwaway profile so it doesn't clash
    //    with any already-running Chrome instance.
    #ifdef _WIN32
    int pid = _getpid();
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    std::string profileDir = std::string(tempPath) + "cdp_profile_" + std::to_string(pid);
#else
    int pid = getpid();
    std::string profileDir = "/tmp/cdp_profile_" + std::to_string(pid);
#endif
#ifdef _WIN32
    std::string launchCmd =
    "\"" + chromeBin + "\""
    " --headless=new"
    " --disable-gpu"
    " --remote-allow-origins=*"
    " --remote-debugging-port=" + port +
    " --user-data-dir=\"" + profileDir + "\" about:blank";

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::vector<char> cmd(launchCmd.begin(), launchCmd.end());
    cmd.push_back('\0');

    if (!CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE,
                        CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
    {
        std::cerr << "Failed to launch Chrome. Error = "
                  << GetLastError() << "\n";
        return 1;
    }
    CloseHandle(pi.hThread);
#else
    std::string launchCmd = chromeBin +
        " --headless=new --disable-gpu --no-sandbox"
        " --remote-debugging-port=" + port +
        " --user-data-dir=" + profileDir +
        " about:blank > /tmp/cdp_chrome_log_" + std::to_string(pid) + ".txt 2>&1 &";
    std::system(launchCmd.c_str());
#endif

    if (!waitForChromeEndpoint(port)) {
        std::cerr << "Chrome did not start / DevTools endpoint not reachable on port " << port << "\n";
        return 1;
    }

    try {
        // 2) Open a new tab/target via the HTTP endpoint, get its ws debugger URL.
        std::string newTabResp = httpPut("http://localhost:" + port + "/json/new");
        std::cout << "newTabResp = " << newTabResp << std::endl;
        json tabInfo = json::parse(newTabResp);
        std::string wsUrl = tabInfo["webSocketDebuggerUrl"].get<std::string>();

        // 3) Connect CDP over WebSocket.
        CDPClient client(wsUrl);
        client.connect();

        client.waitForResult(client.send("Page.enable"));
        client.waitForResult(client.send("Runtime.enable"));
        client.waitForResult(client.send("Network.enable"));

        // 4) Navigate, then wait for the load event (fires after initial JS/DOM settle).
        client.waitForResult(client.send("Page.navigate", {{"url", targetUrl}}));
        client.waitForEvent("Page.loadEventFired", 60000);

        // 5) Extra grace period for SPA frameworks (React/Vue/etc.) that keep
        //    fetching/rendering after the native "load" event fires.
        std::this_thread::sleep_for(std::chrono::seconds(extraWaitSeconds));

        // 6) Pull the live, rendered DOM out of the page.
        json evalResult = client.waitForResult(client.send("Runtime.evaluate", {
            {"expression", "document.documentElement.outerHTML"},
            {"returnByValue", true}
        }));

        if (!evalResult.contains("result") || !evalResult["result"].contains("value")) {
            std::cerr << "Runtime.evaluate did not return a value:\n" << evalResult.dump(2) << "\n";
            return 1;
        }

        std::string html = evalResult["result"]["value"].get<std::string>();

        std::ofstream out(outFile, std::ios::binary);
        out << html;
        out.close();

        std::cout << "Saved rendered HTML to " << outFile << " (" << html.size() << " bytes)\n";

        client.send("Page.close");
        client.close();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        #ifdef _WIN32
        TerminateProcess(pi.hProcess,0);
        CloseHandle(pi.hProcess);
#else
        std::system(("pkill -f \"" + profileDir + "\" >/dev/null 2>&1").c_str());
#endif
        return 1;
    }

    // 7) Cleanup: kill this specific Chrome instance (matched by its unique profile dir).
    #ifdef _WIN32
        TerminateProcess(pi.hProcess,0);
        CloseHandle(pi.hProcess);
#else
        std::system(("pkill -f \"" + profileDir + "\" >/dev/null 2>&1").c_str());
#endif
    return 0;
}
