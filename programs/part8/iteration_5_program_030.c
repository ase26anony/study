/* loop-doloop-test.c
 * Test program to exercise GCC's doloop_optimize pass lines 136-150
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 loop-doloop-test.c
 */

/* Global variable to prevent loop removal */
volatile int sink;

int main(void) {
    /* Use unsigned int for better decrement-and-branch optimization */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation 
         * Memory clobber prevents dead code elimination
         * but doesn't interfere with register allocation */
        __asm__ volatile("" ::: "memory");
    }
    
    /* Use the result to prevent entire loop optimization */
    sink = counter;
    
    return sink;
}
