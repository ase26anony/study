/* test_mcf_coverage.c */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a real function */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 2;
    int d = seed * 5;
    int e = seed | 0xFF;
    int f = seed ^ 0xAA;
    int g = seed << 2;
    int h = seed >> 1;
    int result = 0;
    
    /* Complex loop with data dependencies to stress register allocation */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f ^ (i * 2);
        c = h - b;
        e = c | a;
        g = e * d;
        b = g >> 1;
        
        /* Mix in pointer-like arithmetic */
        int* ptr = &result;
        *ptr += (a + b + c + d + e + f + g + h) & 0xFF;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        iterations = 1000;
    }
    
    /* Use command-line argument to prevent constant propagation */
    int result = mcf_stress(iterations, argc);
    
    printf("Result: %d\n", result);
    return 0;
}
