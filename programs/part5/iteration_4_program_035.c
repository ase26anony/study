#include <stdio.h>

// Function declarations (assuming they exist elsewhere)
void test_invalid_address_of_bitfield(void);
void test_incomplete_sizeof(void);

int main(void) {
    volatile int flag = 1;  // Changed to 1 to execute the tests
    
    if (flag) {
        test_invalid_address_of_bitfield();
        test_incomplete_sizeof();
    }
    
    return 0;
}
