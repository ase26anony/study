/* test_mcf_coverage.c
 * Compile with: gcc -O2 -fdump-rtl-mcf -c test_mcf_coverage.c
 * This will generate dump files showing the fixup graph with special nodes.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Declare many variables to pressure register allocation */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x1234;
    int d = seed - 19;
    int e = seed | 0xABCD;
    int f = seed & 0xF0F0;
    int g = seed << 3;
    int h = seed >> 2;
    int i = seed % 31;
    int j = seed * seed;
    int k = 1;
    int l = 0;
    int m = -1;
    int n = 0xFFFFFFFF;
    int o = 0x7FFFFFFF;
    
    /* Pointer arithmetic to create complex addressing */
    int *ptr1 = &a;
    int *ptr2 = &b;
    int *ptr3 = &c;
    
    /* Loop with data-dependent computations */
    for (int idx = 0; idx < iterations; ++idx) {
        /* Chain of arithmetic operations creating data dependencies */
        a = b + c + idx;
        b = c ^ d ^ a;
        c = d * e + b;
        d = e & f & c;
        e = f | g | d;
        f = g << 2 ^ e;
        g = h >> 1 + f;
        h = i - j + g;
        i = j % 17 + h;
        j = k * l + i;
        k = l + m + j;
        l = m ^ n ^ k;
        m = n & o & l;
        n = o | seed | m;
        o = a * b + n;
        
        /* Pointer arithmetic with data dependencies */
        *ptr1 = *ptr2 + *ptr3 + idx;
        ptr1 = (int*)((char*)ptr1 + 1);
        ptr2 = (int*)((char*)ptr2 + 2);
        ptr3 = (int*)((char*)ptr3 + 3);
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Combine results to prevent dead code elimination */
    int result = a + b + c + d + e + f + g + h + i + j + 
                 k + l + m + n + o + (int)(ptr1) + (int)(ptr2) + (int)(ptr3);
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
    
    /* Use command-line argument to prevent constant propagation */
    int seed = iterations * 12345;
    
    int result = mcf_stress(iterations, seed);
    
    printf("Result: %d\n", result);
    
    /* Additional loop in main to create more CFG complexity */
    int sum = 0;
    for (int i = 0; i < iterations % 100; ++i) {
        sum += i * result;
        asm volatile("" : : : "memory");
    }
    
    return sum != 0 ? 0 : 1;
}
