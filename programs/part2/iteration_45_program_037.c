/* Test program for GCC early rematerialization pass
 * Targets uncovered lines in early-remat.cc:930-937
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Main test function with high register pressure */
static int __attribute__((noinline)) test_remat(int seed) {
    /* Declare many variables to create pseudo registers */
    register int r0 asm ("r12") = seed;
    int v1 = seed * 2;
    int v2 = seed + 1;
    int v3 = seed - 1;
    int v4 = seed ^ 0x5555;
    int v5 = seed * 3;
    int v6 = seed / 2;
    int v7 = seed << 1;
    int v8 = seed >> 1;
    int v9 = seed | 0xAAAA;
    int v10 = seed & 0xFFFF;
    int v11 = seed + 100;
    int v12 = seed - 50;
    int v13 = seed * 5;
    int v14 = seed + 200;
    int v15 = seed - 75;
    int v16 = seed * 7;
    int v17 = seed + 300;
    int v18 = seed - 100;
    int v19 = seed * 11;
    int v20 = seed + 400;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create long dependency chains */
        r0 = r0 + v1 + side_effect(i);
        v1 = v1 * v2 + r0;
        v2 = v2 + v3 * v4;
        v3 = v3 ^ v5 + v1;
        v4 = v4 | v6 * v2;
        v5 = v5 & v7 + v3;
        v6 = v6 + v8 ^ v4;
        v7 = v7 * v9 + v5;
        v8 = v8 - v10 * v6;
        v9 = v9 ^ v11 + v7;
        v10 = v10 | v12 & v8;
        v11 = v11 + v13 * v9;
        v12 = v12 - v14 ^ v10;
        v13 = v13 & v15 + v11;
        v14 = v14 | v16 * v12;
        v15 = v15 ^ v17 + v13;
        v16 = v16 + v18 & v14;
        v17 = v17 * v19 + v15;
        v18 = v18 - v20 ^ v16;
        v19 = v19 ^ r0 + v17;
        v20 = v20 | v1 & v18;
        
        /* Memory barrier to prevent reordering */
        barrier = i;
        
        /* Use asm to clobber registers */
        asm volatile ("" : : "r"(r0), "r"(v1), "r"(v2), "r"(v3), 
                      "r"(v4), "r"(v5), "r"(v6), "r"(v7) : 
                      "memory");
    }
    
    /* Conditional use to extend live ranges */
    int result;
    if (seed & 1) {
        result = r0 + v1 + v3 + v5 + v7 + v9 + v11 + v13 + v15 + v17 + v19;
    } else {
        result = v2 + v4 + v6 + v8 + v10 + v12 + v14 + v16 + v18 + v20;
    }
    
    /* Final computation with side effect */
    return result + side_effect(barrier);
}

/* Alternate version with different register usage pattern */
static int __attribute__((noinline)) test_remat2(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed * 2, f = seed * 3, g = seed * 4, h = seed * 5;
    int i = seed ^ 1, j = seed ^ 2, k = seed ^ 3, l = seed ^ 4;
    int m = seed | 1, n = seed | 2, o = seed | 3, p = seed | 4;
    int q = seed & 0xFF, r = seed & 0xFE, s = seed & 0xFD, t = seed & 0xFC;
    
    volatile int sync = 0;
    
    for (int iter = 0; iter < 50; iter++) {
        /* Cross-dependent computations */
        a = b + c * d - e;
        b = c + d * e - f;
        c = d + e * f - g;
        d = e + f * g - h;
        e = f + g * h - i;
        f = g + h * i - j;
        g = h + i * j - k;
        h = i + j * k - l;
        i = j + k * l - m;
        j = k + l * m - n;
        k = l + m * n - o;
        l = m + n * o - p;
        m = n + o * p - q;
        n = o + p * q - r;
        o = p + q * r - s;
        p = q + r * s - t;
        q = r + s * t - a;
        r = s + t * a - b;
        s = t + a * b - c;
        t = a + b * c - d;
        
        sync = iter;
        
        /* Inline asm with multiple clobbers */
        asm volatile ("# dummy" : : 
                     "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
                     "r"(f), "r"(g), "r"(h), "r"(i), "r"(j) : 
                     "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
    }
    
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t + sync;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i);
        total += test_remat2(i * 3);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
