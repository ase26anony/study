/* loop-doloop.cc test case */
/* Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 -S this_file.c */

/* Global variable to prevent loop elimination */
volatile int sink;

int main(void) {
    /* Use unsigned to avoid signed overflow issues */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * This should generate RTL pattern:
     * (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal loop body that can't be optimized away */
        sink = counter;  /* Write to volatile global */
        
        /* Alternative: asm volatile("" ::: "memory"); */
    }
    
    /* Use the result to prevent dead code elimination */
    return (int)counter;
}
