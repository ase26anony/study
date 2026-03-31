/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter += x;
    return counter & 1;
}

/* Test function designed to create high register pressure */
static int __attribute__((noinline)) test_remat(int iterations) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = 1;
    int v1 = 2, v2 = 3, v3 = 4, v4 = 5;
    int v5 = 6, v6 = 7, v7 = 8, v8 = 9, v9 = 10;
    int v10 = 11, v11 = 12, v12 = 13, v13 = 14, v14 = 15;
    int v15 = 16, v16 = 17, v17 = 18, v18 = 19, v19 = 20;
    int v20 = 21, v21 = 22, v22 = 23, v23 = 24, v24 = 25;
    int v25 = 26, v26 = 27, v27 = 28, v28 = 29, v29 = 30;
    
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Create long dependency chains */
        v1 = v0 + v1 + side_effect(i);
        v2 = v1 * v2 - side_effect(v1);
        v3 = v2 / (v3 + 1) + side_effect(v2);
        v4 = v3 ^ v4 ^ side_effect(v3);
        v5 = v4 | v5 | side_effect(v4);
        v6 = v5 & v6 & ~side_effect(v5);
        v7 = v6 << (v7 % 4) + side_effect(v6);
        v8 = v7 >> (v8 % 4) - side_effect(v7);
        v9 = v8 + v9 * side_effect(v8);
        v10 = v9 - v10 / (side_effect(v9) + 1);
        
        /* More computations creating register pressure */
        v11 = v10 + v11 + barrier;
        v12 = v11 * v12 - barrier;
        v13 = v12 + v13 ^ barrier;
        v14 = v13 | v14 & barrier;
        v15 = v14 << 1 + barrier;
        v16 = v15 >> 2 - barrier;
        v17 = v16 + v17 * barrier;
        v18 = v17 - v18 / (barrier + 1);
        v19 = v18 ^ v19 ^ barrier;
        v20 = v19 | v20 | barrier;
        
        /* Conditional assignments to create basic blocks */
        if (i % 3 == 0) {
            v21 = v20 + v21 + side_effect(v20);
            v22 = v21 * v22 - side_effect(v21);
        } else if (i % 3 == 1) {
            v23 = v22 + v23 + side_effect(v22);
            v24 = v23 * v24 - side_effect(v23);
        } else {
            v25 = v24 + v25 + side_effect(v24);
            v26 = v25 * v26 - side_effect(v25);
        }
        
        /* Force register reuse with asm clobber */
        asm volatile ("" : "+r" (v0), "+r" (v1), "+r" (v2)
                      : : "r8", "r9", "r10", "r11", "r13", "r14", "r15");
        
        /* More computations after asm barrier */
        v27 = v26 + v27 + side_effect(v26);
        v28 = v27 * v28 - side_effect(v27);
        v29 = v28 ^ v29 ^ side_effect(v28);
        
        /* Mix all variables to create complex live ranges */
        v0 = v29 + v0 - side_effect(v29);
        
        /* Memory barrier to prevent reordering */
        barrier = i;
    }
    
    /* Combine all results to ensure they're used */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    return result & 0xFF; /* Return small value to prevent overflow */
}

/* Alternate version with different computation pattern */
static int __attribute__((noinline)) test_remat2(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    volatile int mem = 0;
    
    /* Unrolled computation with duplicate expressions */
    for (int x = 0; x < 100; x++) {
        /* Same computation in multiple places - candidate for remat */
        int t1 = a * b + c;
        int t2 = a * b + c;  /* Duplicate - may be rematerialized */
        
        a = t1 + d + mem;
        b = t2 - e + mem;
        
        int t3 = f * g - h;
        int t4 = f * g - h;  /* Another duplicate */
        
        c = t3 ^ i ^ mem;
        d = t4 | j | mem;
        
        int t5 = k << 2;
        int t6 = k << 2;     /* Another duplicate */
        
        e = t5 + l + side_effect(x);
        f = t6 - m + side_effect(x + 1);
        
        /* Force spill/remat candidates */
        g = (a + b) * (c + d);
        h = (a + b) * (c + d);  /* Same expensive computation */
        
        i = g >> 1;
        j = h >> 1;
        
        k = (e & f) | (g & h);
        l = (e & f) | (g & h);  /* Another duplicate */
        
        m = i * j + k * l;
        n = i * j + k * l;      /* Same pattern */
        
        o = m ^ n ^ mem;
        p = o + p + side_effect(m);
        
        mem = x;
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

int main() {
    int total = 0;
    
    /* Call with different parameters to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i % 10 + 5);
        total += test_remat2(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
