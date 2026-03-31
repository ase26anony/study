/* early-remat-trigger.c
 * Designed to trigger virtual register creation in GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 early-remat-trigger.c -o test
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects and prevent optimization */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter += x;
    return counter & 1;
}

/* Test function designed to create high register pressure and rematerialization candidates */
static int __attribute__((noinline)) trigger_remat(int seed) {
    /* Declare many variables to create register pressure */
    register int v0 asm ("r12") = seed + 1;
    int v1 = seed * 2;
    int v2 = seed / 3;
    int v3 = seed ^ 0x55AA;
    int v4 = seed << 2;
    int v5 = seed >> 1;
    int v6 = seed + 0x1234;
    int v7 = seed - 0x5678;
    int v8 = seed | 0xF0F0;
    int v9 = seed & 0x0F0F;
    int v10 = seed * 3;
    int v11 = seed + v0;
    int v12 = v1 * v2;
    int v13 = v3 ^ v4;
    int v14 = v5 + v6;
    int v15 = v7 - v8;
    int v16 = v9 | v10;
    int v17 = v11 & v12;
    int v18 = v13 + v14;
    int v19 = v15 - v16;
    int v20 = v17 * v18;
    int v21 = v19 ^ v20;
    
    /* Volatile variable to create memory side effects */
    volatile int barrier = 0;
    
    /* Complex loop with dependent computations to create long live ranges */
    for (int i = 0; i < 100; i++) {
        /* Mix register and automatic variables in expressions */
        v0 = v1 + v2 + side_effect(i);
        v1 = v3 * v4 - v0;
        v2 = v5 ^ v6 | v1;
        v3 = v7 & v8 + v2;
        v4 = v9 - v10 * v3;
        v5 = v11 | v12 ^ v4;
        v6 = v13 + v14 - v5;
        v7 = v15 * v16 & v6;
        v8 = v17 - v18 | v7;
        v9 = v19 ^ v20 + v8;
        v10 = v21 * v0 - v9;
        
        /* Recompute same values in different ways (rematerialization candidates) */
        int t1 = v1 + v2;  /* Same as part of v0 computation above */
        int t2 = v3 * v4;  /* Same as part of v1 computation above */
        int t3 = v5 ^ v6;  /* Same as part of v2 computation above */
        
        /* Use the recomputed values with volatile barrier to prevent elimination */
        barrier = t1 + t2 + t3;
        
        /* More dependent computations extending live ranges */
        v11 = v12 + v13 + barrier;
        v12 = v14 * v15 - v11;
        v13 = v16 ^ v17 | v12;
        v14 = v18 & v19 + v13;
        v15 = v20 - v21 * v14;
        v16 = v0 | v1 ^ v15;
        v17 = v2 + v3 - v16;
        v18 = v4 * v5 & v17;
        v19 = v6 - v7 | v18;
        v20 = v8 ^ v9 + v19;
        v21 = v10 * v11 - v20;
        
        /* Inline asm with clobbered registers to increase pressure */
        asm volatile ("# Dummy asm" : : "r"(v0), "r"(v1), "r"(v2) : "r12", "r13", "r14");
    }
    
    /* Conditional assignments to create different def-use chains */
    int result;
    if (barrier > 50) {
        result = v0 + v1 + v2 + v3;
    } else if (barrier > 25) {
        result = v4 * v5 - v6;
    } else {
        result = v7 ^ v8 | v9;
    }
    
    /* Final computation using many variables */
    result += v10 - v11 + v12 * v13 - v14 ^ v15 | v16 & v17 - v18 + v19 * v20 - v21;
    
    return result;
}

/* Alternate test function with different pattern */
static int __attribute__((noinline)) trigger_remat2(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed * 2, f = seed * 3, g = seed * 4, h = seed * 5;
    
    volatile int sync = 0;
    
    for (int i = 0; i < 50; i++) {
        /* Chain of dependent computations */
        a = b + c + side_effect(i);
        b = c * d - a;
        c = d ^ e | b;
        d = e & f + c;
        e = f - g * d;
        f = g | h ^ e;
        g = h + a - f;
        h = b * c & g;
        
        /* Duplicate computations for rematerialization */
        int tmp1 = b + c;  /* Same as part of 'a' computation */
        int tmp2 = c * d;  /* Same as part of 'b' computation */
        
        sync = tmp1 * tmp2;
        
        /* Continue the chain */
        a = a + sync;
        b = b - sync;
        c = c ^ sync;
        d = d | sync;
    }
    
    return a + b + c + d + e + f + g + h;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += trigger_remat(i * 100);
        total += trigger_remat2(i * 50 + 1);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
