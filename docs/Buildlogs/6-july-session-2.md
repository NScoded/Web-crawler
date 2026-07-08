# Build Log – 6 July 2026 - Session 2


## Part 1 – Research on Alternative Web Crawling Approaches

After identifying the limitations of the **libcurl-based HTML fetching module**, the next task was to investigate alternative techniques capable of handling JavaScript-rendered websites.

### Objective

Identify a solution that can retrieve fully rendered webpage content while remaining compatible with the project's C++ architecture.

### Research Conducted

The following approaches were studied and evaluated:

#### 1. Backend API Extraction

* Researched extracting website data by directly consuming backend API endpoints.
* Studied how browser Developer Tools can be used to inspect network traffic and identify API requests.
* Evaluated the feasibility of building a crawler around API responses.

#### 2. Browser Automation using Puppeteer

* Investigated Puppeteer as a browser automation framework.
* Studied its workflow for launching Chromium, executing JavaScript, waiting for page rendering, and extracting the final DOM.
* Evaluated its integration possibilities with the existing project.

#### 3. Chromium Embedded Framework (CEF)

* Researched Chromium Embedded Framework (CEF) as a native browser embedding solution.
* Studied its rendering capabilities, JavaScript execution model, and native C++ integration.
* Evaluated the implementation complexity and runtime overhead associated with embedding Chromium.

#### 4. Chrome DevTools Protocol (CDP)

* Investigated Chrome DevTools Protocol as a low-level interface for controlling Chromium.
* Studied its architecture, communication model, browser targets, and supported commands.
* Evaluated its suitability for rendering dynamic webpages without embedding an entire browser framework.

### Outcome

The research provided a clear understanding of the advantages, limitations, and trade-offs of each approach. These findings established the foundation for selecting the most appropriate rendering strategy for the web crawler.

---


## Part 2 - Research on Crawler Architecture

Along with rendering techniques, I also started designing the overall architecture of the crawler before beginning the implementation.

### Frontier Design

The **Frontier** will be implemented using a **Queue** so that URLs are processed in the order they are discovered. This naturally follows a **Breadth-First Search (BFS)** strategy, allowing the crawler to visit webpages level by level.

Another important responsibility of the Frontier will be **URL Normalization**.

Before inserting any URL into the queue, it will first be normalized. Different representations of the same webpage (such as trailing slashes, relative URLs, fragments, etc.) should map to a single canonical URL. This prevents duplicate crawling and keeps the frontier clean.

### Downloader

Once a URL is removed from the Frontier, it will be passed to the **Downloader** module.

The Downloader will be responsible for fetching the webpage and returning the **fully rendered HTML**, instead of just the initial HTML source. This ensures that websites using client-side rendering can also be crawled correctly.

### Page Store

Rather than directly parsing the downloaded HTML, I decided to introduce a **Page Store** module.

The current plan is to use **SQLite** as the storage engine because it is lightweight, file-based, and integrates well with C++. The rendered HTML, along with its corresponding URL, will be stored in the database before further processing.

This design separates downloading from parsing, making the crawler easier to maintain and extend.

### HTML Parsing

After the page is stored, the parser will read the HTML, extract hyperlinks, and send newly discovered URLs back to the Frontier after normalization.

### Crawl Depth

To control the crawling depth, I plan to use **recursion**.

Each recursive call will represent the next depth level, while a configurable maximum depth will prevent the crawler from traversing indefinitely.

---



## Outcome

By the end of this session, the high-level design of the crawler was finalized. The responsibilities of the Frontier, Downloader, Page Store, and Parser were clearly defined, providing a structured roadmap for the implementation phase.
