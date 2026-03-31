/* test_mcf_coverage.c */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int k = 11, l = 12, m = 13, n = 14, o = 15;
    int p = 16, q = 17, r = 18, s = 19, t = 20;
    int result = 0;
    
    /* Complex loop with data dependencies to stress register allocation */
    for (int idx = 0; idx < iterations; ++idx) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f ^ i;
        j = h | k;
        l = j - m;
        n = l * o;
        p = n / (q + 1);
        r = p << 2;
        s = r >> 1;
        t = s % 19;
        
        /* Mix with pointer-like arithmetic */
        int *ptr1 = &a;
        int *ptr2 = &b;
        int offset = (*ptr1 + *ptr2) * idx;
        
        /* More complex dependencies */
        b = c + offset;
        c = d + *ptr1;
        e = f + t;
        g = h + idx;
        i = j * 3;
        k = l / 2;
        m = n + offset;
        o = p ^ idx;
        q = r | 0xFF;
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : : "memory");
        
        /* Accumulate result to prevent dead code elimination */
        result += a + b + c + d + e + f + g + h + i + j +
                  k + l + m + n + o + p + q + r + s + t;
    }
    
    return result;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        iterations = 100;
    }
    
    int result = mcf_stress(iterations);
    printf("Result: %d\n", result);
    
    return 0;
}
