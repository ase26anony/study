/* mcf_test.c - Test program for GCC MCF pass graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure complex control flow */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 19;
    int e = seed | 0xFF;
    int f = seed << 2;
    int g = seed >> 1;
    int h = seed % 31;
    int result = 0;
    
    /* Complex loop with data dependencies to pressure register allocation */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        b = c - d;
        c = d * e;
        d = e ^ f;
        e = f | g;
        f = g & h;
        g = h + a;
        h = a - b;
        
        /* Pointer arithmetic to create complex addressing modes */
        int *ptr = &result;
        *ptr += (a + b + c + d + e + f + g + h);
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Additional computation to prevent tail optimization */
    result = (result * 1103515245 + 12345) & 0x7fffffff;
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
    int result = mcf_stress(iterations, argc);
    
    printf("Result: %d\n", result);
    return 0;
}
