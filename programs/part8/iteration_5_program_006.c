/* loop-doloop.cc test case for GCC doloop_optimize pattern matching */
volatile int sink;  /* Global variable to prevent optimization */

int main(void) {
    /* Use unsigned int for better decrement-and-branch optimization */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = 1;  /* Write to volatile variable to prevent dead code elimination */
        
        /* Optional: Add memory barrier to prevent reordering */
        __asm__ volatile ("" ::: "memory");
    }
    
    /* Use the counter to prevent entire loop from being optimized out */
    return (int)counter;
}
