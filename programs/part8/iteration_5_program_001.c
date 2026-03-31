/* loop-doloop.cc test case */
/* Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 -S -o test.s test.c */

volatile int sink; /* Prevent dead code elimination */

int main(void) {
    /* Use unsigned to avoid signed overflow UB, but int works too for this pattern */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     * The decrement (-1) and comparison with 0 must be in the condition
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = 1; /* Write to volatile variable - can't be optimized away */
    }
    
    /* Use the result to prevent entire loop elimination */
    return (int)counter;
}
