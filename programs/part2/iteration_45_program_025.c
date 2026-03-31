/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to prevent optimization */
static int __attribute__((noinline)) helper(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Function to create high register pressure */
static int __attribute__((noinline)) create_register_pressure(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed + 1;
    int v1 = seed + 2;
    int v2 = seed + 3;
    int v3 = seed + 4;
    int v4 = seed + 5;
    int v5 = seed + 6;
    int v6 = seed + 7;
    int v7 = seed + 8;
    int v8 = seed + 9;
    int v9 = seed + 10;
    int v10 = seed + 11;
    int v11 = seed + 12;
    int v12 = seed + 13;
    int v13 = seed + 14;
    int v14 = seed + 15;
    int v15 = seed + 16;
    int v16 = seed + 17;
    int v17 = seed + 18;
    int v18 = seed + 19;
    int v19 = seed + 20;
    
    /* Volatile variable to create side effects */
    volatile int barrier = 0;
    
    /* Complex computation with dependent operations */
    /* This creates long live ranges and def-use chains */
    for (int i = 0; i < 100; i++) {
        /* Use helper to prevent CSE */
        int h = helper(i);
        
        /* Chain of dependent operations */
        v0 = v1 + v2 + h;
        v1 = v0 * v3 - h;
        v2 = v1 / (v4 + 1) + h;
        v3 = v2 ^ v5 ^ h;
        v4 = v3 | v6 | h;
        v5 = v4 & v7 & ~h;
        v6 = v5 + v8 + (h << 1);
        v7 = v6 - v9 - (h >> 1);
        v8 = v7 * v10 * (h + 1);
        v9 = v8 % (v11 + 2) + h;
        v10 = v9 ^ v12 ^ (h * 2);
        v11 = v10 | v13 | (h & 0xFF);
        v12 = v11 & v14 & (h | 0x55);
        v13 = v12 + v15 + (h % 7);
        v14 = v13 - v16 - (h % 5);
        v15 = v14 * v17 * (h % 3);
        v16 = v15 / (v18 + 3) + (h % 2);
        v17 = v16 ^ v19 ^ (h * 3);
        v18 = v17 | v0 | (h + 0xAA);
        v19 = v18 & v1 & (h ^ 0x55);
        
        /* Memory barrier to prevent reordering */
        barrier = i;
        
        /* Inline asm to clobber registers and increase pressure */
        asm volatile ("# Force register clobber" : : : 
            "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    }
    
    /* Conditional use to extend live ranges across blocks */
    int result;
    if (barrier > 50) {
        /* Complex expression that might be rematerialized */
        result = v0 + v1 - v2 * v3 / (v4 + 1) | v5 & v6 ^ v7;
        result += v8 - v9 + v10 * v11 / (v12 + 1) | v13 & v14 ^ v15;
        result += v16 - v17 + v18 * v19 / (v0 + 1) | v1 & v2 ^ v3;
    } else {
        /* Alternative computation path */
        result = v19 + v18 - v17 * v16 / (v15 + 1) | v14 & v13 ^ v12;
        result += v11 - v10 + v9 * v8 / (v7 + 1) | v6 & v5 ^ v4;
        result += v3 - v2 + v1 * v0 / (v19 + 1) | v18 & v17 ^ v16;
    }
    
    /* Final computation mixing all variables */
    result = (result + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
              v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19);
    
    return result & 0xFFFF; /* Prevent overflow issues */
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += create_register_pressure(i * 100);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
