/* loop-doloop-test.c
 * Designed to generate RTL pattern: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
 * for coverage of lines 136-150 in loop-doloop.cc
 */

volatile int sink;  /* Prevent dead code elimination */

int main(void) {
    /* Use unsigned to avoid signed overflow UB, but int works too for this pattern */
    unsigned int counter = 1000;
    
    /* Pattern: while (--counter != 0)
     * This should generate:
     *   decrement counter (counter - 1)
     *   compare result with 0
     *   branch based on comparison
     */
    while (--counter != 0) {
        /* Minimal non-removable operation
         * 'memory' clobber prevents reordering/elimination
         * but doesn't force counter to memory
         */
        __asm__ volatile ("" : : : "memory");
        
        /* Alternative: simple store to volatile variable */
        /* sink = counter; */
    }
    
    /* Use the result to prevent entire loop elimination */
    sink = counter;
    
    return (int)counter;
}
