volatile int flag = 1;  // Change to 1 to execute the tests

// Function prototypes (these would need to be defined elsewhere)
void test_invalid_address_of_bitfield(void);
void test_incomplete_sizeof(void);

int main(void) {
    if (flag) {
        test_invalid_address_of_bitfield();
        test_incomplete_sizeof();
    }
    return 0;
}
