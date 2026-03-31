#include <stdio.h>

volatile int flag = 0;

struct Example {
    unsigned int bitfield : 4;  // Bitfield member
};

struct Incomplete;  // Forward declaration - incomplete type

void test_invalid_address_of_bitfield() {
    struct Example ex;
    // This would cause a compilation error:
    // unsigned int* ptr = &ex.bitfield;  // ERROR: cannot take address of bitfield
    printf("Cannot take address of bitfield member\n");
}

void test_incomplete_sizeof() {
    // This would cause a compilation error:
    // size_t size = sizeof(struct Incomplete);  // ERROR: invalid application of sizeof to incomplete type
    printf("Cannot use sizeof on incomplete type\n");
}

int main() {
    if (flag) {
        test_invalid_address_of_bitfield();
        test_incomplete_sizeof();
    }
    return 0;
}
