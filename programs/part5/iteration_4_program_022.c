volatile int flag = 0;  // Declare a volatile integer initialized to 0
if (flag) {             // Check if flag is non-zero (always false)
    // These lines are never executed
    test_invalid_address_of_bitfield();
    test_incomplete_sizeof();
}
