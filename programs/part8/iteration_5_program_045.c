/* loop-doloop.cc - Test case for GCC doloop_optimize pattern matching */

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
        sink = 42;  /* Write to volatile to prevent dead code elimination */
    }
    
    /* Use the result to prevent entire loop optimization */
    return (int)counter;
}
