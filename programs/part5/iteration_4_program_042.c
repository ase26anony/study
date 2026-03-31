#include <stdio.h>

// Function prototypes (assuming these are defined elsewhere)
void test_invalid_address_of_bitfield(void);
void test_incomplete_sizeof(void);

int main(void) {
    volatile int flag = 0;
    
    // This won't execute unless flag is non-zero
    if (flag) {
        test_invalid_address_of_bitfield();
        test_incomplete_sizeof();
    }
    
    return 0;
}
