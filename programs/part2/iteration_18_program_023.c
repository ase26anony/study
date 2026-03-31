/* mcf_coverage.c - Program to trigger MCF fixup graph dumping with special nodes */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure complex control flow */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    /* Declare many variables to pressure register allocation */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 5;
    int d = seed ^ 0x1234;
    int e = seed * seed;
    int f = seed | 0xABCD;
    int g = seed << 3;
    int h = seed >> 2;
    int i = seed % 17;
    int j = seed + 1000;
    
    /* Complex loop with data dependencies */
    for (int n = 0; n < iterations; n++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f ^ i;
        j = h * 3;
        c = j - b;
        e = c | d;
        g = e * 7;
        i = g + h;
        b = i % 13;
        
        /* Pointer arithmetic to create complex addressing modes */
        int *ptr = &a;
        ptr += (n & 3);  /* Varying offset */
        *ptr = *ptr + n;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Combine results to prevent dead code elimination */
    return a + b + c + d + e + f + g + h + i + j;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Use command-line argument to prevent constant propagation */
    int result = mcf_stress(iterations, argc);
    
    printf("Result: %d\n", result);
    return 0;
}
