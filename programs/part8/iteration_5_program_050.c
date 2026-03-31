/* loop-doloop-test.c
 * Designed to generate RTL pattern: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
 * for coverage of lines 136-150 in loop-doloop.cc
 */

volatile int global_sink;  /* Prevents complete optimization of loop body */

int main(void) {
    /* Use unsigned to avoid signed overflow UB, encouraging decrement pattern */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate: compare (counter - 1) with 0
     * At RTL level: (compare (plus (reg:SI counter) (const_int -1)) (const_int 0))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        global_sink = counter;  /* Write to volatile global */
        
        /* Alternative: inline asm with memory clobber */
        /* asm volatile("" ::: "memory"); */
    }
    
    /* Use result to prevent dead code elimination */
    return (int)counter;
}
