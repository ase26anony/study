/* mcf_coverage.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    /* Declare many variables to pressure register allocation */
    int a = seed * 2;
    int b = seed + 1;
    int c = seed ^ 0x55;
    int d = seed << 3;
    int e = seed >> 1;
    int f = seed | 0xFF;
    int g = seed & 0x7F;
    int h = seed % 17;
    int i = seed * 3;
    int j = seed - 5;
    
    /* Complex loop with data dependencies */
    for (int n = 0; n < iterations; n++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f ^ i;
        j = h << 2;
        b = j - a;
        c = b | d;
        e = c ^ f;
        g = e * h;
        i = g >> 1;
        
        /* Pointer arithmetic to create addressing modes */
        int *ptr = &a;
        ptr += (n & 3);  /* Prevent optimization */
        *ptr += n;
        
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
