# Web Crawler Design Proposal - Version 1

## Overview

The Web Crawler recursively visits web pages starting from a seed URL, downloads their HTML content, extracts hyperlinks, and stores each downloaded page for future processing. The crawler is implemented in C++ using custom data structures such as `Queue`, `HashMap`, and `DynamicArray`, while `libcurl` is used for HTTP communication.

---

# Section 1 - Public API

The crawler is divided into independent modules to improve maintainability and allow future extensions without modifying the overall architecture.

```cpp

class Crawler{
    void run();
    void crawler(string seed,int depth);
    string download(string url);
    DynamicArray<string> parser
    public:
    int depth;
    string seed;
    Crawler();

}

```