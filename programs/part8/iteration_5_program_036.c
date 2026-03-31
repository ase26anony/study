/* loop-doloop.c - Test case for doloop_optimize pattern matching */

volatile int global_sink;  /* Prevent dead code elimination */

int main(void) {
    /* Use unsigned int to avoid signed overflow UB, encouraging loop optimization */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        global_sink = counter;  /* Write to volatile global */
        
        /* Alternative: inline asm with memory clobber */
        /* asm volatile("" ::: "memory"); */
    }
    
    /* Use the final counter value to prevent loop removal */
    return (int)counter;
}
