#include <stdio.h>

// Function declarations (assuming these exist elsewhere)
void test_invalid_address_of_bitfield(void);
void test_incomplete_sizeof(void);

int main(void) {
    volatile int flag = 0;
    
    if (flag) {
        test_invalid_address_of_bitfield();
        test_incomplete_sizeof();
    }
    
    return 0;
}
