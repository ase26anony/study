/* loop-doloop.cc test case */
volatile int sink; /* Prevent loop elimination */

int main() {
    /* Use unsigned to avoid signed overflow UB, but int works too */
    unsigned int counter = 1000;
    
    /* Pattern: while (--counter != 0)
     * Should generate: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = counter; /* Write to volatile to prevent dead code elimination */
        
        /* Alternative: asm volatile("" ::: "memory"); */
    }
    
    /* Use result to prevent entire function optimization */
    return (int)counter;
}
