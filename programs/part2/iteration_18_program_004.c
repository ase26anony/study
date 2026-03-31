/* mcf_test.c - Test program for Maximum Flow (MCF) pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Declare multiple variables to pressure register allocation */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 5;
    int d = seed ^ 0x1234;
    int e = seed | 0xABCD;
    int f = seed & 0x5678;
    int g = seed << 2;
    int h = seed >> 1;
    int i = seed % 17;
    int j = seed * seed;
    
    /* Pointer arithmetic to create complex addressing */
    int *ptr = &a;
    int offset = 0;
    
    /* Loop with data-dependent computations */
    for (int n = 0; n < iterations; n++) {
        /* Chain of arithmetic operations creating data dependencies */
        a = b + c + n;
        b = c * d - a;
        c = d ^ e ^ b;
        d = e | f | c;
        e = f & g & d;
        f = g << (n % 4);
        g = h >> ((n + 1) % 4);
        h = i * j + g;
        i = j - a + h;
        j = (a * b) / (c + 1);
        
        /* More complex operations mixing variables */
        offset = (offset + n) % 16;
        *(ptr + offset) = a + b + c;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Return a combination of all variables */
    return a + b + c + d + e + f + g + h + i + j + *ptr;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Use command-line argument to prevent constant propagation */
    int seed = iterations * 37;
    
    int result = mcf_stress(iterations, seed);
    
    printf("Result: %d\n", result);
    return 0;
}
