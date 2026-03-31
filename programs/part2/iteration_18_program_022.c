/* mcf_coverage.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Declare multiple variables to create register pressure */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 5;
    int d = seed ^ 0xFF;
    int e = seed << 2;
    int f = seed >> 1;
    int g = seed | 0xAA;
    int h = seed & 0x55;
    int i = seed % 17;
    int j = seed * seed;
    
    /* Pointer arithmetic to create complex addressing */
    int *ptr = &a;
    int offset = 0;
    
    /* Loop with data-dependent computations */
    for (int n = 0; n < iterations; ++n) {
        /* Chain of arithmetic operations creating data dependencies */
        a = b + c + n;
        b = c * d - a;
        c = d ^ e ^ b;
        d = e << (n % 4);
        e = f >> (a & 3);
        f = g | h | e;
        g = h & i & f;
        h = i * j + g;
        i = j - a + h;
        j = a * b * i;
        
        /* Pointer arithmetic with varying offset */
        offset = (offset + 1) % 4;
        *(ptr + offset) = j;
        
        /* Complex addressing mode */
        int temp = *(ptr + (n & 3));
        temp = temp * 3 + 1;
        *(ptr + ((n + 1) & 3)) = temp;
    }
    
    /* Memory barrier to prevent excessive optimization */
    asm volatile ("" : : : "memory");
    
    /* Return a combination of all variables */
    return a + b + c + d + e + f + g + h + i + j + *ptr;
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
