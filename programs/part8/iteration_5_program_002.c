/* loop-doloop.cc test case for doloop_optimize pattern matching */
volatile int sink;  /* Global to prevent optimization */

int main(void) {
    /* Use unsigned to avoid signed overflow issues */
    unsigned int counter = 1000;
    
    /* Pattern: while (--counter != 0)
     * Should generate: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation
         * Memory clobber prevents dead code elimination
         * but doesn't interfere with register allocation */
        __asm__ volatile("" ::: "memory");
    }
    
    /* Use the result to prevent loop removal */
    sink = counter;
    
    return sink;
}
