volatile int flag = 0;

/* Some mechanism that could change flag */
/* e.g., hardware register, interrupt handler, another thread */

if (flag) {
    test_invalid_address_of_bitfield();
    test_incomplete_sizeof();
}
