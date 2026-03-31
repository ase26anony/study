volatile int flag = 0;

// Somewhere else, flag might be set to 1 by:
// - Hardware interrupt
// - Another thread
// - Debugger/modification

if (flag) {
    test_invalid_address_of_bitfield();
    test_incomplete_sizeof();
}
