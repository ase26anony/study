volatile int flag = 0;
if (flag) {
    // This code will never execute because flag is initialized to 0
    test_invalid_address_of_bitfield();
    test_incomplete_sizeof();
}
