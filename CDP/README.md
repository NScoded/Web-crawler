# CDP Full-Page Scraper (C++)

Headless Chrome + Chrome DevTools Protocol (CDP) se JS-rendered website ka
poora HTML nikaal ke file me save karta hai — static `curl` fetch jaisa nahi,
poora DOM after JavaScript execution.

## Kaise kaam karta hai

1. `google-chrome --headless=new --remote-debugging-port=9222` se Chrome start hota hai (throwaway profile ke saath).
2. `http://localhost:9222/json/new` HTTP endpoint se ek naya tab open hota hai, jiska `webSocketDebuggerUrl` milta hai.
3. Us WebSocket URL par CDP JSON-RPC commands bheje jaate hain: `Page.enable`, `Runtime.enable`, `Page.navigate`.
4. `Page.loadEventFired` event ka wait karte hain (matlab page + initial JS load ho chuka).
5. Kuch extra seconds (SPA/React/Vue jaise frameworks ke liye) wait karte hain.
6. `Runtime.evaluate` se `document.documentElement.outerHTML` nikaal ke file me likh dete hain.

## Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential cmake libcurl4-openssl-dev pkg-config

# Google Chrome (agar already installed nahi hai)
wget -q -O - https://dl.google.com/linux/linux_signing_key.pub | sudo apt-key add -
echo "deb [arch=amd64] http://dl.google.com/linux/chrome/deb/ stable main" | sudo tee /etc/apt/sources.list.d/google-chrome.list
sudo apt update && sudo apt install -y google-chrome-stable

# nlohmann/json (header-only)
sudo apt install -y nlohmann-json3-dev

# IXWebSocket (build from source, no apt package on most distros)
git clone https://github.com/machinezone/IXWebSocket.git /tmp/IXWebSocket
cd /tmp/IXWebSocket
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_TLS=OFF
make -j$(nproc)
sudo make install
sudo ldconfig
```

> Agar Chromium use karna hai Chrome ki jagah, `chromium-browser` install karo
> aur program chalate waqt 4th argument me binary ka naam de do.

## Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Run

```bash
./cdp_scraper "https://example.com" output.html 3 google-chrome
```

Arguments:
1. `url` — target website (required)
2. `output_file` — kahan save karna hai (default: `output.html`)
3. `extra_wait_seconds` — Page.loadEventFired ke baad kitna extra wait karna hai SPA rendering ke liye (default: 3)
4. `chrome_binary` — `google-chrome` / `chromium-browser` / full path (default: `google-chrome`)

## Notes / Limitations (important — production me improve karna)

- **Fixed wait, not true "network idle" detection.** Abhi bas `extra_wait_seconds` sleep karta hai. Better approach: `Network.enable` se request/response events track karo aur jab tak N seconds tak koi naya network request na aaye tab tak wait karo (real "network idle" logic).
- **Single page only.** Agar crawl karna hai (links follow karke pura site), tu ye binary ek loop me har URL ke liye call kar sakta hai, ya program ke andar hi HTML parse karke (e.g. Gumbo/libxml2 se) `<a href>` nikaal ke queue bana sakta hai.
- **No screenshot/PDF support** abhi — agar chahiye to `Page.captureScreenshot` / `Page.printToPDF` commands add kar sakte hain, same CDPClient class use karke.
- **--no-sandbox** flag diya hai kyunki container/CI environments me sandbox fail hota hai; apne desktop par isko hata sakta hai security ke liye.
- Chrome process ko uske unique `--user-data-dir` se match karke `pkill` se kill kiya jaa raha hai cleanup me — parallel multiple scrapes chalane par bhi safe rahega.
- Agar site bot-detection use karta hai (Cloudflare etc.), headless Chrome detect ho sakta hai — usko bypass karna is scope se bahar hai.

## Extending: Robots.txt / crawling / legality

Agar poora website "scan karke download" karna hai (multiple pages), respect
`robots.txt` aur target site ki Terms of Service — rate-limit lagao taaki
server par load na pade.
