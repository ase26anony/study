/* loop-doloop-test.c
 * Test program to exercise GCC's doloop_optimize pass lines 136-150
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 loop-doloop-test.c
 */

/* Global variable to prevent loop elimination */
volatile int sink = 0;

int main(void) {
    /* Use unsigned to avoid signed overflow issues at zero */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * This should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = counter;  /* Store to volatile prevents elimination */
    }
    
    /* Use result to prevent dead code elimination */
    return (int)counter;
}
