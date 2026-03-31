/* loop-doloop-test.c
 * Test case for GCC's doloop_optimize pass (lines 136-150)
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 loop-doloop-test.c
 */

/* Global variable to prevent loop elimination */
volatile int sink = 0;

int main(void) {
    /* Use unsigned int to avoid signed overflow issues */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation
         * Memory clobber prevents optimization of loop body
         */
        __asm__ volatile ("" ::: "memory");
        
        /* Also write to global to ensure loop isn't eliminated */
        sink = counter;
    }
    
    /* Use the result to prevent dead code elimination */
    return (int)counter;
}
