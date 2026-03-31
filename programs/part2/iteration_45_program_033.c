/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    volatile int dummy = 0;
    dummy = x;
    return dummy + 1;
}

/* Function with high register pressure designed to trigger early remat */
static int __attribute__((noinline)) high_pressure_computation(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed;
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed / 3;
    int v4 = seed ^ 0x55;
    int v5 = seed | 0xAA;
    int v6 = seed & 0xFF;
    int v7 = seed << 1;
    int v8 = seed >> 2;
    int v9 = seed + 100;
    int v10 = seed - 50;
    int v11 = seed * 3;
    int v12 = seed / 2;
    int v13 = seed ^ 0xCC;
    int v14 = seed | 0x33;
    int v15 = seed & 0xF0;
    int v16 = seed << 2;
    int v17 = seed >> 1;
    int v18 = seed + 200;
    int v19 = seed - 100;
    
    /* Create complex def-use chains with side effects */
    for (int i = 0; i < 10; i++) {
        /* Force recomputation by mixing with side effects */
        v0 = side_effect(v0) + v1;
        v1 = v0 * v2 + side_effect(v1);
        v2 = v1 - v3 * side_effect(v2);
        v3 = v2 ^ v4 + side_effect(v3);
        v4 = v3 | v5 - side_effect(v4);
        v5 = v4 & v6 * side_effect(v5);
        v6 = v5 << v7 + side_effect(v6);
        v7 = v6 >> v8 - side_effect(v7);
        v8 = v7 + v9 * side_effect(v8);
        v9 = v8 - v10 + side_effect(v9);
        v10 = v9 * v11 - side_effect(v10);
        v11 = v10 ^ v12 + side_effect(v11);
        v12 = v11 | v13 - side_effect(v12);
        v13 = v12 & v14 * side_effect(v13);
        v14 = v13 << v15 + side_effect(v14);
        v15 = v14 >> v16 - side_effect(v15);
        v16 = v15 + v17 * side_effect(v16);
        v17 = v16 - v18 + side_effect(v17);
        v18 = v17 * v19 - side_effect(v18);
        v19 = v18 ^ v0 + side_effect(v19);
        
        /* Create conditional assignments to extend live ranges */
        if (i & 1) {
            v0 = v19 + v1;
            v2 = v1 - v3;
            v4 = v3 | v5;
            v6 = v5 & v7;
            v8 = v7 + v9;
            v10 = v9 - v11;
            v12 = v11 ^ v13;
            v14 = v13 | v15;
            v16 = v15 & v17;
            v18 = v17 + v19;
        } else {
            v1 = v0 * v2;
            v3 = v2 - v4;
            v5 = v4 ^ v6;
            v7 = v6 | v8;
            v9 = v8 - v10;
            v11 = v10 * v12;
            v13 = v12 ^ v14;
            v15 = v14 | v16;
            v17 = v16 & v18;
            v19 = v18 + v0;
        }
    }
    
    /* Mix all results to ensure they're live at the end */
    int result = v0 + v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8 + v9 
                 - v10 + v11 - v12 + v13 - v14 + v15 - v16 + v17 - v18 + v19;
    
    /* Use inline asm to create register pressure and clobber registers */
    asm volatile (
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        : "+r" (result)
        : "r" (v0), "r" (v1)
        : "cc", "memory"
    );
    
    asm volatile (
        "xorl %1, %0\n\t"
        "orl %2, %0\n\t"
        : "+r" (result)
        : "r" (v2), "r" (v3)
        : "cc"
    );
    
    return result;
}

/* Main function to prevent constant propagation */
int main() {
    int total = 0;
    
    /* Call with different seeds to prevent optimization */
    for (int i = 0; i < 100; i++) {
        total += high_pressure_computation(i);
        total -= high_pressure_computation(i * 2);
        total ^= high_pressure_computation(i * 3);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
