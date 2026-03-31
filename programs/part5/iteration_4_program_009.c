volatile int flag = 0;

// Some mechanism that could change flag (e.g., interrupt, another thread)

if (flag) {  // Compiler must read flag from memory each time
    test_invalid_address_of_bitfield();
    test_incomplete_sizeof();
}
