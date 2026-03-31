/* loop-doloop.cc test case for doloop_optimize pattern matching */
volatile int sink;  /* Global variable to prevent optimization */

int main(void) {
    /* Use unsigned int to ensure well-defined wrap-around behavior */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * This should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = counter;  /* Write to volatile global to prevent dead code elimination */
        
        /* Alternative: inline asm with memory clobber */
        /* asm volatile("" ::: "memory"); */
    }
    
    /* Use the final counter value to prevent loop removal */
    return (int)counter;
}
