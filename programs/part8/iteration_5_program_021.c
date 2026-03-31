/* loop-doloop-test.c
 * Test case for GCC's doloop_optimize pass (lines 136-150)
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 loop-doloop-test.c
 */

/* Global variable to prevent loop elimination */
volatile int sink;

int main(void) {
    /* Use unsigned to avoid signed overflow UB, but int works too */
    unsigned int counter = 1000;
    
    /* Pattern: while (--counter != 0)
     * Should generate: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation
         * Memory clobber prevents elimination but doesn't affect register allocation
         */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    sink = counter;
    
    return sink;
}
