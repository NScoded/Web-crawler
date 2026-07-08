# Build Log – 6 July 2026 - Session 3


## Objective

Started the implementation of the **Frontier** module, which will act as the entry point of the crawler and manage the scheduling of URLs.

---

## Queue Implementation

As planned during the design phase, the Frontier is based on a **Queue** data structure.

Instead of implementing it from scratch, I reused the **LinkedList** implementation from my previous project and built a generic Queue on top of it. This allowed me to reuse an already tested data structure and reduced duplicate code.

The following operations were implemented for the Queue:

* `push(T val)` – Insert an element at the rear of the queue.
* `pop()` – Remove and return the front element.
* `front()` – Access the front element without removing it.
* `size()` – Return the current number of elements.
* `empty()` – Check whether the queue is empty.

The Queue is implemented as a template so that it can store URLs as well as other data types if required in future modules.

---

## Problem Encountered

While integrating the Queue with the existing **LinkedList**, I encountered an issue with the `get()` function.

The original implementation accepted the **value** as a parameter for searching an element. During the Queue implementation, this design was not very useful because queue operations require direct access to elements based on their position rather than searching by value.

To resolve this, I modified the LinkedList implementation from my previous project.

### Changes Made

* Updated the `get()` method to accept an **index** instead of a value.
* Refactored the LinkedList implementation to support indexed access.
* Verified that the updated method worked correctly with the Queue implementation.

This change also improves the usability of the LinkedList for future data structures where positional access is required.

---

## Outcome

The Queue module was successfully completed using the existing LinkedList implementation. During the process, the LinkedList API was improved by redesigning the `get()` method to use index-based access, making it more suitable for both the current project and future reuse.

With the Queue implementation complete, the basic foundation of the **Frontier** module is now ready for further development.
