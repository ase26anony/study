/* test_mcf_coverage.c - Program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Force no inlining to create a distinct compilation unit for MCF analysis */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 5;
    int d = seed ^ 0xFF;
    int e = seed << 2;
    int f = seed >> 1;
    int g = seed | 0xAA;
    int h = seed & 0x55;
    int result = 0;
    
    /* Complex loop with data dependencies to stress register allocation */
    for (int i = 0; i < iterations; ++i) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        b = c * d;
        c = d ^ e;
        d = e + f;
        e = f - g;
        f = g & h;
        g = h | a;
        h = a * b;
        
        /* Mix integer and pointer-like arithmetic */
        int* ptr = (int*)((long)&result + (i & 0x3));
        *ptr += a + b + c;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Combine all variables to prevent dead code elimination */
    result += a + b + c + d + e + f + g + h;
    return result;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Use command-line argument to make loop bound non-constant */
    int result = mcf_stress(iterations, 42);
    
    printf("Result: %d\n", result);
    return 0;
}
