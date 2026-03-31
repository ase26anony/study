/* test_mcf_coverage.c */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Declare multiple variables to create register pressure */
    int a = seed * 2;
    int b = seed + 7;
    int c = seed - 3;
    int d = seed | 0xFF;
    int e = seed ^ 0x55;
    int f = seed << 2;
    int g = seed >> 1;
    int h = seed % 17;
    int i = 0;
    int j = 0;
    int k = 0;
    int result = 0;
    
    /* Pointer arithmetic to create complex addressing */
    int *ptr1 = &a;
    int *ptr2 = &b;
    int *ptr3 = &c;
    
    /* Loop with data-dependent computations */
    for (int n = 0; n < iterations; n++) {
        /* Chain of arithmetic operations creating data dependencies */
        a = b + c + n;
        b = c * d - n;
        c = d ^ e ^ a;
        d = e & f | b;
        e = f + g + c;
        f = g - h + d;
        g = h * a + e;
        h = (a + b) % (c + 1);
        
        /* More complex computations mixing variables */
        i = (a << 2) | (b >> 3);
        j = (c * d) + (e * f);
        k = (g ^ h) & (i | j);
        
        /* Pointer arithmetic with updates */
        *ptr1 = *ptr1 + *ptr2;
        *ptr2 = *ptr2 + *ptr3;
        ptr3 = (int*)((char*)ptr3 + (n % 4));
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
        
        /* Accumulate result to prevent dead code elimination */
        result += a + b + c + d + e + f + g + h + i + j + k;
    }
    
    return result;
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
    
    /* Use command-line argument to make loop bound non-constant */
    int result = mcf_stress(iterations, 42);
    
    printf("Result: %d\n", result);
    return 0;
}
