/* loop-doloop.cc test case */
#include <stdio.h>

/* Global variable to prevent loop removal */
volatile int sink;

int main(void) {
    /* Use unsigned to avoid signed overflow UB */
    unsigned int counter = 1000;
    
    /* 
     * Pattern: while (--counter != 0)
     * Should generate: (set (cc0) (compare (plus (reg) (const_int -1)) (const_int 0)))
     */
    while (--counter != 0) {
        /* Minimal non-removable operation */
        sink = 1;
    }
    
    /* Use result to prevent dead code elimination */
    return (int)counter;
}
