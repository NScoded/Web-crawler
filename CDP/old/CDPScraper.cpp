// CDPScraper.cpp
// CDPScraper.h mein declare kiye gaye sab functions yahan define hain.

#include "CDPScraper.h"
#include <curl/curl.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#include <csignal>
#endif

using json = nlohmann::json;
using namespace std;

// ==========================================================
// PART 1: Chhote HTTP helpers
// Chrome ek local HTTP server bhi chalata hai (debugging ke liye),
// hum uske saath GET / PUT / DELETE requests se baat karte hain.
// ==========================================================

// Curl ko response likhne ke liye ek buffer chahiye, ye function wahi kaam karta hai
static size_t writeCallback(void* data, size_t size, size_t count, void* buffer) {
    static_cast<string*>(buffer)->append(static_cast<char*>(data), size * count);
    return size * count;
}

string CDPScraper::httpGet(const string& url) {
    CURL* curl = curl_easy_init();
    string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

string CDPScraper::httpPut(const string& url) {
    CURL* curl = curl_easy_init();
    string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

string CDPScraper::httpDelete(const string& url) {
    CURL* curl = curl_easy_init();
    string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

// ==========================================================
// PART 2: CDPClient
// Ye ek websocket ke through Chrome ko commands bhejta hai
// aur Chrome se aane wale replies/events ko store karta hai.
// ==========================================================

CDPScraper::CDPClient::CDPClient(const string& wsUrl) {
    ws_.setUrl(wsUrl);
    ws_.disableAutomaticReconnection();

    // Jab bhi Chrome se koi message aaye, isko handle karo
    ws_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            onMessage(msg->str);
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            lock_guard<mutex> lk(mtx_);
            connected_ = true;
            cv_.notify_all();
        } else if (msg->type == ix::WebSocketMessageType::Error) {
            cerr << "WebSocket error: " << msg->errorInfo.reason << "\n";
        }
    });
}

// Websocket connection start karke, connect hone tak wait karo
void CDPScraper::CDPClient::connect() {
    ws_.start();
    unique_lock<mutex> lk(mtx_);
    bool connected = cv_.wait_for(lk, chrono::seconds(10), [this] { return connected_; });
    if (!connected) {
        throw runtime_error("Chrome DevTools WebSocket se connect nahi ho paya");
    }
}

// Ek CDP command bhejo (jaise "Page.navigate"), uski id return hoti hai
int CDPScraper::CDPClient::send(const string& method, const json& params) {
    int id = ++msgId_;
    json command = {{"id", id}, {"method", method}, {"params", params}};
    ws_.send(command.dump());
    return id;
}

// Bheje gaye command ka jawab aane tak wait karo
json CDPScraper::CDPClient::waitForResult(int id, int timeoutMs) {
    unique_lock<mutex> lk(mtx_);
    bool gotResult = cv_.wait_for(lk, chrono::milliseconds(timeoutMs),
                                   [this, id] { return results_.count(id) > 0; });
    if (!gotResult) {
        throw runtime_error("CDP response ka wait karte hue timeout ho gaya (id=" + to_string(id) + ")");
    }
    json result = results_[id];
    results_.erase(id);
    return result;
}

// Kisi particular event (jaise "Page.loadEventFired") ka wait karo
json CDPScraper::CDPClient::waitForEvent(const string& method, int timeoutMs) {
    unique_lock<mutex> lk(mtx_);
    bool gotEvent = cv_.wait_for(lk, chrono::milliseconds(timeoutMs),
                                  [this, &method] { return events_.count(method) > 0; });
    if (!gotEvent) {
        throw runtime_error("Event ka wait karte hue timeout ho gaya: " + method);
    }
    json event = events_[method];
    events_.erase(method);
    return event;
}

void CDPScraper::CDPClient::close() {
    ws_.stop();
}

// Chrome se aaya message ya to kisi command ka "result" hota hai,
// ya phir apne aap fire hua koi "event" hota hai. Dono ko yahan store karte hain.
void CDPScraper::CDPClient::onMessage(const string& text) {
    try {
        json msg = json::parse(text);
        lock_guard<mutex> lk(mtx_);

        if (msg.contains("id")) {
            // ye kisi command ka jawab hai
            results_[msg["id"].get<int>()] = msg.contains("result") ? msg["result"] : msg;
        } else if (msg.contains("method")) {
            // ye apne aap hua ek event hai
            events_[msg["method"].get<string>()] = msg.contains("params") ? msg["params"] : json::object();
        }
        cv_.notify_all();
    } catch (const exception& e) {
        cerr << "CDP message parse nahi ho paya: " << e.what() << "\n";
    }
}

// ==========================================================
// PART 3: CDPScraper (main class)
// ==========================================================

CDPScraper::CDPScraper(string chromeBinary, string port, int extraWaitSeconds)
    : chromeBin_(chromeBinary),
      port_(port),
      extraWaitSeconds_(extraWaitSeconds) {
    ix::initNetSystem();
    launchChrome();
}

CDPScraper::~CDPScraper() {
    killChrome();
}

// Chrome ka debugging port ready hone tak baar-baar check karo
bool CDPScraper::waitForChromeEndpoint(int retries) const {
    for (int i = 0; i < retries; i++) {
        string response = httpGet("http://localhost:" + port_ + "/json/version");
        if (!response.empty()) {
            return true; // chrome ready hai
        }
        this_thread::sleep_for(chrono::milliseconds(300));
    }
    return false;
}

// Headless Chrome ko ek naye temporary profile ke saath start karo
void CDPScraper::launchChrome() {
#ifdef _WIN32
    int pid = _getpid();
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    profileDir_ = string(tempPath) + "cdp_profile_" + to_string(pid);

    string launchCmd =
        "\"" + chromeBin_ + "\""
        " --headless=new"
        " --disable-gpu"
        " --remote-allow-origins=*"
        " --remote-debugging-port=" + port_ +
        " --user-data-dir=\"" + profileDir_ + "\" about:blank";

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    pi_ = PROCESS_INFORMATION{};

    vector<char> cmd(launchCmd.begin(), launchCmd.end());
    cmd.push_back('\0');

    bool started = CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE,
                                   CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi_);
    if (!started) {
        throw runtime_error("Chrome launch nahi ho paya (error code " + to_string(GetLastError()) + ")");
    }
    CloseHandle(pi_.hThread);
#else
    int pid = getpid();
    profileDir_ = "/tmp/cdp_profile_" + to_string(pid);

    string launchCmd = chromeBin_ +
        " --headless=new --disable-gpu --no-sandbox"
        " --remote-debugging-port=" + port_ +
        " --user-data-dir=" + profileDir_ +
        " about:blank > /tmp/cdp_chrome_log_" + to_string(pid) + ".txt 2>&1 & echo $!";

    // launch karke uska process id capture kar lo (baad mein band karne ke liye)
    FILE* pipe = popen(launchCmd.c_str(), "r");
    if (!pipe) {
        throw runtime_error("Chrome launch nahi ho paya");
    }
    char buf[64] = {0};
    if (fgets(buf, sizeof(buf), pipe)) {
        chromePid_ = atol(buf);
    }
    pclose(pipe);
#endif

    if (!waitForChromeEndpoint()) {
        throw runtime_error("Chrome start toh hua par port " + port_ + " par ready nahi hua");
    }

    chromeRunning_ = true;
}

// Chrome process ko band kar do
void CDPScraper::killChrome() {
    if (!chromeRunning_) return;

#ifdef _WIN32
    if (pi_.hProcess) {
        TerminateProcess(pi_.hProcess, 0);
        CloseHandle(pi_.hProcess);
    }
#else
    if (chromePid_ > 0) {
        kill((pid_t)chromePid_, SIGKILL);
    }
#endif

    chromeRunning_ = false;
}

// ===== Ye hi wo main function hai jo user use karega =====
string CDPScraper::getHtml(const string& url) {
    if (!chromeRunning_) {
        throw runtime_error("Chrome chal nahi raha");
    }

    // Step 1: Is request ke liye ek naya tab kholo
    string newTabResponse = httpPut("http://localhost:" + port_ + "/json/new");
    json tabInfo = json::parse(newTabResponse);
    string wsUrl = tabInfo["webSocketDebuggerUrl"].get<string>();
    string tabId = tabInfo.value("id", "");

    string html;

    try {
        // Step 2: Us tab se websocket connection banao
        CDPClient client(wsUrl);
        client.connect();

        client.waitForResult(client.send("Page.enable"));
        client.waitForResult(client.send("Runtime.enable"));
        client.waitForResult(client.send("Network.enable"));

        // Step 3: Diye gaye link par jao aur page load hone ka wait karo
        client.waitForResult(client.send("Page.navigate", {{"url", url}}));
        client.waitForEvent("Page.loadEventFired", 60000);

        // Step 4: Kuch extra second ruko (JS wali sites ke liye, taki content aa jaye)
        this_thread::sleep_for(chrono::seconds(extraWaitSeconds_));

        // Step 5: Page ka poora rendered HTML nikal lo
        json result = client.waitForResult(client.send("Runtime.evaluate", {
            {"expression", "document.documentElement.outerHTML"},
            {"returnByValue", true}
        }));

        if (!result.contains("result") || !result["result"].contains("value")) {
            throw runtime_error("HTML nikal nahi paya");
        }

        html = result["result"]["value"].get<string>();
        client.close();
    }
    catch (...) {
        // Kuch bhi galat ho, tab ko band zaroor karo
        if (!tabId.empty()) {
            httpDelete("http://localhost:" + port_ + "/json/close/" + tabId);
        }
        throw; // error ko aage bhej do
    }

    // Step 6: Tab band kar do (Chrome process chalta rahega, agli call ke liye)
    if (!tabId.empty()) {
        httpDelete("http://localhost:" + port_ + "/json/close/" + tabId);
    }

    return html;
}
