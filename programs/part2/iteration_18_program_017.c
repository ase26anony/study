/* mcf_coverage.c - Program to trigger MCF fixup graph special node dumping */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to maintain complex control flow */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    /* Declare multiple variables to pressure register allocation */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 19;
    int e = seed | 0xFF;
    int f = 0;
    int g = 1;
    int h = 2;
    int *ptr = &a;  /* Add pointer to create addressing modes */
    
    /* Complex loop with data dependencies */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        b = c * d;
        c = d ^ e;
        d = e - a;
        e = a | b;
        
        /* Mix with pointer arithmetic */
        f = *ptr + g;
        g = f * h;
        h = g & 0x3F;
        
        /* Update pointer to create varying addressing */
        ptr = (i % 2 == 0) ? &b : &c;
        
        /* Additional operations to increase live ranges */
        a = (a << 2) | (b >> 3);
        b = (b + c) * (d - 1);
        c = c ^ d ^ e;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Combine results to prevent elimination */
    return a + b + c + d + e + f + g + h;
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
    
    /* Use command line argument to prevent constant propagation */
    int result = mcf_stress(iterations, argc);
    
    printf("Result: %d\n", result);
    return 0;
}
