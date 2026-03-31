/* mcf_test.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 2;
    int d = seed * seed;
    int e = seed | 0xFF;
    int f = seed ^ 0xAA;
    int g = seed << 3;
    int h = seed >> 1;
    int i = seed % 17;
    int j = ~seed;
    
    int result = 0;
    
    /* Complex loop with data dependencies to stress register allocation */
    for (int n = 0; n < iterations; n++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        b = c - d;
        c = d * e;
        d = e ^ f;
        e = f | g;
        f = g & h;
        g = h << 2;
        h = i >> 1;
        i = j + a;
        j = a * b;
        
        /* Mix with pointer-like arithmetic */
        int* ptr = &a;
        int offset = b % 16;
        int val = *(ptr + offset % 4);
        
        /* More complex operations */
        result += (a * b) - (c / (d + 1)) + (e & f) | (g ^ h);
        result = (result << 3) | (result >> 29);  /* rotate */
        
        /* Memory barrier to prevent over-optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Final computation using all variables */
    return result + a + b + c + d + e + f + g + h + i + j;
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
