/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    volatile int dummy = x;
    return dummy + 1;
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
    v1 = side_effect(seed + 1);
    v2 = seed * 2;
    v3 = side_effect(seed + 3);
    
    /* Use explicit register variables in computation */
    r0 = v0 + v1;
    r1 = v2 - v3;
    
    /* Create complex dependency chain across many variables */
    v4 = r0 * r1 + v0;
    v5 = side_effect(v4) - v1;
    v6 = v2 + v3 * r0;
    v7 = r1 - side_effect(v5);
    v8 = v4 * v5 + v6;
    v9 = v7 - side_effect(v6);
    v10 = v8 + v9 * r0;
    v11 = side_effect(v9) - r1;
    v12 = v10 * v11 + v4;
    v13 = v5 - side_effect(v12);
    v14 = v6 + v7 * v8;
    v15 = side_effect(v13) - v9;
    v16 = v10 * v11 + v12;
    v17 = v13 - side_effect(v14);
    v18 = v15 + v16 * r0;
    v19 = side_effect(v17) - r1;
    v20 = v18 * v19 + v8;
    v21 = v9 - side_effect(v20);
    v22 = v10 + v11 * v12;
    v23 = side_effect(v21) - v13;
    v24 = v14 * v15 + v16;
    v25 = v17 - side_effect(v22);
    v26 = v18 + v19 * v20;
    v27 = side_effect(v23) - v21;
    v28 = v24 * v25 + v26;
    v29 = v27 - side_effect(v28);
    
    /* Create conditional branches to split basic blocks */
    if (seed & 1) {
        /* Different computation path to create separate basic block */
        v0 = v29 + v28;
        v1 = side_effect(v0) - v27;
        v2 = v26 * v25;
    } else {
        /* Alternative path */
        v0 = v29 - v28;
        v1 = side_effect(v0) + v27;
        v2 = v26 / (v25 ? v25 : 1);
    }
    
    /* More arithmetic to extend live ranges */
    v3 = v0 + v1 * v2;
    v4 = side_effect(v3) - v29;
    v5 = v28 * v27 + v26;
    v6 = v25 - side_effect(v4);
    
    /* Use inline asm to create register clobbering */
    asm volatile (
        "addl %1, %0\n\t"
        "subl %2, %0"
        : "+r" (v3)
        : "r" (v4), "r" (v5)
        : "cc"
    );
    
    /* Final computation mixing all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 +
                 v7 + v8 + v9 + v10 + v11 + v12 + v13 +
                 v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 +
                 v26 + v27 + v28 + v29;
    
    /* Use result in side effect to prevent elimination */
    return side_effect(result);
}

/* Alternative version with different computation pattern */
static int __attribute__((noinline)) create_register_pressure2(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Unrolled computation loop */
    e = a + b * c - d;
    f = side_effect(e) + a - b;
    g = c * d + side_effect(f);
    h = e - f * g;
    i = side_effect(h) + a * b;
    j = c + d - side_effect(i);
    k = e * f + g;
    l = h - side_effect(j);
    m = i + j * k;
    n = side_effect(l) - m;
    o = k * l + m;
    p = n - side_effect(o);
    q = m + n * o;
    r = side_effect(p) - q;
    s = o * p + q;
    t = r - side_effect(s);
    
    /* Conditional with overlapping live ranges */
    if (seed & 2) {
        a = t + s;
        b = side_effect(a) - r;
        c = q * p;
    } else {
        a = t - s;
        b = side_effect(a) + r;
        c = q / (p ? p : 1);
    }
    
    /* Force register usage with asm */
    register int tmp1 asm ("r14") = a + b;
    register int tmp2 asm ("r15") = c + d;
    
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0"
        : "+r" (tmp1)
        : "r" (tmp2), "r" (e)
        : "cc"
    );
    
    return side_effect(tmp1 + tmp2 + a + b + c + d + e + f + g + h + i + j + 
                      k + l + m + n + o + p + q + r + s + t);
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += create_register_pressure(i);
        total += create_register_pressure2(i * 3 + 1);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
