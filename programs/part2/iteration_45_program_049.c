/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test_remat test_remat.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test_remat_debug test_remat.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter += x;
    return counter & 0xFF;
}

/* Function to create high register pressure and trigger rematerialization */
static int __attribute__((noinline)) create_register_pressure(int seed) {
    /* Declare many variables to create pseudo registers */
    register int r0 asm ("r12") = seed + 1;
    register int r1 asm ("r13") = seed + 2;
    int v0 = seed * 3;
    int v1 = seed * 5;
    int v2 = seed * 7;
    int v3 = seed * 11;
    int v4 = seed * 13;
    int v5 = seed * 17;
    int v6 = seed * 19;
    int v7 = seed * 23;
    int v8 = seed * 29;
    int v9 = seed * 31;
    int v10 = seed * 37;
    int v11 = seed * 41;
    int v12 = seed * 43;
    int v13 = seed * 47;
    int v14 = seed * 53;
    int v15 = seed * 59;
    int v16 = seed * 61;
    int v17 = seed * 67;
    int v18 = seed * 71;
    int v19 = seed * 73;
    
    /* Create complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* First computation chain - creates long dependency chain */
        v0 = v1 + v2 + side_effect(i);
        v1 = v0 * v3 - side_effect(v0);
        v2 = v1 / (v4 + 1) + side_effect(v1);
        v3 = v2 ^ v5 ^ side_effect(v2);
        v4 = v3 | v6 | side_effect(v3);
        v5 = v4 & v7 & side_effect(v4);
        
        /* Second independent chain using register variables */
        r0 = r1 + v8 + side_effect(r1);
        r1 = r0 * v9 - side_effect(r0);
        v8 = r1 + v10 + side_effect(r1);
        v9 = v8 * v11 - side_effect(v8);
        
        /* Third chain with conditional to create basic blocks */
        if (i & 1) {
            v10 = v12 + v13 + side_effect(v12);
            v11 = v10 * v14 - side_effect(v10);
        } else {
            v10 = v15 + v16 + side_effect(v15);
            v11 = v10 * v17 - side_effect(v10);
        }
        
        /* Fourth chain with memory-like operations */
        v12 = v18 + v19 + side_effect(v18);
        v13 = v12 * v0 - side_effect(v12);
        v14 = v13 / (v1 + 1) + side_effect(v13);
        
        /* Mix all chains together to create register pressure */
        v15 = v2 + v5 + v8 + v11 + side_effect(v2);
        v16 = v3 + v6 + v9 + v12 + side_effect(v3);
        v17 = v4 + v7 + v10 + v13 + side_effect(v4);
        v18 = v15 * v16 - side_effect(v15);
        v19 = v17 ^ v18 ^ side_effect(v17);
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("# Dummy asm to clobber registers" 
                     : 
                     : "r" (r0), "r" (r1), "r" (v0), "r" (v1), "r" (v2), "r" (v3)
                     : "memory");
    }
    
    /* Final computation using all variables to ensure they're live */
    int result = r0 + r1 + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    return result & 0xFFFF;
}

/* Main function to call test repeatedly with different seeds */
int main() {
    int total = 0;
    
    /* Call multiple times to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += create_register_pressure(i * 100);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
