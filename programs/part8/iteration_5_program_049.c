/* loop-doloop.cc test case */
/* Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 -S this_file.c */

volatile int sink; /* Prevent complete optimization */

int main(void) {
    /* Use unsigned to avoid signed overflow UB, but int works too */
    unsigned int counter = 1000;
    
    /* Pattern: while (--counter != 0)
     * Should generate: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = 1; /* Write to volatile prevents dead code elimination */
    }
    
    /* Use result to prevent loop removal */
    return (int)counter;
}
