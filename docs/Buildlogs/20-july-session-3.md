# Build Log – 20 July 2026 (Session 3)

## Duration 4:00 pm to 6:30 pm



## Goal

Improve indexing quality by normalizing words before storing them.

## Work Completed

Implemented a normalization pipeline before indexing.

### Lowercase Conversion

Converted all words to lowercase.

Example:

```
Hello
HELLO
hello
```

↓

```
hello
```

---

### Removed Non-Alphanumeric Characters

Ignored punctuation and special symbols.

Example:

```
hello!!!
book,
car.
```

↓

```
hello
book
car
```

---

### Stop Word Removal

Implemented a custom stop-word set.

Ignored common English grammar words such as:

- the
- is
- are
- am
- can
- could
- will
- would
- have
- has
- had
- of
- to
- in
- on
- at
- and
- or
- but
- if
- with
- from

and many other frequently occurring words.

These words are not useful for searching and therefore are not indexed.

---

### Empty Word Removal

Ignored empty strings generated after normalization.

---

### Porter-style Stemming

Implemented a lightweight Porter-style stemmer.

Examples:

```
cars
```

↓

```
car
```

```
books
```

↓

```
book
```

```
booked
```

↓

```
book
```

```
playing
```

↓

```
play
```

```
running
```

↓

```
run
```

```
racing
```

↓

```
race
```

```
smelling
```

↓

```
smell
```

```
studies
```

↓

```
study
```

The stemmer removes common suffixes such as:

- s
- es
- ed
- ing
- ies

and performs additional corrections for common English word forms.

## Benefits

Normalization significantly reduces duplicate words in the index.

Example:

```
Play
playing
played
plays
```

All contribute toward a common indexed form, improving search accuracy and reducing storage.

## Outcome

Completed the complete normalization pipeline.
