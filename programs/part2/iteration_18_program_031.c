/* mcf_coverage.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure complex control flow */
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
    int h = seed % 17;
    int i = seed * seed;
    int j = ~seed;
    
    int result = 0;
    
    /* Complex loop with data dependencies */
    for (int n = 0; n < iterations; n++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f | i;
        j = h ^ n;
        b = j - c;
        c = b * 2;
        e = c / 3;
        g = e + f;
        i = g % 19;
        
        /* Mix with pointer arithmetic to create addressing modes */
        int *ptr = &a;
        ptr += (n & 3);  /* Prevent optimization */
        *ptr = n;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Accumulate result to prevent dead code elimination */
        result += a + b + c + d + e + f + g + h + i + j;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
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
