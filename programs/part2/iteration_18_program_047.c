/* mcf_test.c - Test program to trigger MCF fixup graph dump with special nodes */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes this function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Declare multiple variables to create register pressure */
    int a = seed * 2;
    int b = seed + 1;
    int c = seed ^ 0x5555;
    int d = seed >> 3;
    int e = seed * 3 + 7;
    int f = seed | 0xAAAA;
    int g = seed - 100;
    int h = seed % 17;
    int i = seed << 2;
    int j = seed & 0x3333;
    
    /* Pointer arithmetic to create complex addressing */
    int *ptr = &a;
    int sum = 0;
    
    /* Loop with data-dependent condition */
    for (int k = 0; k < iterations; k++) {
        /* Chain of arithmetic operations creating data dependencies */
        a = b + c;
        b = c - d;
        c = d * e;
        d = e ^ f;
        e = f | g;
        f = g & h;
        g = h + i;
        h = i - j;
        i = j * a;
        j = a ^ b;
        
        /* Pointer arithmetic with different offsets */
        sum += *(ptr + (k % 4));
        sum += ptr[k % 4];
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Mix all results to prevent dead code elimination */
    return sum + a + b + c + d + e + f + g + h + i + j;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        iterations = 1000;
    }
    
    /* Use command-line argument to prevent constant propagation */
    int result = mcf_stress(iterations, iterations);
    
    /* Ensure observable behavior */
    printf("Result: %d\n", result);
    
    return 0;
}
