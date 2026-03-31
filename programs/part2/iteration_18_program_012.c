/* mcf_test.c - Test program to trigger MCF pass graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes this function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Declare many variables to create register pressure */
    int a = seed * 2;
    int b = seed + 1;
    int c = seed ^ 0x55;
    int d = seed >> 3;
    int e = seed * 3 + 7;
    int f = seed | 0xFF;
    int g = seed - 100;
    int h = seed % 17;
    int i = seed * seed;
    int j = ~seed;
    
    /* Pointer arithmetic to create complex addressing */
    int *ptr1 = &a;
    int *ptr2 = &b;
    
    /* Loop with data-dependent computations */
    for (int n = 0; n < iterations; n++) {
        /* Chain of arithmetic operations creating data dependencies */
        a = b + c + n;
        b = c * d - a;
        c = d ^ e ^ b;
        d = e + f + c;
        e = f & g & d;
        f = g | h | e;
        g = h * i * f;
        h = i - j - g;
        i = j + a + h;
        j = a * b * i;
        
        /* Pointer arithmetic with mixing */
        *ptr1 = *ptr1 + *ptr2 + n;
        *ptr2 = *ptr2 - *ptr1;
        
        /* Complex addressing mode */
        int temp = *(ptr1 + (n & 1)) + *(ptr2 + ((n >> 1) & 1));
        
        /* Use all variables to prevent elimination */
        j = j + temp;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Combine results to return something meaningful */
    return a + b + c + d + e + f + g + h + i + j;
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
    int seed = iterations * 12345;
    
    int result = mcf_stress(iterations, seed);
    
    printf("Result: %d\n", result);
    return 0;
}
