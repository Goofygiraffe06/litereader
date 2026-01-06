API REFERENCE
=============

This document provides detailed documentation for all public functions
and data structures in LiteReader.


CONTENTS
--------

    1. Data Types
    2. Parser Functions (parser.h)
    3. Schema Functions (schema.h)
    4. Cell Functions (cell.h)
    5. Utility Functions (utils.h)
    6. Constants (constants.h)


1. DATA TYPES
=============

Defined in: include/types.h


db_header_t
-----------

SQLite database header structure (100 bytes).

    typedef struct {
        uint8_t  magic[16];
        uint16_t page_size;
        uint8_t  file_format_write;
        uint8_t  file_format_read;
        uint8_t  reserved_space;
        uint8_t  max_embed_payload_frac;
        uint8_t  min_embed_payload_frac;
        uint8_t  leaf_payload_frac;
        uint32_t file_change_counter;
        uint32_t header_db_size;
        uint32_t first_freelist_trunk;
        uint32_t total_freelist_trunk;
        uint32_t schema_cookie;
        uint32_t schema_format_number;
        uint32_t default_page_cache_size;
        uint32_t page_number_largest_root;
        uint32_t db_text_encoding;
        uint32_t user_version;
        uint32_t incremental_version_mode;
        uint32_t application_id;
        uint8_t  reserved_expansion[20];
        uint32_t version_valid_for;
        uint32_t sqlite_version_number;
    } db_header_t;

Fields:
    magic                   - Must be "SQLite format 3\0"
    page_size               - Database page size (512-65536)
    file_format_write       - Write version (1=legacy, 2=WAL)
    file_format_read        - Read version
    reserved_space          - Reserved bytes at end of each page
    max_embed_payload_frac  - Maximum embedded payload (always 64)
    min_embed_payload_frac  - Minimum embedded payload (always 32)
    leaf_payload_frac       - Leaf payload fraction (always 32)
    file_change_counter     - Incremented on each transaction
    header_db_size          - Size of database in pages
    first_freelist_trunk    - First freelist trunk page (0 if none)
    total_freelist_trunk    - Total freelist pages
    schema_cookie           - Schema version number
    schema_format_number    - Schema format (1-4)
    default_page_cache_size - Suggested cache size
    page_number_largest_root - For auto-vacuum databases
    db_text_encoding        - Text encoding (1=UTF-8, 2=UTF-16le, 3=UTF-16be)
    user_version            - User-defined version number
    incremental_version_mode - Incremental vacuum mode flag
    application_id          - Application identifier
    reserved_expansion      - Reserved (must be zero)
    version_valid_for       - SQLite version that wrote the file
    sqlite_version_number   - SQLite version number


btree_page_header_t
-------------------

B-tree page header structure.

    typedef struct {
        uint8_t   page_type;
        uint16_t  first_freeblock;
        uint16_t  cell_count;
        uint16_t  cell_content_start;
        uint8_t   fragmented_free_bytes;
        uint32_t  rightmost_pointer;
        uint16_t *cell_pointers;
    } btree_page_header_t;

Fields:
    page_type             - Page type (0x02, 0x05, 0x0a, 0x0d)
    first_freeblock       - Offset to first freeblock (0 if none)
    cell_count            - Number of cells on this page
    cell_content_start    - Offset to start of cell content area
    fragmented_free_bytes - Total fragmented free bytes
    rightmost_pointer     - Right child pointer (interior pages only)
    cell_pointers         - Array of cell offsets (dynamically allocated)


database_t
----------

Complete parsed database structure.

    typedef struct {
        db_header_t          header;
        btree_page_header_t *page_headers;
        void                *file_data;
        size_t               file_size;
    } database_t;

Fields:
    header       - Parsed database header
    page_headers - Array of page headers (one per page)
    file_data    - Pointer to mmap'd file data
    file_size    - Total file size in bytes


schema_entry_t
--------------

Single sqlite_master table entry.

    typedef struct {
        char     *type;
        char     *name;
        char     *tbl_name;
        uint64_t  rootpage;
        char     *sql;
    } schema_entry_t;

Fields:
    type     - Object type: "table", "index", "view", "trigger"
    name     - Object name
    tbl_name - Associated table name
    rootpage - Root page number of object's B-tree
    sql      - Original CREATE statement


schema_t
--------

Collection of schema entries.

    typedef struct {
        schema_entry_t *entries;
        size_t          count;
        size_t          capacity;
    } schema_t;

Fields:
    entries  - Array of schema entries
    count    - Number of valid entries
    capacity - Allocated array size


2. PARSER FUNCTIONS
===================

Defined in: include/parser.h
Implemented in: src/parser.c


parse_database
--------------

    database_t* parse_database(const char *filename);

Opens and parses an SQLite database file.

Parameters:
    filename - Path to SQLite database file

Returns:
    Pointer to database_t structure on success, NULL on failure.

Description:
    Opens the specified file using mmap() for memory-efficient access.
    Parses the 100-byte database header and all B-tree page headers.
    Allocates memory for page_headers array and cell_pointers arrays.

Error conditions:
    - File does not exist or cannot be opened
    - File is not a valid SQLite database (bad magic)
    - Memory allocation failure
    - Page offset out of bounds

Example:
    database_t *db = parse_database("test.db");
    if (db == NULL) {
        fprintf(stderr, "Failed to parse database\n");
        return 1;
    }
    // Use db...
    free_database(db);


free_database
-------------

    void free_database(database_t *db);

Releases all memory associated with a database structure.

Parameters:
    db - Pointer to database_t structure (may be NULL)

Returns:
    None.

Description:
    Frees all dynamically allocated memory including:
    - Individual cell_pointers arrays for each page
    - page_headers array
    - Unmaps file data (munmap)
    - database_t structure itself

Example:
    database_t *db = parse_database("test.db");
    // Use db...
    free_database(db);
    db = NULL;  // Avoid dangling pointer


3. SCHEMA FUNCTIONS
===================

Defined in: include/schema.h
Implemented in: src/schema.c


parse_schema
------------

    schema_t* parse_schema(database_t *db);

Extracts schema information from sqlite_master table.

Parameters:
    db - Pointer to parsed database structure

Returns:
    Pointer to schema_t structure on success, NULL on failure.

Description:
    Reads the sqlite_master table from page 1 of the database.
    Parses each cell to extract type, name, tbl_name, rootpage, and sql.
    Allocates memory for schema entries and string fields.

Requirements:
    - db must be non-NULL
    - db->header.header_db_size must be > 0
    - Page 1 must be a leaf table page (0x0d)

Example:
    database_t *db = parse_database("test.db");
    schema_t *schema = parse_schema(db);
    if (schema) {
        print_schema(schema);
        free_schema(schema);
    }
    free_database(db);


print_schema
------------

    void print_schema(schema_t *schema);

Prints schema information to stdout.

Parameters:
    schema - Pointer to schema structure (may be NULL)

Returns:
    None.

Description:
    Prints formatted output for each schema entry including:
    - Entry number and type
    - Object name
    - Associated table name
    - Root page number
    - CREATE statement

Output format:
    === Database Schema ===
    
    [1] table: users
        table: users
        rootpage: 2
        sql: CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT)


free_schema
-----------

    void free_schema(schema_t *schema);

Releases all memory associated with a schema structure.

Parameters:
    schema - Pointer to schema_t structure (may be NULL)

Returns:
    None.

Description:
    Frees all dynamically allocated memory including:
    - String fields for each entry (type, name, tbl_name, sql)
    - entries array
    - schema_t structure itself


4. CELL FUNCTIONS
=================

Defined in: include/cell.h
Implemented in: src/cell.c


parse_cell
----------

    int parse_cell(uint8_t *page_data, uint16_t cell_offset, size_t page_size);

Parses and prints a single cell from a leaf table page.

Parameters:
    page_data   - Pointer to start of page data
    cell_offset - Offset from page start to cell
    page_size   - Total page size in bytes

Returns:
    0 on success, -1 on error.

Description:
    Decodes a leaf table cell including:
    - Payload size (varint)
    - Rowid (varint)
    - Record header (varints for serial types)
    - Column values (based on serial types)
    
    Prints decoded values to stdout in format:
    rowid: N | value1, value2, ...

Supported value types:
    - NULL: printed as "NULL"
    - Integers: printed as decimal
    - Text: printed as quoted string
    - BLOB: printed as "BLOB(N bytes)"

Limitations:
    - Does not handle overflow pages
    - Index cells not supported

Example:
    for (uint16_t i = 0; i < page->cell_count; i++) {
        parse_cell(page_data, page->cell_pointers[i], page_size);
    }


5. UTILITY FUNCTIONS
====================

Defined in: include/utils.h
Implemented in: src/utils.c


read_be16
---------

    uint16_t read_be16(uint8_t *ptr);

Reads a 16-bit big-endian integer.

Parameters:
    ptr - Pointer to 2 bytes of data

Returns:
    16-bit unsigned integer value.

Description:
    Converts 2 bytes from big-endian to native byte order.

Example:
    uint8_t data[] = {0x10, 0x00};  // 4096 in big-endian
    uint16_t page_size = read_be16(data);  // Returns 4096


read_be32
---------

    uint32_t read_be32(uint8_t *ptr);

Reads a 32-bit big-endian integer.

Parameters:
    ptr - Pointer to 4 bytes of data

Returns:
    32-bit unsigned integer value.

Description:
    Converts 4 bytes from big-endian to native byte order.

Example:
    uint8_t data[] = {0x00, 0x00, 0x00, 0x03};  // 3 in big-endian
    uint32_t count = read_be32(data);  // Returns 3


read_varint
-----------

    uint64_t read_varint(uint8_t *data, size_t *bytes_read, size_t max_len);

Reads a SQLite variable-length integer.

Parameters:
    data       - Pointer to varint data
    bytes_read - Output: number of bytes consumed
    max_len    - Maximum bytes available to read

Returns:
    64-bit unsigned integer value.

Description:
    Decodes SQLite's varint format:
    - High bit of each byte indicates continuation
    - Lower 7 bits contribute to value
    - Maximum 9 bytes

Error handling:
    If max_len is insufficient, bytes_read is set to 0.

Example:
    size_t consumed;
    uint64_t value = read_varint(ptr, &consumed, remaining);
    if (consumed == 0) {
        // Error: truncated varint
    }
    ptr += consumed;


6. CONSTANTS
============

Defined in: include/constants.h


Database Header Offsets
-----------------------

    OFFSET_MAGIC                    0x00   Magic string
    OFFSET_PAGE_SIZE                0x10   Page size
    OFFSET_FILE_FORMAT_WRITE_VERSION 0x12  Write version
    OFFSET_FILE_FORMAT_READ         0x13   Read version
    OFFSET_RESERVED_SPACE           0x14   Reserved space
    OFFSET_MAX_EMBED_PAYLOAD_FRAC   0x15   Max payload fraction
    OFFSET_MIN_EMBED_PAYLOAD_FRAC   0x16   Min payload fraction
    OFFSET_LEAF_PAYLOAD_FRAC        0x17   Leaf payload fraction
    OFFSET_FILE_CHANGE_COUNTER      0x18   Change counter
    OFFSET_HEADER_DB_SIZE           0x1C   Database size
    OFFSET_FIRST_FREELIST_TRUNK     0x20   First freelist trunk
    OFFSET_TOTAL_FREELIST_PAGES     0x24   Total freelist pages
    OFFSET_SCHEMA_COOKIE            0x28   Schema cookie
    OFFSET_SCHEMA_FORMAT_NUMBER     0x2C   Schema format
    OFFSET_DEFAULT_PAGE_CACHE_SIZE  0x30   Default cache size
    OFFSET_PAGE_NUMBER_LARGEST_ROOT 0x34   Largest root page
    OFFSET_DB_TEXT_ENCODING         0x38   Text encoding
    OFFSET_USER_VERSION             0x3C   User version
    OFFSET_INCREMENTAL_VACCUM_MODE  0x40   Incremental vacuum
    OFFSET_APPLICATION_ID           0x44   Application ID
    OFFSET_RESERVED_EXPANSION       0x48   Reserved (20 bytes)
    OFFSET_VERSION_VALID_FOR        0x5C   Version valid for
    OFFSET_SQLITE_VERSION_NUMBER    0x60   SQLite version


B-Tree Page Header Offsets
--------------------------

    OFFSET_BTREE_PAGE_TYPE          0x00   Page type byte
    OFFSET_BTREE_FIRST_FREEBLOCK    0x01   First freeblock
    OFFSET_BTREE_CELL_COUNT         0x03   Cell count
    OFFSET_BTREE_CELL_CONTENT_START 0x05   Content start
    OFFSET_BTREE_FRAG_FREE_BYTES    0x07   Fragmented bytes
    OFFSET_BTREE_RIGHTMOST_POINTER  0x08   Right pointer


Magic String
------------

    SQLITE_MAGIC    "SQLite format 3\0"


Page Types
----------

    PAGE_TYPE_INTERIOR_INDEX    0x02   Interior index B-tree
    PAGE_TYPE_INTERIOR_TABLE    0x05   Interior table B-tree
    PAGE_TYPE_LEAF_INDEX        0x0A   Leaf index B-tree
    PAGE_TYPE_LEAF_TABLE        0x0D   Leaf table B-tree


Serial Types
------------

    SERIAL_TYPE_NULL       0    NULL value
    SERIAL_TYPE_INT8       1    8-bit signed integer
    SERIAL_TYPE_INT16      2    16-bit signed integer
    SERIAL_TYPE_INT24      3    24-bit signed integer
    SERIAL_TYPE_INT32      4    32-bit signed integer
    SERIAL_TYPE_INT48      5    48-bit signed integer
    SERIAL_TYPE_INT64      6    64-bit signed integer
    SERIAL_TYPE_FLOAT64    7    64-bit IEEE 754 float
    SERIAL_TYPE_ZERO       8    Integer value 0
    SERIAL_TYPE_ONE        9    Integer value 1
    SERIAL_TYPE_INTERNAL1  10   Reserved
    SERIAL_TYPE_INTERNAL2  11   Reserved

    For N >= 12 and even: BLOB of (N-12)/2 bytes
    For N >= 13 and odd:  TEXT of (N-13)/2 bytes


Serial Content Sizes
--------------------

    SERIAL_SIZE_NULL       0
    SERIAL_SIZE_INT8       1
    SERIAL_SIZE_INT16      2
    SERIAL_SIZE_INT24      3
    SERIAL_SIZE_INT32      4
    SERIAL_SIZE_INT48      6
    SERIAL_SIZE_INT64      8
    SERIAL_SIZE_FLOAT64    8
    SERIAL_SIZE_ZERO       0
    SERIAL_SIZE_ONE        0


NOTE ON DOCUMENTATION
---------------------

This documentation was generated programmatically and may contain
inaccuracies or inconsistencies with the actual implementation. Users
are encouraged to verify function signatures and behavior against the
source code. Corrections and improvements are welcome.
