/* loop-doloop.cc test case for doloop_optimize pattern matching */
#include <stdio.h>

/* Global variable to prevent loop elimination */
volatile int sink = 0;

int main() {
    /* Use unsigned to avoid signed overflow issues */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate RTL: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = counter;
        
        /* 
         * Memory clobber to prevent reordering/elimination
         * but keep it simple to not obscure the loop pattern
         */
        asm volatile("" ::: "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final counter: %u\n", counter);
    
    return (int)counter;
}
