/* mcf_coverage.c - Program to trigger MCF fixup graph special node dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure complex control flow */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    /* Declare many variables to pressure register allocation */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 5;
    int d = seed * 2;
    int e = seed / 3;
    int f = seed % 11;
    int g = seed ^ 0x55;
    int h = seed | 0xAA;
    int i = seed << 2;
    int j = seed >> 1;
    int k = 0;
    
    /* Complex loop with data dependencies */
    for (int n = 0; n < iterations; ++n) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        b = c - d;
        c = d * e;
        d = e / (f + 1);
        e = f ^ g;
        f = g | h;
        g = h & i;
        h = i << 1;
        i = j >> 1;
        j = a + k;
        
        /* Pointer arithmetic to create complex addressing */
        int *ptr = &a;
        ptr += (n & 3);  /* Prevent optimization */
        k += *ptr;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Combine all results to prevent dead code elimination */
    return a + b + c + d + e + f + g + h + i + j + k;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Use command line argument to prevent constant propagation */
    int result = mcf_stress(iterations, argc);
    
    printf("Result: %d\n", result);
    return 0;
}
