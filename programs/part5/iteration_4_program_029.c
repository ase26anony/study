volatile int flag = 0;

/* Some mechanism to change flag (e.g., interrupt, another thread) */

if (flag) {
    test_invalid_address_of_bitfield();
    test_incomplete_sizeof();
}
