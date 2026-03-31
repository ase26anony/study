This test script comprehensively covers all the uncovered lines:

1. **Lines 111-112 (`case 'h'`)**: Tested with `-h` and `--help` flags
2. **Lines 113-114 (`case 'v'`)**: Tested with `-v` and `--version` flags  
3. **Lines 115-116 (`case 'l'`)**: Tested with `-l` flag in multiple contexts
4. **Lines 117-118 (`case 'p'`)**: Tested with `-p` flag
5. **Lines 119-120 (`case 'r'`)**: Tested with `-r` flag
6. **Lines 121-122 (`case 's'`)**: Tested with `-s` flag
7. **Lines 123-124 (`default` case)**: Tested with invalid `-x` flag

The script also tests various combinations and edge cases:
- Individual flags on single files
- Multiple flags combined
- Different flag orders
- Multiple input files
- Mixed file types (.gcda and .gcno)
- Error conditions (invalid flag, no files, non-existent files)

To run this test, save it as `test_gcov_dump.sh`, make it executable, and run it:
