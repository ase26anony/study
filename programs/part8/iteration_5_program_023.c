/* loop-doloop.cc coverage test */
/* Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 -S -o test.s test.c */

volatile int sink; /* Prevent loop from being optimized away */

int main(void) {
    /* Use unsigned to avoid signed overflow UB, but int works too for this pattern */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = 1; /* Write to volatile variable */
    }
    
    /* Use result to prevent dead code elimination */
    return (int)counter;
}
