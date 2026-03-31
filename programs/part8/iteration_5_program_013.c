/* loop-doloop-test.c
 * Designed to generate RTL pattern: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
 * for coverage of lines 136-150 in loop-doloop.cc
 */

volatile int sink;  /* Global variable to prevent optimization */

int main(void) {
    /* Use unsigned int to avoid signed overflow UB */
    unsigned int counter = 1000;
    
    /* Pattern: while (--counter != 0)
     * This should generate:
     * 1. SET with source as COMPARE
     * 2. COMPARE's second operand is const0_rtx
     * 3. COMPARE's first operand is PLUS expression
     * 4. PLUS adds register and const_int -1
     * 5. Destination is condition code register
     */
    while (--counter != 0) {
        /* Minimal non-removable operation
         * asm with memory clobber prevents dead code elimination
         * but doesn't interfere with loop structure */
        __asm__ volatile("" ::: "memory");
        
        /* Alternative: simple store to volatile global */
        sink = 1;
    }
    
    /* Use the counter value to prevent entire loop removal */
    return (int)counter;
}
