/* mcf_coverage.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Declare many variables to pressure register allocation */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 19;
    int e = seed | 0xFF;
    int f = seed << 2;
    int g = seed >> 1;
    int h = seed % 17;
    int i = seed * seed;
    int j = ~seed;
    
    /* Complex pointer arithmetic to create addressing modes */
    int *ptr1 = &a;
    int *ptr2 = &b;
    int *ptr3 = &c;
    
    /* Loop with data-dependent computations */
    for (int n = 0; n < iterations; n++) {
        /* Chain of arithmetic operations creating data dependencies */
        a = b + c + n;
        b = c * d - a;
        c = d ^ e ^ b;
        d = e & f | c;
        e = f + g + d;
        f = g - h + e;
        g = h * i ^ f;
        h = i / (n + 1) + g;
        i = j << (n % 4) + h;
        j = a + b + c + d + e + f + g + h + i + n;
        
        /* Pointer arithmetic with complex addressing */
        *(ptr1 + (n % 2)) = a + b;
        *(ptr2 + (n % 3)) = c + d;
        ptr3 = ptr1 + (n % 4);
        *ptr3 = e + f;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Return a complex result to prevent dead code elimination */
    return a + b + c + d + e + f + g + h + i + j + *ptr1 + *ptr2 + *ptr3;
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
