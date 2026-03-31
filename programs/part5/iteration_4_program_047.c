volatile int flag = 0;

// Some external mechanism sets flag to 1
// (e.g., hardware interrupt, another thread, etc.)

if (flag) {
    // This code runs only when flag becomes non-zero
    test_invalid_address_of_bitfield();
    test_incomplete_sizeof();
}
