/* mcf_test.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Declare multiple variables to create register pressure */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 13;
    int d = seed ^ 0x55;
    int e = seed << 2;
    int f = seed >> 1;
    int g = seed | 0xFF;
    int h = seed & 0x0F;
    int i, j, k, l;
    int result = 0;
    
    /* Pointer arithmetic to create complex addressing */
    int *ptr = &a;
    
    /* Loop with data-dependent computations */
    for (i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations with data dependencies */
        j = a + b * c;
        k = (d - e) ^ f;
        l = (g & h) | (j * k);
        
        /* More complex computations using previous results */
        a = b + (c << 1);
        b = c ^ (d * 3);
        c = d + (e >> 2);
        d = e | (f & 0x7F);
        e = f - (g * h);
        f = g ^ (h + j);
        g = h * (k | 0x3);
        h = (l & 0xFF) + i;
        
        /* Pointer arithmetic with multiple operations */
        ptr = &a + (i & 0x3);
        *ptr = *ptr + b;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
        
        /* Accumulate result to prevent dead code elimination */
        result += a + b + c + d + e + f + g + h + j + k + l;
    }
    
    return result;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Use command-line argument to prevent constant propagation */
    int seed = iterations * 17;
    
    int result = mcf_stress(iterations, seed);
    
    printf("Result: %d\n", result);
    return 0;
}
