CONTRIBUTING
============

Thank you for your interest in contributing to LiteReader. This document
provides guidelines for submitting patches, reporting bugs, and participating
in development.


GETTING STARTED
---------------

1. Fork the repository on GitHub
2. Clone your fork locally
3. Create a feature branch for your work
4. Make your changes
5. Test thoroughly
6. Submit a pull request


CODE STYLE
----------

LiteReader follows a consistent coding style. Please adhere to these
conventions:

Indentation:
  - Use 4 spaces for indentation (no tabs)
  - Align continuation lines appropriately

Braces:
  - Opening brace on same line as statement
  - Closing brace on its own line

    if (condition) {
        statement;
    } else {
        other_statement;
    }

Naming:
  - Functions: lowercase_with_underscores
  - Types: lowercase_with_t_suffix (e.g., db_header_t)
  - Constants: UPPERCASE_WITH_UNDERSCORES
  - Variables: lowercase_with_underscores

Comments:
  - Use // for single-line comments
  - Use /* */ for multi-line comments
  - Document function purpose above definition
  - Explain non-obvious code inline

Line Length:
  - Maximum 80 characters per line
  - Break long lines at logical points

Headers:
  - Use include guards (#ifndef/#define/#endif)
  - Group includes: system headers, then project headers
  - Sort alphabetically within groups


FILE ORGANIZATION
-----------------

    include/            Public header files
    src/                Implementation files
    tests/              Test files and test databases
    docs/               Documentation

When adding new functionality:
  - Public declarations go in include/
  - Implementation goes in src/
  - Update Makefile if adding new source files


COMMIT MESSAGES
---------------

Write clear, descriptive commit messages:

Format:
    Short summary (50 chars or less)
    
    More detailed explanation if necessary. Wrap at 72 characters.
    Explain the problem being solved and why this approach was chosen.
    
    - Bullet points are acceptable
    - Use present tense ("Add feature" not "Added feature")

Examples:
    Add overflow page support for large records
    
    Fix memory leak in schema parsing
    
    Update documentation for cell decoding


TESTING
-------

Before submitting changes:

1. Build successfully with warnings enabled:
   
       make clean
       make CFLAGS="-Wall -Wextra -std=c11"

2. Run the test suite:
   
       make test

3. Test with various database files:
   - Empty databases
   - Single-table databases
   - Multi-table databases
   - Databases with indexes
   - Large databases

4. Check for memory leaks (if valgrind available):
   
       valgrind --leak-check=full ./bin/litereader tests/db/test.db


SUBMITTING PATCHES
------------------

Via Pull Request (preferred):
1. Push your feature branch to your fork
2. Open a pull request against main branch
3. Describe your changes clearly
4. Reference any related issues

Via Email:
1. Generate patch with: git format-patch -1
2. Send to project maintainer
3. Include description in email body


BUG REPORTS
-----------

When reporting bugs, include:

1. LiteReader version (git commit hash)
2. Operating system and version
3. GCC version
4. Steps to reproduce
5. Expected behavior
6. Actual behavior
7. Sample database file (if possible and not sensitive)

Example bug report:

    Title: Crash when parsing database with empty table
    
    Version: commit abc123
    OS: Debian 12, Linux 6.1
    GCC: 12.2.0
    
    Steps:
    1. Create database: sqlite3 test.db "CREATE TABLE t1(x);"
    2. Run: ./bin/litereader test.db
    
    Expected: Parse completes successfully
    Actual: Segmentation fault


FEATURE REQUESTS
----------------

Feature requests are welcome. When proposing new features:

1. Check existing issues for duplicates
2. Describe the use case
3. Explain proposed implementation (if you have ideas)
4. Consider backward compatibility


AREAS FOR CONTRIBUTION
----------------------

High priority:
  - Overflow page support
  - Index page cell parsing
  - Better error messages

Medium priority:
  - WAL mode support
  - JSON output format
  - CSV output format
  - Performance optimizations

Low priority:
  - Windows/macOS portability
  - Documentation improvements
  - Additional test databases


CODE REVIEW
-----------

All contributions are reviewed before merging. Reviewers check for:

  - Correctness
  - Code style compliance
  - Memory safety (no leaks, no buffer overflows)
  - Error handling
  - Documentation
  - Test coverage

Be prepared to make revisions based on feedback.


LICENSE
-------

By contributing to LiteReader, you agree that your contributions will
be licensed under the GNU General Public License v3.0.


QUESTIONS
---------

If you have questions about contributing:

1. Check existing documentation
2. Search closed issues/PRs
3. Open a new issue with your question

Thank you for contributing to LiteReader.


NOTE ON DOCUMENTATION
---------------------

This documentation was generated programmatically and may contain
inaccuracies. Please verify details against the source code and report
any discrepancies.
