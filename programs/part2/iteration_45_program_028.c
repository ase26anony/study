/* Test program for GCC early rematerialization pass
 * Targeting lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter += x;
    return counter & 0xFF;
}

/* Main test function with high register pressure */
static int __attribute__((noinline)) test_remat(int iterations) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = 1;
    int v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6;
    int v6 = 7, v7 = 8, v8 = 9, v9 = 10, v10 = 11;
    int v11 = 12, v12 = 13, v13 = 14, v14 = 15, v15 = 16;
    int v16 = 17, v17 = 18, v18 = 19, v19 = 20, v20 = 21;
    int v21 = 22, v22 = 23, v23 = 24, v24 = 25, v25 = 26;
    int v26 = 27, v27 = 28, v28 = 29, v29 = 30, v30 = 31;
    
    volatile int memory_barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Create def-use chains across operations */
        v1 = v0 + v1 + side_effect(i);
        v2 = v1 * v2 - v0;
        v3 = v2 + v3 + (v1 >> 2);
        v4 = v3 * v4 - v2;
        v5 = v4 + v5 + (v3 & 0xF);
        v6 = v5 * v6 - v4;
        v7 = v6 + v7 + (v5 ^ 0xAA);
        v8 = v7 * v8 - v6;
        v9 = v8 + v9 + (v7 | 0x55);
        v10 = v9 * v10 - v8;
        
        /* Conditional assignments to create basic blocks */
        if (i & 1) {
            v11 = v10 + v11 + v0;
            v12 = v11 * v12 - v10;
        } else {
            v11 = v10 - v11 + v0;
            v12 = v11 / (v12 ? v12 : 1) + v10;
        }
        
        v13 = v12 + v13 + (v11 << 2);
        v14 = v13 * v14 - v12;
        v15 = v14 + v15 + (v13 % 17);
        v16 = v15 * v16 - v14;
        v17 = v16 + v17 + (v15 ^ v16);
        v18 = v17 * v18 - v16;
        v19 = v18 + v19 + (v17 & v18);
        v20 = v19 * v20 - v18;
        
        /* Memory barrier to prevent reordering */
        memory_barrier = i;
        
        v21 = v20 + v21 + memory_barrier;
        v22 = v21 * v22 - v20;
        v23 = v22 + v23 + (v21 >> 3);
        v24 = v23 * v24 - v22;
        v25 = v24 + v25 + (v23 & 0x7F);
        v26 = v25 * v26 - v24;
        v27 = v26 + v27 + (v25 ^ 0xCC);
        v28 = v27 * v28 - v26;
        v29 = v28 + v29 + (v27 | 0x33);
        v30 = v29 * v30 - v28;
        
        /* Force v0 to be used in multiple places */
        v0 = v30 + v0 + side_effect(v29);
        
        /* Inline asm with clobbered registers to increase pressure */
        asm volatile ("# Dummy asm" : : "r"(v0), "r"(v15), "r"(v30) : "memory");
    }
    
    /* Combine results to ensure all values are used */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    
    return result & 0xFFFF;
}

/* Variant with different computation pattern */
static int __attribute__((noinline)) test_remat2(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    volatile int sync = 0;
    
    /* Different computation pattern to create varied RTL */
    for (int x = 0; x < 100; x++) {
        a = b * c + side_effect(x);
        b = c * d - a;
        c = d * e + b;
        d = e * f - c;
        e = f * g + d;
        f = g * h - e;
        g = h * i + f;
        h = i * j - g;
        i = j * k + h;
        j = k * l - i;
        k = l * m + j;
        l = m * n - k;
        m = n * o + l;
        n = o * p - m;
        o = p * a + n;
        p = a * b - o;
        
        sync = x;
        
        /* Conditional with same computation in both paths */
        if (sync & 2) {
            a = b + c + side_effect(a);
        } else {
            a = b - c + side_effect(a);  /* Same side effect, different computation */
        }
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

int main() {
    int total = 0;
    
    /* Call with different parameters to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_remat(50 + i);
        total += test_remat2(i * 7);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
