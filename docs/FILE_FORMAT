SQLITE FILE FORMAT SPECIFICATION
=================================

This document describes the SQLite database file format as implemented
by LiteReader. It is a subset of the full SQLite file format specification.

For the complete specification, see: https://www.sqlite.org/fileformat.html


FILE STRUCTURE OVERVIEW
-----------------------

An SQLite database file consists of one or more fixed-size pages:

    +------------------+
    |     Page 1       |  Contains database header + first B-tree page
    +------------------+
    |     Page 2       |
    +------------------+
    |       ...        |
    +------------------+
    |     Page N       |
    +------------------+

Page size is a power of 2 between 512 and 65536 bytes (default: 4096).


DATABASE HEADER
---------------

The first 100 bytes of the database file contain the database header:

    Offset  Size  Description
    ------  ----  -----------
    0       16    Magic string: "SQLite format 3\000"
    16      2     Page size in bytes (big-endian)
    18      1     File format write version (1 = legacy, 2 = WAL)
    19      1     File format read version
    20      1     Reserved space at end of each page
    21      1     Maximum embedded payload fraction (must be 64)
    22      1     Minimum embedded payload fraction (must be 32)
    23      1     Leaf payload fraction (must be 32)
    24      4     File change counter
    28      4     Database size in pages
    32      4     First freelist trunk page
    36      4     Total number of freelist pages
    40      4     Schema cookie
    44      4     Schema format number (1, 2, 3, or 4)
    48      4     Default page cache size
    52      4     Page number of largest root B-tree (auto-vacuum)
    56      4     Database text encoding (1=UTF-8, 2=UTF-16le, 3=UTF-16be)
    60      4     User version (set by user_version pragma)
    64      4     Incremental-vacuum mode flag
    68      4     Application ID (set by application_id pragma)
    72      20    Reserved for expansion (must be zero)
    92      4     Version-valid-for number
    96      4     SQLite version number

All multi-byte values are stored in big-endian byte order.


PAGE SIZE
---------

The page size is stored at offset 16 as a 2-byte big-endian integer.
Special case: value 1 means page size is 65536.

Valid page sizes: 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536


TEXT ENCODING
-------------

The text encoding at offset 56 determines how text strings are stored:

    Value   Encoding
    -----   --------
    1       UTF-8
    2       UTF-16le (little-endian)
    3       UTF-16be (big-endian)


B-TREE PAGES
------------

SQLite uses B-trees for table and index storage. There are four page types:

    Value   Type
    -----   ----
    0x02    Interior index B-tree page
    0x05    Interior table B-tree page
    0x0a    Leaf index B-tree page
    0x0d    Leaf table B-tree page


B-TREE PAGE HEADER
------------------

Every B-tree page has an 8-byte header (12 bytes for interior pages):

    Offset  Size  Description
    ------  ----  -----------
    0       1     Page type (0x02, 0x05, 0x0a, or 0x0d)
    1       2     First freeblock offset (0 if none)
    3       2     Number of cells on this page
    5       2     Offset to first byte of cell content area
    7       1     Number of fragmented free bytes
    8       4     Right-most pointer (interior pages only)

For page 1, the B-tree header begins at offset 100 (after database header).
For all other pages, the B-tree header begins at offset 0.


CELL POINTER ARRAY
------------------

Following the page header is an array of 2-byte cell pointers, one for
each cell. Each pointer is the offset from the start of the page to
the beginning of the cell content.

Cell pointers are stored in key order (ascending for tables).


CELL FORMAT - LEAF TABLE
------------------------

Leaf table B-tree cells contain actual row data:

    +-----------------+
    | Payload size    |  varint
    +-----------------+
    | Rowid           |  varint
    +-----------------+
    | Payload         |  record format
    +-----------------+

Payload format (record):

    +-------------------+
    | Header size       |  varint (includes this varint)
    +-------------------+
    | Serial type 1     |  varint
    +-------------------+
    | Serial type 2     |  varint
    +-------------------+
    | ...               |
    +-------------------+
    | Serial type N     |  varint
    +-------------------+
    | Value 1           |  variable size based on serial type
    +-------------------+
    | Value 2           |
    +-------------------+
    | ...               |
    +-------------------+
    | Value N           |
    +-------------------+


CELL FORMAT - INTERIOR TABLE
----------------------------

Interior table B-tree cells contain pointers to child pages:

    +-----------------+
    | Left child page |  4 bytes, big-endian
    +-----------------+
    | Rowid           |  varint (key)
    +-----------------+


CELL FORMAT - LEAF INDEX
------------------------

Leaf index B-tree cells contain index entries:

    +-----------------+
    | Payload size    |  varint
    +-----------------+
    | Payload         |  record format (indexed columns + rowid)
    +-----------------+


CELL FORMAT - INTERIOR INDEX
----------------------------

Interior index B-tree cells:

    +-----------------+
    | Left child page |  4 bytes, big-endian
    +-----------------+
    | Payload size    |  varint
    +-----------------+
    | Payload         |  record format
    +-----------------+


VARINT FORMAT
-------------

SQLite uses a variable-length integer encoding called "varint":

    Bytes   Value Range
    -----   -----------
    1       0 to 127
    2       128 to 16383
    3       16384 to 2097151
    ...     ...
    9       Full 64-bit range

Encoding:
  - High bit of each byte indicates continuation (1 = more bytes follow)
  - Lower 7 bits of each byte contribute to value
  - 9th byte (if present) uses all 8 bits

Decoding algorithm:

    uint64_t result = 0;
    for (int i = 0; i < 9; i++) {
        if (i == 8) {
            result = (result << 8) | data[i];
            return result;
        }
        result = (result << 7) | (data[i] & 0x7F);
        if ((data[i] & 0x80) == 0) {
            return result;
        }
    }


SERIAL TYPES
------------

Serial types define the type and size of each column value:

    Serial Type   Content Size   Meaning
    -----------   ------------   -------
    0             0              NULL
    1             1              8-bit signed integer
    2             2              16-bit signed big-endian integer
    3             3              24-bit signed big-endian integer
    4             4              32-bit signed big-endian integer
    5             6              48-bit signed big-endian integer
    6             8              64-bit signed big-endian integer
    7             8              IEEE 754 64-bit float (big-endian)
    8             0              Integer constant 0
    9             0              Integer constant 1
    10            *              Reserved for internal use
    11            *              Reserved for internal use
    N >= 12       (N-12)/2       BLOB (N even)
    N >= 13       (N-13)/2       Text string (N odd)

Integer encoding:
  - Signed integers use two's complement
  - Stored in big-endian byte order
  - Leading zeros are removed for smaller serial types


SQLITE_MASTER TABLE
-------------------

The sqlite_master table is always stored in the B-tree rooted at page 1.
It contains the database schema with these columns:

    Column    Type      Description
    ------    ----      -----------
    type      TEXT      "table", "index", "view", or "trigger"
    name      TEXT      Name of the object
    tbl_name  TEXT      Associated table name
    rootpage  INTEGER   Root page of the object's B-tree
    sql       TEXT      CREATE statement text

For indexes, triggers, and views, tbl_name contains the associated
table name. For tables, tbl_name equals name.


OVERFLOW PAGES
--------------

When a cell payload exceeds the maximum allowed size, excess data is
stored in overflow pages. The cell contains the beginning of the
payload followed by a 4-byte page number pointing to the first
overflow page.

Each overflow page contains:
  - 4 bytes: next overflow page number (0 if last)
  - Remaining bytes: overflow content

NOTE: LiteReader does not currently support overflow pages.


FREELIST
--------

Deleted pages are added to a freelist for reuse. The freelist is a
linked list of trunk pages, each containing an array of leaf page
numbers.

Trunk page format:
  - 4 bytes: next trunk page number (0 if last)
  - 4 bytes: number of leaf pointers on this page
  - N * 4 bytes: array of leaf page numbers


EXAMPLE: PARSING A SIMPLE DATABASE
----------------------------------

Given a database with one table and one row:

    CREATE TABLE t1(a INTEGER, b TEXT);
    INSERT INTO t1 VALUES(42, 'hello');

Page 1 layout:

    Offset 0-99:     Database header
    Offset 100:      B-tree page header (sqlite_master)
      - 0x0d         Page type (leaf table)
      - 0x0000       First freeblock
      - 0x0001       Cell count (1 entry in sqlite_master)
      - ...
    Offset 108:      Cell pointer array
      - 2 bytes      Pointer to CREATE TABLE cell
    ...
    Cell content:    sqlite_master row
      - varint       Payload size
      - varint       Rowid (1)
      - varint       Header size
      - varint       Serial type for "type" column
      - varint       Serial type for "name" column
      - varint       Serial type for "tbl_name" column
      - varint       Serial type for "rootpage" column
      - varint       Serial type for "sql" column
      - data         "table"
      - data         "t1"
      - data         "t1"
      - data         2 (root page of t1)
      - data         "CREATE TABLE t1(a INTEGER, b TEXT)"

Page 2 layout (t1 table):

    Offset 0:        B-tree page header
      - 0x0d         Page type (leaf table)
      - 0x0000       First freeblock
      - 0x0001       Cell count (1 row)
      - ...
    Cell content:
      - varint       Payload size
      - varint       Rowid (1)
      - varint       Header size (3)
      - varint       Serial type 1 (8-bit int)
      - varint       Serial type 17 ((17-13)/2 = 2 byte text)
      - 1 byte       42 (integer value)
      - 5 bytes      "hello" (text value)


REFERENCES
----------

SQLite File Format:  https://www.sqlite.org/fileformat.html
SQLite B-tree:       https://www.sqlite.org/btreemodule.html
Record Format:       https://www.sqlite.org/fileformat2.html#record_format


NOTE ON DOCUMENTATION
---------------------

This documentation was generated programmatically and may contain
inaccuracies. For authoritative information on the SQLite file format,
always refer to the official SQLite documentation linked above.
Corrections are welcome.
