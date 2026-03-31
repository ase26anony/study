/* mcf_test.c - Test program for GCC MCF pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 5;
    int d = seed * 2;
    int e = seed / 3;
    int f = seed % 11;
    int g = seed ^ 0xABCD;
    int h = seed | 0x1234;
    int result = 0;
    
    /* Complex loop with data dependencies to stress register allocation */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f | (d >> 3);
        c = h - a;
        e = c * b;
        g = e ^ f;
        b = g + h;
        
        /* Pointer arithmetic to create addressing modes */
        int *ptr = &a;
        *ptr += i;
        ptr = &b;
        *ptr -= c;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
        
        /* Accumulate result to prevent dead code elimination */
        result += a + b + c + d + e + f + g + h;
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
    int seed = iterations * 17;
    
    int result = mcf_stress(iterations, seed);
    
    printf("MCF stress test result: %d\n", result);
    return 0;
}
