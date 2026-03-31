/* loop-doloop-test.c
 * Test case for GCC's doloop_optimize pass lines 136-150
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 loop-doloop-test.c
 */

volatile int sink; /* Prevent loop removal */

int main(void) {
    /* Use unsigned to avoid overflow issues with decrement to zero */
    unsigned int counter = 1000;
    
    /* Pattern: while (--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation
         * Memory clobber prevents dead code elimination
         * but doesn't interfere with register allocation
         */
        __asm__ volatile ("" ::: "memory");
    }
    
    /* Use the result to prevent entire function optimization */
    sink = counter;
    
    return sink;
}
