/* loop-doloop.cc test case for doloop_optimize pattern matching */
/* Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 -S this_file.c */

volatile int sink; /* Prevent dead code elimination */

int main(void) {
    /* Use unsigned to avoid signed overflow UB, encouraging decrement pattern */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = 1; /* Write to volatile prevents loop removal */
    }
    
    /* Use counter to prevent entire loop elimination */
    return (int)counter;
}
