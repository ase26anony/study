/* loop-doloop.cc test case for doloop_optimize pattern matching */
volatile int sink; /* Prevent loop removal */

int main(void) {
    /* Use unsigned to avoid signed overflow UB, encouraging decrement pattern */
    unsigned int counter = 1000;
    
    /* Pattern: while(--counter != 0) 
     * Should generate: (compare (plus (reg) (const_int -1)) (const_int 0)) */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = 1;
    }
    
    /* Use result to prevent dead code elimination */
    return (int)counter;
}
