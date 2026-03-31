/* loop-doloop.cc test case for doloop_optimize pattern matching */
volatile int sink;  /* Global to prevent optimization */

int main(void) {
    /* Use unsigned to avoid signed overflow UB, encouraging loop optimization */
    unsigned int counter = 1000;
    
    /* Pattern: while (--counter != 0)
     * Should generate: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation - memory clobber prevents elimination */
        asm volatile("" ::: "memory");
        
        /* Alternative minimal body that won't be optimized away:
         * sink = counter;
         */
    }
    
    /* Use result to prevent dead code elimination */
    sink = counter;
    
    return (int)counter;
}
