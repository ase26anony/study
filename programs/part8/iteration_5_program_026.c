/* loop-doloop.cc test case for doloop_optimize pattern matching */
volatile int sink;  /* Global variable to prevent optimization */

int main(void) {
    /* Use unsigned int to avoid signed overflow issues */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * This should generate RTL pattern:
     * (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = 1;  /* Write to volatile to prevent dead code elimination */
    }
    
    /* Use the counter to prevent entire loop from being optimized out */
    return (int)counter;
}
