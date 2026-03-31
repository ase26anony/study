/* loop-doloop.c - Test case for doloop_optimize pattern matching */
volatile int sink;  /* Prevent dead code elimination */

int main(void) {
    /* Use unsigned to avoid signed overflow issues */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = 1;
    }
    
    /* Use the result to prevent loop elimination */
    return (int)counter;
}
