#include <stdio.h>

volatile int flag = 0;

// Function declarations (assuming these exist elsewhere)
void test_invalid_address_of_bitfield(void);
void test_incomplete_sizeof(void);

int main(void) {
    // flag might be changed by:
    // - Hardware interrupt
    // - Another thread
    // - Debugger/modification at runtime
    
    if (flag) {
        test_invalid_address_of_bitfield();
        test_incomplete_sizeof();
    }
    
    return 0;
}
