# Build Log – 7 July 2026 (Session 2)

**Project:** Web Crawler   
**Module:** Chrome DevTools Protocol (CDP) Integration

## Objective

Finalize the rendering mechanism for the crawler and implement a solution capable of fetching **fully rendered HTML** from modern JavaScript-based websites.

---

## Design Decision

The session began by re-evaluating all the approaches that had been researched previously, including **Backend API Extraction**, **Puppeteer**, **Chromium Embedded Framework (CEF)**, and **Chrome DevTools Protocol (CDP)**.

After a technical discussion and doubt-clearing session with **Umair Sir**, it was concluded that **CDP** can be used for this project. It provides direct access to Chromium's rendering engine without introducing additional runtime dependencies like Node.js or embedding an entire browser framework.

Based on this discussion, I decided to proceed with the CDP-based implementation.

---

## Research on Chrome DevTools Protocol

Before starting the implementation, I spent time understanding how CDP actually works internally.

The research included:

* How Chromium exposes a **Remote Debugging** interface.
* How a client discovers browser targets using the DevTools HTTP endpoint.
* How communication takes place over a **WebSocket** connection.
* How JSON-RPC messages are exchanged between the client and Chromium.
* The sequence of commands required to open a webpage, wait for JavaScript execution, and retrieve the fully rendered DOM.

This research provided a clear understanding of how Chromium can be controlled programmatically without relying on browser automation frameworks.

---

## Implementation

After understanding the protocol, I implemented the complete CDP workflow in C++.

The implementation involved:

* Launching Chromium in **headless mode** with the Remote Debugging Port enabled.
* Discovering the active browser target.
* Establishing a WebSocket connection with Chromium.
* Sending CDP commands to navigate to a webpage.
* Waiting for the page to finish loading and JavaScript execution.
* Retrieving the final rendered HTML for further processing.

The implementation successfully demonstrated that rendered HTML could be obtained directly from Chromium while keeping the project completely native in C++.

---

## Additional Learning

While setting up the implementation, I also learned how the **Ninja Build System** works.

Until now, I had only used CMake to generate build files. During this implementation, I understood that **CMake is only a build-system generator**, whereas **Ninja** is the actual build tool responsible for compiling source files.

I also explored how Ninja:

* Tracks file dependencies.
* Rebuilds only the files that have changed instead of recompiling the entire project.
* Works with CMake-generated `build.ninja` files to provide much faster incremental builds.

Understanding this build workflow helped me debug build-related issues more effectively.

---

## Problems Encountered

### 1. Linker Errors

While integrating the required networking libraries, the project initially failed during the linking stage.

The linker was unable to resolve symbols related to external libraries because the required libraries were either missing from the link command or linked in an incorrect order.

This issue was resolved by updating the **CMake configuration**, linking all required libraries correctly, and regenerating the build files before recompiling the project.

---

### 2. DLL Configuration Issues

After the project compiled successfully, the executable failed to launch because several runtime DLLs were not available.

The missing runtime libraries included components from the **MSYS2 UCRT64** toolchain, such as the GCC runtime and C++ standard library DLLs.

To resolve this issue:

* The missing DLLs were identified using the Windows `where` command.
* The required runtime libraries were copied alongside the executable during testing.
* The compiler environment and PATH configuration were verified to ensure the application could locate all runtime dependencies correctly.

After fixing the runtime dependencies, the executable launched successfully.

---

### 3. Dependency Configuration

Additional time was spent configuring third-party libraries required for CDP communication.

This involved:

* Correcting include directory paths.
* Fixing library search paths.
* Updating CMake targets.
* Rebuilding the project after each configuration change until all dependencies were resolved successfully.

---

## Outcome

By the end of the session, the CDP-based implementation was successfully integrated into the project. Alongside implementing the communication workflow with Chromium, I gained a deeper understanding of the CDP architecture, the CMake–Ninja build pipeline, linker behavior, and runtime dependency management. The crawler was now capable of retrieving fully rendered HTML without relying on Node.js or heavyweight browser frameworks.
