/* loop-doloop-test.c
 * Designed to generate RTL pattern: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
 * for coverage of lines 136-150 in loop-doloop.cc
 */

volatile int sink;  /* Global variable to prevent optimization */

int main(void) {
    /* Use unsigned to avoid signed overflow UB, encouraging decrement pattern */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate: compare (counter - 1) with 0
     * At RTL level: (compare (plus (reg counter) (const_int -1)) (const_int 0))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation 
         * 'memory' clobber prevents reordering/elimination
         * but doesn't force counter to memory
         */
        __asm__ volatile("" ::: "memory");
        
        /* Alternative minimal body that doesn't use counter's address:
         * sink = 1;
         */
    }
    
    /* Use the final counter value to prevent dead code elimination */
    sink = counter;
    
    return (int)counter;
}
