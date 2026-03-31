volatile int flag = 0;

// Option 1: Use a non-volatile copy
int current_flag = flag;
if (current_flag) {
    // test code
}

// Option 2: Use preprocessor directives
#ifdef ENABLE_TESTS
    test_invalid_address_of_bitfield();
    test_incomplete_sizeof();
#endif

// Option 3: Use function pointers (if tests must be runtime-configurable)
typedef void (*test_func_t)(void);
test_func_t test_functions[] = {test_invalid_address_of_bitfield, test_incomplete_sizeof};
// Then call conditionally based on flag
