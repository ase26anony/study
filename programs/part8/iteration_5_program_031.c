/* loop-doloop.cc test case for doloop_optimize pattern matching */
volatile int sink;  /* Global volatile to prevent optimization */

int main(void) {
    /* Use unsigned to avoid signed overflow UB, encouraging loop optimization */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * This should generate RTL pattern:
     * (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = counter;  /* Write to volatile prevents dead code elimination */
        
        /* Alternative: asm volatile("" ::: "memory"); */
    }
    
    /* Use result to prevent entire loop from being optimized away */
    return (int)counter;
}
