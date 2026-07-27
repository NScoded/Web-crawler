# Build Log – 20 July 2026 (Session 2)

## Duration 2:00 pm to 3:00 pm

## Goal

Integrate the Indexer with the MySQL database.

## Work Completed

- Created the `indexer` table in MySQL.
- Added `putIndexer()` inside `PageStorage`.
- Used MySQL prepared statements for inserting records.
- Connected the Indexer with PageStorage.
- Stored the generated inverted index into the database.
- Implemented `storeIndex()` to insert every indexed word into MySQL.

## Database Schema

Each record stores:

- Word
- Maximum Frequency
- Comma-separated ranked URLs

## Improvements

Previously:

- Entire inverted index existed only in RAM.

Now:

- Every indexed word is permanently stored in MySQL.
- Index survives application restart.
- Search module can directly read from the database.

## Problems Faced

### MySQL Prepared Statement Issues

While implementing prepared statements:

- Binding parameters
- Correct buffer lengths
- Statement execution
- Data type mismatches

required debugging.

---

### URL Formatting

Multiple URLs had to be converted into a comma-separated string before storing.

---

### Ranking Storage

Ensured only the highest ranked URLs were written into the database.

## Outcome

Indexer is now fully integrated with the database.

