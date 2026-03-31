/* mcf_coverage.c - Program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    /* Declare many variables to pressure register allocation */
    int a = seed * 2;
    int b = seed + 1;
    int c = seed ^ 0x5555;
    int d = seed >> 3;
    int e = seed * 3 + 1;
    int f = 0;
    int g = 0x1234;
    int h = 0;
    int *ptr = &h;
    
    /* Complex loop with data dependencies and pointer arithmetic */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f | (i << 2);
        
        /* Pointer arithmetic to create addressing modes */
        *(ptr + (i & 1)) = h;
        
        /* More operations to extend live ranges */
        b = c ^ d;
        c = d + f;
        e = g - a;
        g = h * 2;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Return a value based on all computations */
    return a + b + c + d + e + f + g + h;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Call stress function with dynamic input */
    int result = mcf_stress(iterations, 42);
    
    printf("Result: %d\n", result);
    return 0;
}
