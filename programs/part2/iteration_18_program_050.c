/* mcf_test.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Force this function to not be inlined to create more complex CFG */
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
    int i = seed * 3;
    int j = seed + 11;
    
    int result = 0;
    
    /* Loop with data-dependent computations */
    for (int n = 0; n < iterations; n++) {
        /* Create a web of data dependencies */
        a = b + c;
        b = c - d;
        c = d * e;
        d = e & f;
        e = f | g;
        f = g ^ h;
        g = h + i;
        h = i - j;
        i = j * a;
        j = a + b;
        
        /* Mix in pointer arithmetic to create complex addressing */
        int *ptr = &a;
        int val = *(ptr + (n & 3));
        result += val;
        
        /* Memory barrier to prevent over-optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Final computation to use all variables */
    result = (a + b + c + d + e + f + g + h + i + j) % 1000;
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
    
    /* Use command-line argument to prevent constant propagation */
    int seed = iterations * 3;
    
    int result = mcf_stress(iterations, seed);
    
    printf("Result: %d\n", result);
    return 0;
}
