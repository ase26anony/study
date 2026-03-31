/* loop-doloop-test.c
 * Test program to exercise GCC's doloop_optimize pass lines 136-150
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 loop-doloop-test.c
 */

volatile int sink; /* Global to prevent optimization */

int main(void) {
    /* Use unsigned to avoid signed overflow issues */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = 1; /* Write to volatile to prevent dead code elimination */
    }
    
    /* Use the result to prevent entire loop optimization */
    return (int)counter;
}
