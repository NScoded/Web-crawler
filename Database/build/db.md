# Web Crawler Design Proposal - Version 2

## Overview

The Web Crawler recursively visits web pages starting from a seed URL, renders and downloads their HTML content, extracts hyperlinks, and stores each downloaded page for future processing.

The crawler is implemented in C++ using custom data structures such as `Queue`, `HashMap`, and `DynamicArray`. Modern web pages are rendered using **Chrome DevTools Protocol (CDP)**, allowing the crawler to process both static and JavaScript-based websites.

---

# Section 1 - Public API

The crawler is divided into independent modules to improve maintainability and allow future extensions without modifying the overall architecture.

## Crawler

```cpp
class Crawler
{
private:
    Downloader downloader;
    HTMLParser parser;
    Frontier frontier;
    HashMap<string, bool> visited;
    PageStorage storage;

    int depth;
    string seed;

public:
    Crawler();

    void crawler(string seed, int depth);

    string download(string url);

    void run();
};
```

Coordinates the complete crawling process by downloading pages, parsing links, managing the frontier, and storing downloaded pages.

---

## Frontier

```cpp
class Frontier
{
private:
    Queue<string> urlQueue;

public:
    Frontier();

    void push(const string& url);

    string pop();

    string front() const;

    bool empty() const;

    int size() const;
};
```

Maintains URLs waiting to be crawled using a FIFO queue.

---

## Downloader

```cpp
class Downloader
{
public:
    Downloader();

    std::string fetch(const std::string& url);
};
```

Uses **Chrome DevTools Protocol (CDP)** to launch a headless Chrome instance, render the webpage, execute JavaScript, and return the fully rendered HTML.

---

## HTMLParser

```cpp
class HTMLParser
{
public:
    DynamicArray<string> extractLinks(const string& html);
};
```

Extracts hyperlinks from the rendered HTML document.

---

## PageStorage

```cpp
class PageStorage
{
private:

    struct PageData
    {
        string url;
        string html;
        int depth;
    };

    DynamicArray<PageData> pages;

public:
    int storePage(const string& url,
                  const string& html,
                  int depth);

    string getPage(const string& url);

    bool hasPage(const string& url);

    int pageCount() const;
};
```

Stores rendered pages independently so they can be reused by future modules.

---

## Design Justification

Instead of placing all functionality inside a single crawler class, responsibilities are divided into dedicated modules.

- `Crawler` coordinates the crawling process.
- `Downloader` handles webpage rendering using Chrome DevTools Protocol (CDP).
- `HTMLParser` extracts hyperlinks from rendered HTML.
- `Frontier` manages URLs waiting to be crawled.
- `PageStorage` stores downloaded pages independently from the crawler.
- `HashMap` prevents duplicate crawling.

The modular design improves readability, testing, and future extensibility. Since browser-specific functionality is isolated inside the `Downloader`, a different rendering engine can be adopted later without modifying the remaining components.

---

# Section 2 - Internal Representation

## Frontier

Uses a custom Queue.

![Queue](images/Queue.jpg)

The queue stores URLs in FIFO order.

---

## Downloader

Uses **Chrome DevTools Protocol (CDP)** to communicate with a headless Chrome browser.

```
URL
  |
  v
Downloader
  |
  v
Chrome (CDP)
  |
Render JavaScript
  |
  v
Rendered HTML
```

This enables crawling of JavaScript-heavy websites where the original HTTP response does not contain the complete page content.

---

## Visited URL Store

Uses a custom HashMap.

![Visited](images/Visited.jpg)

The HashMap provides fast duplicate detection before downloading a page.

---

## Page Storage

Uses a custom DynamicArray.

![Page](images/Page_Store.jpg)

Each stored page contains

- URL
- Rendered HTML
- Crawling Depth

---

# Section 3 - Failure Handling

## Invalid URL

Invalid URLs are skipped and the crawler continues processing the remaining URLs.

---

## Duplicate URL

Before downloading a page, the crawler checks the HashMap. Already visited URLs are ignored.

---

## Browser Launch Failure

If Chrome cannot be launched or a CDP connection cannot be established, the crawler reports the error and continues crawling.

---

## Page Rendering Timeout

If page rendering exceeds the timeout limit, the page is skipped and crawling continues.

---

## Malformed HTML

The parser extracts all valid hyperlinks that can be identified. Invalid HTML is ignored without terminating the crawler.

---

## Empty Page

Empty HTML documents are stored successfully, although no hyperlinks are extracted.

---

# Section 4 - Complexity Analysis

## Frontier

Implemented using a Queue.

| Operation | Best | Average | Worst | Reason |
|-----------|:----:|:-------:|:-----:|--------|
| `push()` | O(1) | O(1) | O(1) | Inserts at the rear of the queue. |
| `pop()` | O(1) | O(1) | O(1) | Removes the front element. |
| `front()` | O(1) | O(1) | O(1) | Returns the front element. |
| `empty()` | O(1) | O(1) | O(1) | Checks whether the queue is empty. |
| `size()` | O(1) | O(1) | O(1) | Returns the maintained queue size. |

---

## Downloader

| Operation | Best | Average | Worst | Reason |
|-----------|:----:|:-------:|:-----:|--------|
| `fetch()` | O(n) | O(n) | O(n) | Runtime depends on the size of the rendered HTML document. |

---

## HTMLParser

| Operation | Best | Average | Worst | Reason |
|-----------|:----:|:-------:|:-----:|--------|
| `extractLinks()` | O(n) | O(n) | O(n) | Performs a linear scan of the HTML document. |

---

## Visited URL Store

| Operation | Best | Average | Worst | Reason |
|-----------|:----:|:-------:|:-----:|--------|
| `exists()` | O(1) | O(1) | O(n) | Average constant-time lookup; collisions increase runtime. |
| `insert()` | O(1) | O(1) | O(n) | Rehashing or excessive collisions increase runtime. |

---

## Page Storage

| Operation | Best | Average | Worst | Reason |
|-----------|:----:|:-------:|:-----:|--------|
| `storePage()` | O(1) | O(1) | O(n) | DynamicArray resizing copies existing pages. |
| `getPage()` | O(n) | O(n) | O(n) | Linear search by URL. |
| `hasPage()` | O(n) | O(n) | O(n) | Linear search. |
| `pageCount()` | O(1) | O(1) | O(1) | Returns the maintained page count. |

Here, **n** represents the size of the rendered HTML document.

---

# Section 5 - Future Compatibility

The crawler is designed using a modular architecture so that downloaded pages can be reused directly by future components such as an **Indexer**.

A dedicated `PageStorage` module stores every rendered webpage independently of the crawler. Each page is stored together with its URL, rendered HTML, and crawling depth.

The `PageStorage` interface is shown below.

```cpp
int storePage(const std::string& url,
              const std::string& html,
              int depth);

std::string getPage(const std::string& url);

bool hasPage(const std::string& url);

int pageCount() const;
```

### Compatibility with the Indexer

The Indexer only requires access to the rendered HTML pages and does not need to know how they were downloaded or rendered. Since JavaScript execution is completed before storage, the Indexer can process both static and dynamic websites without requiring any browser integration.

Future enhancements such as storing HTTP status codes, page titles, timestamps, metadata, compressed HTML, or replacing the storage implementation can be introduced inside the `PageStorage` module without affecting the crawler or the indexer.