# Build Log – 6 July 2026 - Session 1

## Objective

Implement an initial webpage fetching module using **libcurl** and evaluate its suitability for downloading webpage content for the web crawler.

---

## Implementation

Implemented a `fetch()` function using **libcurl** to send HTTP GET requests and download the HTML source of a webpage.

### Approach

* Integrated **libcurl** into the C++ project.
* Configured HTTP requests using the libcurl easy interface.
* Implemented a write callback function to collect the server response into a string buffer.
* Designed the `fetch()` function to return the downloaded HTML content for further parsing.
* Verified the implementation by testing it on multiple websites.

---

## Testing

The implementation was tested against both static and modern web applications.

### Results

* Successfully downloaded HTML from static websites.
* HTTP requests completed successfully with valid response codes.
* The downloaded content could be stored and processed without issues.

However, testing modern websites built with frameworks such as **React** revealed a major limitation.

---

## Issue Encountered

For websites using **client-side rendering (CSR)**, the HTML returned by libcurl did not contain the actual webpage content.

Instead, the response primarily consisted of:

* A minimal HTML document.
* Root container elements (e.g., `<div id="root">`).
* References to JavaScript bundles and assets.

The application data and UI components were generated only after the JavaScript code executed inside a browser environment. Since libcurl functions purely as an HTTP client and does not execute JavaScript, it was unable to retrieve the fully rendered webpage.

---

## Root Cause Analysis

The limitation originated from the architectural differences between traditional server-rendered websites and modern single-page applications (SPAs).

* **libcurl** performs only the network request and downloads the server response.
* It does not provide:

  * JavaScript execution.
  * DOM rendering.
  * Browser APIs.
  * Client-side rendering capabilities.

As a result, websites built using frameworks such as React deliver only the initial application shell, while the actual content is generated dynamically within the browser after page load.

---

## Design Proposal

Alongside the implementation and testing of the fetching module, I also prepared **Version 1 (V1)** of the project's design proposal.

The initial design documented the overall crawler architecture, major software components, data flow, and the interaction between different modules such as the Frontier, Downloader, HTML Parser, and Page Store. This served as the baseline architecture, which would later be refined as new implementation challenges and design decisions emerged.

---

## Design Evaluation

### Advantages

* Lightweight implementation.
* Fast HTTP request execution.
* Simple integration with the existing C++ codebase.
* Well suited for crawling static websites.

### Limitations

* Unable to execute JavaScript.
* Cannot retrieve dynamically rendered content.
* Incompatible with modern client-side rendered web applications.

---

## Conclusion

The libcurl-based fetching module functioned correctly from a networking perspective and successfully downloaded static HTML. However, it **failed to extract complete webpage content** from JavaScript-driven websites because it lacked browser rendering capabilities.

Since supporting modern websites was a core requirement of the web crawler, this approach was deemed insufficient as the primary crawling strategy. Consequently, further research was initiated into browser-based rendering solutions such as Backend API extraction, browser automation, and the **Chrome DevTools Protocol (CDP)**.
