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

/* Force register usage with explicit register variables */
register int r0 asm ("r12");
register int r1 asm ("r13");

/* Main test function designed to create high register pressure */
static int __attribute__((noinline)) create_register_pressure(int seed) {
    /* Declare many local variables to create pseudo registers */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    
    /* Initialize with seed to prevent constant propagation */
    v0 = seed;
    v1 = seed + 1;
    v2 = seed * 2;
    v3 = seed ^ 0x1234;
    
    /* Use explicit register variables mixed with stack variables */
    r0 = v0;
    r1 = v1;
    
    /* Create complex def-use chains across basic blocks */
    for (int i = 0; i < 100; i++) {
        /* First basic block with arithmetic chain */
        v4 = v0 + v1 + side_effect(i);
        v5 = v4 * v2 - r0;
        v6 = v5 ^ v3;
        v7 = v6 + r1;
        v8 = v7 * v4;
        v9 = v8 - v5;
        
        /* Conditional to create different execution paths */
        if (i & 1) {
            v10 = v9 * 3;
            v11 = v10 + v6;
            v12 = v11 ^ v7;
            /* Use asm to clobber registers and increase pressure */
            asm volatile ("# Force clobber" : : : "memory", "r0", "r1", "r2", "r3");
        } else {
            v10 = v9 / 2;
            v11 = v10 - v6;
            v12 = v11 | v7;
        }
        
        /* More arithmetic operations creating overlapping live ranges */
        v13 = v12 + v8;
        v14 = v13 * v9;
        v15 = v14 ^ v10;
        v16 = v15 + v11;
        v17 = v16 * v12;
        v18 = v17 - v13;
        v19 = v18 ^ v14;
        v20 = v19 + v15;
        v21 = v20 * v16;
        v22 = v21 - v17;
        v23 = v22 ^ v18;
        v24 = v23 + v19;
        v25 = v24 * v20;
        v26 = v25 - v21;
        v27 = v26 ^ v22;
        v28 = v27 + v23;
        v29 = v28 * v24;
        
        /* Rotate values to maintain dependencies across iterations */
        v0 = v29 & 0xFF;
        v1 = v25 & 0xFF;
        v2 = v26 & 0xFF;
        v3 = v27 & 0xFF;
        
        /* Mix in register variables */
        r0 = (r0 + v28) & 0xFF;
        r1 = (r1 ^ v29) & 0xFF;
        
        /* Another conditional with different computation */
        if (i % 3 == 0) {
            v4 = v0 * 7 + side_effect(v0);
            v5 = v1 * 5 - side_effect(v1);
        } else if (i % 3 == 1) {
            v4 = v0 * 3 + r0;
            v5 = v1 * 2 + r1;
        } else {
            v4 = v0 * 11;
            v5 = v1 * 13;
            /* More asm to create register constraints */
            asm volatile ("# Another clobber point" 
                         : "=r"(v4), "=r"(v5) 
                         : "0"(v4), "1"(v5)
                         : "r2", "r3", "r4", "r5");
        }
        
        /* Final computation chain in this iteration */
        v6 = v4 + v5;
        v7 = v6 * v2;
        v8 = v7 ^ v3;
        v9 = v8 + v4;
        
        /* Ensure values are used to prevent dead code elimination */
        v0 = v9 - v5;
        v1 = v6 ^ v7;
        v2 = v8 * v9;
        v3 = v0 + v1;
    }
    
    /* Combine all values to produce a result */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                 r0 + r1;
    
    return result & 0xFFFF;
}

/* Alternate version with different computation pattern */
static int __attribute__((noinline)) create_register_pressure2(int seed) {
    int a = seed, b = seed * 2, c = seed * 3, d = seed * 4;
    int e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
    
    for (int iter = 0; iter < 50; iter++) {
        /* Duplicate computation that remat might try to recompute */
        e = a + b + side_effect(iter);
        f = e * c;
        g = f - d;
        h = g ^ a;
        
        /* Same computation again with different variables */
        i = a + b + side_effect(iter);  /* Duplicate of e computation */
        j = i * c;                      /* Duplicate of f computation */
        k = j - d;                      /* Duplicate of g computation */
        l = k ^ a;                      /* Duplicate of h computation */
        
        /* Mix them together */
        m = e + i;
        n = f + j;
        o = g + k;
        p = h + l;
        
        /* More chains */
        q = m * n;
        r = o ^ p;
        s = q - r;
        t = s * m;
        u = t ^ n;
        v = u + o;
        w = v * p;
        x = w - q;
        y = x ^ r;
        z = y + s;
        
        /* Update for next iteration */
        a = z & 0xFF;
        b = (a * 3) & 0xFF;
        c = (b + 7) & 0xFF;
        d = (c ^ 0x55) & 0xFF;
        
        /* Use asm with multiple clobbers */
        asm volatile ("# Complex constraint" 
                     : "+r"(a), "+r"(b), "+r"(c), "+r"(d)
                     : 
                     : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent optimization */
    for (int i = 0; i < 10; i++) {
        total += create_register_pressure(i);
        total += create_register_pressure2(i * 7);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
