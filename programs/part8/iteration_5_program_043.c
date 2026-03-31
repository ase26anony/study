/* loop-doloop.cc test case */
volatile int sink; /* Prevent loop elimination */

int main(void) {
    int counter = 1000;
    
    /* Pattern: while (--counter != 0) 
     * Should generate: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = counter;
    }
    
    /* Use result to prevent dead code elimination */
    return counter;
}
