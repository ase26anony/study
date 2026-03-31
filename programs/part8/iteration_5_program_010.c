/* loop-doloop.cc test case for doloop_optimize pattern matching */
volatile int sink; /* Prevent loop elimination */

int main() {
    /* Use unsigned to avoid signed overflow issues */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = 42; /* Write to volatile variable */
        
        /* Alternative: asm volatile("" ::: "memory"); */
    }
    
    /* Use result to prevent dead code elimination */
    return (int)counter;
}
