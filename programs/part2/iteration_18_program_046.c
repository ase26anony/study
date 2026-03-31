/* mcf_test.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes this function */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    /* Declare multiple variables to create register pressure */
    int a = seed * 2;
    int b = seed + 7;
    int c = seed - 3;
    int d = seed / 2;
    int e = seed % 11;
    int f = seed ^ 0x55;
    int g = seed | 0xAA;
    int h = seed & 0xF0;
    int i = seed << 2;
    int j = seed >> 1;
    
    /* Pointer arithmetic to create complex addressing */
    int *ptr = &a;
    int offset = 0;
    
    /* Loop with data-dependent computations */
    for (int n = 0; n < iterations; n++) {
        /* Chain of arithmetic operations creating data dependencies */
        a = b + c + n;
        b = c * d - a;
        c = d ^ e + b;
        d = e / (n + 1) + c;
        e = f % (n + 2) + d;
        f = g & h + e;
        g = h | i + f;
        h = i << (n % 4) + g;
        i = j >> (n % 3) + h;
        j = a + b + c + d + e + f + g + h + i + n;
        
        /* Pointer arithmetic with complex addressing */
        offset = (offset + 1) % 8;
        *(ptr + offset) = j;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Return a value based on all computations */
    return a + b + c + d + e + f + g + h + i + j;
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
    
    /* Use command-line argument to prevent constant propagation */
    int result = mcf_stress(iterations, argc);
    
    printf("Result: %d\n", result);
    return 0;
}
