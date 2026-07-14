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
// Chrome ko "lean" mode me chalane wale flags.
//
// Ye GCM (Google Cloud Messaging) registration attempts, sync, telemetry,
// update-checks, extensions, first-run wizard, jaisi cheezein band karte
// hain jo normal Chrome background me karta hai. Isi background activity
// ki wajah se aapko wo "DEPRECATED_ENDPOINT" / registration_request.cc
// wale error print ho rahe the - wo crawling ko fail nahi karte (harmless
// stderr noise hain) lekin CPU/network cycles chura kar crawler ko
// dheema kar sakte hain, khaas kar jab hazaro pages crawl ho rahe ho.
// ==========================================================
static const char* CHROME_LEAN_FLAGS =
    " --disable-background-networking"
    " --disable-background-timer-throttling"
    " --disable-backgrounding-occluded-windows"
    " --disable-renderer-backgrounding"
    " --disable-ipc-flooding-protection"
    " --disable-client-side-phishing-detection"
    " --disable-component-update"
    " --disable-domain-reliability"
    " --disable-sync"
    " --disable-translate"
    " --disable-default-apps"
    " --disable-extensions"
    " --disable-notifications"
    " --disable-hang-monitor"
    " --disable-prompt-on-repost"
    " --disable-features=Translate,OptimizationHints,MediaRouter,AutofillServerCommunication"
    " --metrics-recording-only"
    " --mute-audio"
    " --no-first-run"
    " --no-default-browser-check"
    " --password-store=basic"
    " --use-mock-keychain"
    " --blink-settings=imagesEnabled=false";

// ==========================================================
// PART 1: Chhote HTTP helpers
// ==========================================================

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
// ==========================================================

CDPScraper::CDPClient::CDPClient(const string& wsUrl) {
    ws_.setUrl(wsUrl);
    ws_.disableAutomaticReconnection();

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

void CDPScraper::CDPClient::connect() {
    ws_.start();
    unique_lock<mutex> lk(mtx_);
    bool connected = cv_.wait_for(lk, chrono::seconds(10), [this] { return connected_; });
    if (!connected) {
        throw runtime_error("Chrome DevTools WebSocket se connect nahi ho paya");
    }
}

int CDPScraper::CDPClient::send(const string& method, const json& params) {
    int id = ++msgId_;
    json command = {{"id", id}, {"method", method}, {"params", params}};
    ws_.send(command.dump());
    return id;
}

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

// Naya: blind sleep ki jagah - Network.requestWillBeSent / loadingFinished /
// loadingFailed events se hum pending requests ka live count rakhte hain.
// Jaise hi ye count kuch der (idleMs) tak 0 rehta hai, hum maan lete hain
// ke page "settle" ho chuka hai. maxWaitMs ek hard safety cap hai taaki
// koi bhi page hamesha ke liye latka na rahe.
void CDPScraper::CDPClient::waitForNetworkIdle(int idleMs, int maxWaitMs) {
    using clock = chrono::steady_clock;
    auto start = clock::now();
    unique_lock<mutex> lk(mtx_);

    while (true) {
        // pending==0 hone tak (ya maxWait khatam hone tak) wait karo
        bool idleNow = cv_.wait_for(lk, chrono::milliseconds(50),
                                     [this] { return pendingRequests_ <= 0; });

        auto elapsed = chrono::duration_cast<chrono::milliseconds>(clock::now() - start).count();
        if (elapsed >= maxWaitMs) return; // safety cap - ab aur wait nahi

        if (idleNow) {
            // thoda aur ruk kar confirm karo ke sach me idle hai (debounce)
            bool stillIdle = cv_.wait_for(lk, chrono::milliseconds(idleMs),
                                           [this] { return pendingRequests_ > 0; });
            if (!stillIdle) return; // idleMs tak koi naya request nahi aaya -> settle ho gaya
            // agar isi debounce ke beech naya request aa gaya, to upar wapas loop karo
        }
    }
}

void CDPScraper::CDPClient::resetNetworkCounters() {
    lock_guard<mutex> lk(mtx_);
    pendingRequests_ = 0;
}

void CDPScraper::CDPClient::close() {
    ws_.stop();
}

void CDPScraper::CDPClient::onMessage(const string& text) {
    try {
        json msg = json::parse(text);
        lock_guard<mutex> lk(mtx_);

        if (msg.contains("id")) {
            results_[msg["id"].get<int>()] = msg.contains("result") ? msg["result"] : msg;
        } else if (msg.contains("method")) {
            string method = msg["method"].get<string>();
            events_[method] = msg.contains("params") ? msg["params"] : json::object();

            // network idle-detection ke liye live counters
            if (method == "Network.requestWillBeSent") {
                pendingRequests_++;
            } else if (method == "Network.loadingFinished" || method == "Network.loadingFailed") {
                if (pendingRequests_ > 0) pendingRequests_--;
            }
        }
        cv_.notify_all();
    } catch (const exception& e) {
        cerr << "CDP message parse nahi ho paya: " << e.what() << "\n";
    }
}

// ==========================================================
// PART 3: CDPScraper (main class)
// ==========================================================

CDPScraper::CDPScraper(string chromeBinary, string port, int maxWaitSeconds, int numTabs, bool blockImagesEtc)
    : chromeBin_(chromeBinary),
      port_(port),
      maxWaitSeconds_(maxWaitSeconds),
      numTabs_(numTabs > 0 ? numTabs : 1),
      blockImagesEtc_(blockImagesEtc) {
    ix::initNetSystem();
    launchChrome();

    // Pool banao: sab tabs upfront khol lo taaki getHtml() ke waqt
    // per-request tab-creation/websocket-handshake overhead na lage.
    for (int i = 0; i < numTabs_; i++) {
        pool_.push_back(createTab());
    }
}

CDPScraper::~CDPScraper() {
    for (auto& tab : pool_) {
        if (tab && tab->client) {
            tab->client->close();
            closeTabRemote(tab->tabId);
        }
    }
    pool_.clear();
    killChrome();
}

bool CDPScraper::waitForChromeEndpoint(int retries) const {
    for (int i = 0; i < retries; i++) {
        string response = httpGet("http://localhost:" + port_ + "/json/version");
        if (!response.empty()) {
            return true;
        }
        this_thread::sleep_for(chrono::milliseconds(300));
    }
    return false;
}

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
        " --user-data-dir=\"" + profileDir_ + "\""
        + CHROME_LEAN_FLAGS +
        " about:blank";

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
        CHROME_LEAN_FLAGS +
        " about:blank > /tmp/cdp_chrome_log_" + to_string(pid) + ".txt 2>&1 & echo $!";

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

// Chrome side se ek naya tab band karo (debugging HTTP API se)
void CDPScraper::closeTabRemote(const string& tabId) {
    if (!tabId.empty()) {
        httpDelete("http://localhost:" + port_ + "/json/close/" + tabId);
    }
}

// Ek naya tab kholo, websocket connect karo, aur ek baar ke setup commands
// (Page/Runtime/Network enable + resource blocking) bhi yahin kar do -
// taaki ye sab baar baar (har getHtml call par) na karna pade.
unique_ptr<CDPScraper::Tab> CDPScraper::createTab() {
    string newTabResponse = httpPut("http://localhost:" + port_ + "/json/new");
    json tabInfo = json::parse(newTabResponse);

    auto tab = make_unique<Tab>();
    tab->wsUrl = tabInfo["webSocketDebuggerUrl"].get<string>();
    tab->tabId = tabInfo.value("id", "");

    tab->client = make_unique<CDPClient>(tab->wsUrl);
    tab->client->connect();

    tab->client->waitForResult(tab->client->send("Page.enable"));
    tab->client->waitForResult(tab->client->send("Runtime.enable"));
    tab->client->waitForResult(tab->client->send("Network.enable"));

    if (blockImagesEtc_) {
        // Sirf HTML/DOM chahiye - images, fonts, media, stylesheets sab
        // block kar do taaki page bahut tez load ho. (Isse content JS
        // execution/DOM par koi asar nahi padta, bas heavy assets skip ho jaate hain.)
        json blocked = {
            "*.png", "*.jpg", "*.jpeg", "*.gif", "*.webp", "*.svg", "*.ico",
            "*.woff", "*.woff2", "*.ttf", "*.otf", "*.eot",
            "*.mp4", "*.webm", "*.mp3", "*.wav", "*.avi", "*.mov"
        };
        tab->client->waitForResult(tab->client->send("Network.setBlockedURLs", {{"urls", blocked}}));
    }

    return tab;
}

// Pool se ek free tab uthao. Agar sab busy hain to koi tab release hone
// tak wait karo (busy-wait ki jagah condition_variable).
unique_ptr<CDPScraper::Tab> CDPScraper::acquireTab() {
    unique_lock<mutex> lk(poolMutex_);
    poolCv_.wait(lk, [this] { return !pool_.empty(); });
    unique_ptr<Tab> tab = std::move(pool_.back());
    pool_.pop_back();
    return tab;
}

void CDPScraper::releaseTab(unique_ptr<Tab> tab) {
    lock_guard<mutex> lk(poolMutex_);
    pool_.push_back(std::move(tab));
    poolCv_.notify_one();
}

// ===== Ye hi wo main function hai jo user use karega =====
string CDPScraper::getHtml(const string& url) {
    if (!chromeRunning_) {
        throw runtime_error("Chrome chal nahi raha");
    }

    unique_ptr<Tab> tab = acquireTab();
    string html;

    try {
        CDPClient& client = *tab->client;
        client.resetNetworkCounters();

        client.waitForResult(client.send("Page.navigate", {{"url", url}}));
        client.waitForEvent("Page.loadEventFired", 60000);

        // Blind fixed sleep ki jagah: jab tak network requests khatam
        // (ya maxWaitSeconds_ ka cap) na ho jaaye, tab tak wait karo.
        client.waitForNetworkIdle(/*idleMs=*/150, /*maxWaitMs=*/maxWaitSeconds_ * 1000);

        json result = client.waitForResult(client.send("Runtime.evaluate", {
            {"expression", "document.documentElement.outerHTML"},
            {"returnByValue", true}
        }));

        if (!result.contains("result") || !result["result"].contains("value")) {
            throw runtime_error("HTML nikal nahi paya");
        }

        html = result["result"]["value"].get<string>();
    }
    catch (...) {
        releaseTab(std::move(tab));
        throw;
    }

    releaseTab(std::move(tab));
    return html;
}
