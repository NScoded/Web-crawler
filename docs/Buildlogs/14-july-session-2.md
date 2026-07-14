# Build Log

**Date:** 14 July 2026  
**Duration:** 2:00 PM to 5:00 PM

---

## Goal
- Identify and resolve the `std::bad_alloc` issue occurring during web crawler execution.
- Stabilize the custom STL `LinkedList` implementation used throughout the web crawler.
- Ensure the crawler executes continuously without crashes, memory corruption, or unexpected runtime failures.
- Validate the complete crawler pipeline after fixing the underlying data structure bug.

---

## Problem
While crawling multiple web pages, the application consistently terminated with a `std::bad_alloc` exception. Initially, the issue appeared to be related to memory exhaustion, but repeated testing showed that the available system memory was sufficient.

The crash occurred after processing several pages, making it difficult to identify the exact source of the failure. Since all crawler modules appeared to work correctly in isolation, the issue was suspected to be caused by one of the custom STL data structures used internally.

---

## Investigation & Debugging

To isolate the problem, the complete crawler execution flow was reviewed step by step.

The following components were verified individually:

- Chrome DevTools Protocol (CDP) page rendering
- HTML downloading
- HTML parsing
- Hyperlink extraction
- URL normalization
- Frontier operations
- SQLite page storage
- Overall crawler workflow

Since each module behaved correctly, attention shifted to the custom `LinkedList` implementation that is heavily used by other data structures.

A complete review of the LinkedList implementation was performed, including:

- Constructor
- Destructor
- Copy Constructor
- Copy Assignment Operator
- Append
- Insert
- Remove
- Pop
- Reverse
- Exists
- Clear

---

## Root Cause Identified

A critical bug was discovered inside the `LinkedList::remove()` method while deleting the first node of the list.

Previous implementation:

```cpp
Node* temp = head;
head = head->next;
delete head;
```

Instead of deleting the old head node (`temp`), the implementation deleted the new head.

This resulted in:

- Dangling pointers
- Heap corruption
- Invalid memory access
- Random runtime crashes
- `std::bad_alloc` during future memory allocations

Although the application reported a memory allocation failure, the actual cause was heap corruption introduced by incorrect pointer deletion.

---

## Fix Implemented

The removal logic was corrected.

Updated implementation:

```cpp
Node* temp = head;
head = head->next;

if(head == nullptr){
    tail = nullptr;
}

delete temp;
size--;
```

Additional validation ensured:

- Correct deletion of the old head node.
- Proper handling of single-node lists.
- Correct maintenance of both `head` and `tail`.
- No dangling pointers remain after deletion.
- Memory is released safely.

---

## Additional Code Review

After fixing the primary issue, the complete `LinkedList` implementation was reviewed for correctness.

Verified functionality:

- Rule of Three implementation
- Deep copy through Copy Constructor
- Copy Assignment Operator
- Destructor
- Append
- Insert at head
- Insert at arbitrary position
- Remove
- Reverse
- Exists
- Clear
- Pop
- Equality operator

Minor code-quality improvements were also identified for future refactoring:

- Remove unreachable `nullptr` check inside `remove()`.
- Pass objects using `const T&` wherever appropriate.
- Mark read-only member functions as `const`.
- Reuse `clear()` inside the destructor to reduce duplicate code.

These improvements were noted but were not required for correctness.

---

## Testing Performed

Performed complete end-to-end testing of the crawler after applying the fix.

Verified:

- Chrome launches successfully using CDP.
- Pages are rendered correctly.
- HTML content is downloaded successfully.
- HTML parser extracts hyperlinks correctly.
- URL normalization behaves as expected.
- Frontier processes URLs continuously.
- SQLite stores crawled pages successfully.
- Multiple pages can be crawled continuously.
- No segmentation faults observed.
- No heap corruption detected during execution.
- No `std::bad_alloc` exceptions after the fix.

---

## Outcome

- Successfully identified the root cause of crawler instability.
- Fixed heap corruption caused by incorrect pointer deletion.
- Eliminated the `std::bad_alloc` issue.
- Stabilized the custom `LinkedList` implementation.
- Successfully validated the complete web crawler pipeline.

The crawler now executes reliably with:

- Stable CDP communication
- Correct HTML parsing
- Continuous URL processing
- Reliable SQLite storage
- No memory corruption
- No runtime crashes
- No `std::bad_alloc` exceptions during testing

---

## Status

**Completed Successfully**

The web crawler is now stable, memory-safe, and capable of processing multiple pages continuously without crashes or heap corruption.