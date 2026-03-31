/* mcf_coverage.c - Program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Declare many variables to pressure register allocation */
    int a = seed * 2;
    int b = seed + 1;
    int c = seed ^ 0x55AA;
    int d = seed >> 3;
    int e = seed << 2;
    int f = seed % 17;
    int g = seed | 0xFF00;
    int h = seed & 0x00FF;
    int i = seed - 100;
    int j = seed + 200;
    
    /* Pointer arithmetic to create complex addressing */
    int *ptr = &a;
    int offset = 0;
    
    /* Loop with data-dependent computations */
    for (int n = 0; n < iterations; n++) {
        /* Chain of arithmetic operations creating data dependencies */
        a = b + c;
        b = c - d;
        c = d * e;
        d = e / (f + 1);  /* Avoid division by zero */
        e = f ^ g;
        f = g & h;
        g = h | i;
        h = i << 2;
        i = j >> 1;
        j = a + n;
        
        /* Pointer arithmetic with multiple variables */
        offset = (a + b + c) % 8;
        *(ptr + offset) = d + e + f;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Final computation using all variables */
    int result = a + b + c + d + e + f + g + h + i + j;
    return result;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        iterations = 1000;
    }
    
    /* Use command-line argument to prevent constant propagation */
    int seed = iterations;
    
    /* Call the stress function */
    int result = mcf_stress(iterations, seed);
    
    /* Print result to prevent dead code elimination */
    printf("MCF stress result: %d\n", result);
    
    return 0;
}
