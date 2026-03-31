/* Test program to trigger MCF fixup graph dumping with special nodes */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a real function */
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
    int result = 0;
    
    /* Complex loop with data dependencies */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        b = c * d;
        c = d ^ e;
        d = e | f;
        e = f & g;
        f = g + h;
        g = h * a;
        h = a - b;
        
        /* Mix integer and pointer-like arithmetic */
        int* ptr = (int*)(unsigned long)(a + b + c);
        int offset = (d + e + f) % 64;
        int val = (int)((unsigned long)ptr + offset);
        
        /* More complex computations */
        result += (a * b) - (c / (d ? d : 1)) + (e & f) | (g ^ h);
        result ^= val;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        iterations = 1000;
    }
    
    /* Use command-line argument to prevent constant propagation */
    int seed = iterations * 12345;
    
    int result = mcf_stress(iterations, seed);
    
    printf("Result: %d\n", result);
    return 0;
}
