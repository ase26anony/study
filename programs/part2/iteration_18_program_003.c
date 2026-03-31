/* mcf_test.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 2;
    int d = seed | 0xFF;
    int e = seed ^ 0xAA;
    int f = seed << 3;
    int g = seed >> 1;
    int h = 0;
    int *ptr = &h;
    
    /* Complex loop with data dependencies and pointer arithmetic */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        b = c * d;
        c = d ^ e;
        d = e | f;
        e = f & g;
        f = g + a;
        g = a - b;
        
        /* Pointer arithmetic to create addressing modes */
        *ptr = (*ptr + a) & 0xFFFF;
        ptr = &a + (i & 3);  /* Varying pointer base */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Return a value using all variables to prevent elimination */
    return a + b + c + d + e + f + g + h;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Use different seeds to create varying execution paths */
    int result = 0;
    for (int s = 0; s < 3; s++) {
        result += mcf_stress(iterations, s * 17 + 123);
    }
    
    printf("Result: %d\n", result);
    return 0;
}
