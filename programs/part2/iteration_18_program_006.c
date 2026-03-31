/* mcf_coverage.c - Program to trigger MCF fixup graph special node dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    /* Declare many variables to pressure register allocation */
    int a = seed * 2;
    int b = seed + 5;
    int c = seed - 3;
    int d = seed | 0xFF;
    int e = seed ^ 0xAA;
    int f = seed << 2;
    int g = seed >> 1;
    int h = seed % 7;
    int i = seed * 3;
    int j = seed + 11;
    
    int result = 0;
    
    /* Loop with data-dependent computations */
    for (int k = 0; k < iterations; k++) {
        /* Create complex web of data dependencies */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f | i;
        j = h ^ k;
        b = j - a;
        c = d + f;
        e = g * h;
        i = j % (k + 1);
        g = (a << 2) | (b >> 1);
        
        /* Mix pointer arithmetic to create addressing modes */
        int *ptr = &a;
        ptr += (k & 3);  /* Simple pointer arithmetic */
        *ptr += k;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
        
        /* Accumulate result to prevent dead code elimination */
        result += a + b + c + d + e + f + g + h + i + j;
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
    int seed = iterations * 3;
    
    int result = mcf_stress(iterations, seed);
    
    printf("Result: %d\n", result);
    return 0;
}
