ARCHITECTURE
============

This document describes the internal architecture and design of LiteReader,
a SQLite database file parser.


SYSTEM OVERVIEW
---------------

LiteReader is structured as a pipeline that transforms raw SQLite database
bytes into structured, human-readable output:

    +------------+     +----------+     +--------+     +--------+
    | File Input | --> | Parser   | --> | Schema | --> | Output |
    | (mmap)     |     | (binary) |     | (text) |     | (print)|
    +------------+     +----------+     +--------+     +--------+


COMPONENT DIAGRAM
-----------------

    +------------------------------------------------------------------+
    |                           main.c                                 |
    |  Entry point, argument parsing, output formatting                |
    +------------------------------------------------------------------+
           |                    |                    |
           v                    v                    v
    +-------------+      +-------------+      +-------------+
    |  parser.c   |      |  schema.c   |      |   cell.c    |
    |             |      |             |      |             |
    | - mmap file |      | - parse     |      | - decode    |
    | - read hdr  |      |   master    |      |   records   |
    | - parse     |      |   table     |      | - serial    |
    |   pages     |      | - extract   |      |   types     |
    +-------------+      |   entries   |      | - varints   |
           |             +-------------+      +-------------+
           |                    |                    |
           v                    v                    v
    +------------------------------------------------------------------+
    |                          utils.c                                 |
    |  Big-endian readers, varint decoder                              |
    +------------------------------------------------------------------+
           |
           v
    +------------------------------------------------------------------+
    |                        constants.h                               |
    |  Offset definitions, magic numbers, page type constants          |
    +------------------------------------------------------------------+
           |
           v
    +------------------------------------------------------------------+
    |                          types.h                                 |
    |  Data structure definitions (db_header_t, btree_page_header_t)   |
    +------------------------------------------------------------------+


MODULE DESCRIPTIONS
-------------------

main.c
    Entry point for the application. Handles command-line argument
    parsing, orchestrates the parsing pipeline, and formats output
    for display. Contains print_db_header() and print_page_header()
    functions for human-readable output.

parser.c
    Core database file parser. Opens files using mmap() for memory-
    efficient access. Parses the 100-byte database header and all
    B-tree page headers. Manages memory allocation for page structures.

    Key functions:
    - parse_database()   Opens and parses entire database file
    - free_database()    Releases all allocated memory

schema.c
    Extracts schema information from the sqlite_master table on page 1.
    Parses table, index, view, and trigger definitions.

    Key functions:
    - parse_schema()     Extracts schema entries from page 1
    - print_schema()     Displays schema in readable format
    - free_schema()      Releases schema memory

cell.c
    Decodes individual cell/record data from leaf table pages. Handles
    SQLite's record format including payload size, rowid, header size,
    serial type array, and data values.

    Key functions:
    - parse_cell()       Decodes a single cell and prints values

utils.c
    Low-level utility functions for reading big-endian integers and
    SQLite varints from raw byte streams.

    Key functions:
    - read_be16()        Read 16-bit big-endian integer
    - read_be32()        Read 32-bit big-endian integer
    - read_varint()      Read SQLite variable-length integer


DATA STRUCTURES
---------------

db_header_t
    Represents the 100-byte SQLite database header. Contains all
    configuration values including page size, encoding, schema format,
    and version information.

    struct db_header_t {
        uint8_t  magic[16];              // "SQLite format 3\0"
        uint16_t page_size;              // Database page size
        uint8_t  file_format_write;      // File format write version
        uint8_t  file_format_read;       // File format read version
        uint8_t  reserved_space;         // Reserved bytes per page
        uint8_t  max_embed_payload_frac; // Maximum embedded payload
        uint8_t  min_embed_payload_frac; // Minimum embedded payload
        uint8_t  leaf_payload_frac;      // Leaf payload fraction
        uint32_t file_change_counter;    // File change counter
        uint32_t header_db_size;         // Database size in pages
        ...
    };

btree_page_header_t
    Represents a B-tree page header (8 or 12 bytes depending on
    page type). Interior pages have an additional 4-byte rightmost
    pointer.

    struct btree_page_header_t {
        uint8_t   page_type;             // Page type (0x02/05/0a/0d)
        uint16_t  first_freeblock;       // First freeblock offset
        uint16_t  cell_count;            // Number of cells
        uint16_t  cell_content_start;    // Cell content area start
        uint8_t   fragmented_free_bytes; // Fragmented free bytes
        uint32_t  rightmost_pointer;     // Interior pages only
        uint16_t* cell_pointers;         // Array of cell offsets
    };

database_t
    Top-level structure containing complete parsed database state.

    struct database_t {
        db_header_t         header;       // Database header
        btree_page_header_t *page_headers; // All page headers
        void                *file_data;   // mmap'd file data
        size_t              file_size;    // Total file size
    };

schema_entry_t
    Single entry from sqlite_master table.

    struct schema_entry_t {
        char     *type;      // "table", "index", "view", "trigger"
        char     *name;      // Object name
        char     *tbl_name;  // Associated table name
        uint64_t rootpage;   // Root page number
        char     *sql;       // CREATE statement
    };

schema_t
    Collection of all schema entries.

    struct schema_t {
        schema_entry_t *entries;   // Array of entries
        size_t         count;      // Number of entries
        size_t         capacity;   // Allocated capacity
    };


MEMORY MANAGEMENT
-----------------

LiteReader uses a hierarchical memory management strategy:

1. File Data
   The database file is memory-mapped using mmap(). This provides
   efficient random access without loading the entire file. The
   mapping is released in free_database().

2. Page Headers
   A single allocation holds all btree_page_header_t structures.
   Each page header may have its own cell_pointers array.

3. Schema Entries
   Schema entries are allocated dynamically as they are parsed.
   String fields (type, name, tbl_name, sql) are individually
   allocated and must be freed.

Deallocation Order:
    1. Individual cell_pointers arrays
    2. page_headers array
    3. File mmap (munmap)
    4. database_t structure
    5. Schema entry strings
    6. Schema entries array
    7. schema_t structure


PARSING FLOW
------------

1. File Opening
   open() -> fstat() -> mmap() -> close()
   
   The file descriptor is closed after mmap() since the mapping
   remains valid.

2. Database Header Parsing (Offset 0x00-0x64)
   Sequential reads of header fields using read_be16/read_be32
   for multi-byte values. SQLite uses big-endian byte order.

3. Page Header Parsing
   For each page in the database:
   
   a. Calculate page offset:
      - Page 1: offset 0x64 (after database header)
      - Page N: offset = (N-1) * page_size

   b. Read 8-byte base header

   c. For interior pages (0x02, 0x05):
      - Read additional 4-byte rightmost pointer

   d. Read cell pointer array (2 bytes * cell_count)

4. Schema Extraction
   Parse cells from page 1 (sqlite_master table):
   
   a. For each cell pointer:
      - Seek to cell offset
      - Read payload size (varint)
      - Read rowid (varint)
      - Read record header size (varint)
      - Read serial type array
      - Decode column values

   b. Extract: type, name, tbl_name, rootpage, sql

5. Cell Decoding
   For leaf table pages:
   
   a. Read cell header:
      - Payload size (varint)
      - Rowid (varint)
      - Record header size (varint)

   b. Parse serial types from record header

   c. Decode values based on serial types:
      - NULL (type 0)
      - Integers (types 1-6, 8, 9)
      - Float (type 7)
      - BLOB (type N>=12, even)
      - TEXT (type N>=13, odd)


ERROR HANDLING
--------------

Errors are handled through return values and cleanup:

- parse_database() returns NULL on failure
- Memory is freed on error paths to prevent leaks
- Error messages are printed to stderr with perror()

Error conditions:
- File open failure
- Memory allocation failure
- Invalid page offset (bounds check)
- Malformed varint (truncated data)


THREAD SAFETY
-------------

LiteReader is NOT thread-safe. The implementation assumes:
- Single-threaded execution
- Exclusive file access
- No concurrent modifications

For thread-safe operation, external synchronization would be needed.


FUTURE CONSIDERATIONS
---------------------

Potential architectural improvements:

1. Overflow Page Support
   Add overflow page chain traversal for large records that span
   multiple pages.

2. Index Page Parsing
   Implement index cell format parsing (different from table cells).

3. Streaming Interface
   Add callback-based API for processing records without storing
   all data in memory.

4. Write Support
   Add database modification capabilities (would require significant
   architectural changes).

5. WAL Support
   Implement Write-Ahead Log processing for databases in WAL mode.


NOTE ON DOCUMENTATION
---------------------

This documentation was generated programmatically and may contain
inaccuracies or inconsistencies with the actual implementation. Users
are encouraged to verify critical details against the source code.
Corrections and improvements are welcome.
