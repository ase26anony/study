/* loop-doloop.cc - Test case for doloop_optimize pattern matching */
/* Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 -S this_file.c */

volatile int sink; /* Prevent loop elimination */

int main(void) {
    /* Use unsigned to avoid signed overflow UB, encouraging decrement pattern */
    unsigned int counter = 1000;
    
    /* Pattern: while(--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation
         * 'memory' clobber prevents reordering/elimination
         * 'cc' clobber preserves condition codes */
        asm volatile("" ::: "memory", "cc");
    }
    
    /* Use result to prevent dead code elimination */
    sink = counter;
    
    return sink;
}
