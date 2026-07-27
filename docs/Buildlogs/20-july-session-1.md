# Build Log – 20 July 2026 (Session 1)

## Duration 9:30 am to 1:00 pm
## Goal
Implement the Indexer module for the crawler and build an inverted index using custom data structures.

## Work Completed

- Implemented the `Indexer` class.
- Retrieved all crawled pages from the database.
- Tokenized the page content into individual words.
- Normalized every word before indexing.
- Counted the frequency of each word in a page.
- Used a custom `HashMap<string, DynamicArray<Url>>` to maintain the inverted index.
- Stored the top ranked URLs for every word based on frequency.
- Implemented logic to keep only the highest-frequency URLs for each word.
- Implemented `storeIndex()` to prepare the final index for storage.

## Internal Design

The inverted index was maintained in memory using:

```cpp
HashMap<string, DynamicArray<Url>>
```

Where:

- **Key:** Word
- **Value:** DynamicArray containing
  - URL
  - Frequency of occurrence

Whenever a word was encountered:

- If it already existed in the HashMap:
  - Update its frequency for the current page.
  - Maintain descending order based on frequency.
  - Keep only the top entries.
- Otherwise:
  - Create a new entry.

## Problems Faced

### Incorrect ranking index

Used:

```cpp
rank[i]
```

instead of

```cpp
rank[j]
```

while iterating through URL rankings.

---

### Incorrect comma separated URL generation

Initially every URL started with an extra comma.

Fixed by inserting commas only after the first URL.

---

### HashMap insertion logic

Needed additional checks while maintaining sorted rankings.

Adjusted insertion logic so ranking order remained correct.

---

### Compiler Errors

Encountered multiple compilation errors including:

- `string was not declared in this scope`
- Incorrect template arguments
- Namespace related issues (`std::string`)
- Member function lookup errors caused by previous template failures

Resolved after correcting template declarations and namespaces.

## Outcome

Successfully completed the in-memory inverted index implementation using custom data structures.

The index was now capable of storing:

- Words
- Frequencies
- Top ranked URLs

completely in memory.