/* loop-doloop.cc test case */
volatile int sink;  /* Prevent dead code elimination */

int main(void) {
    /* Use unsigned to avoid signed overflow UB while still allowing -- != 0 */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = 1;  /* Write to volatile variable */
    }
    
    /* Use result to prevent optimization */
    return (int)counter;
}
