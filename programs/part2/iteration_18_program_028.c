/* test_mcf_coverage.c */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 5;
    int d = seed ^ 0x1234;
    int e = seed * 2;
    int f = seed | 0xABCD;
    int g = seed & 0x7F;
    int h = seed % 17;
    
    /* Complex loop with data dependencies to stress register allocation */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        b = c * d;
        c = d ^ e;
        d = e + f;
        e = f - g;
        f = g * h;
        g = h & a;
        h = a | b;
        
        /* Pointer arithmetic to create addressing modes */
        int *ptr = &a;
        ptr += (b & 0x3);  /* Prevent constant propagation */
        *ptr += c;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Combine results to prevent dead code elimination */
    return a + b + c + d + e + f + g + h;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    int result = mcf_stress(iterations, 42);
    printf("Result: %d\n", result);
    
    return 0;
}
