/* mcf_test.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Declare multiple variables to create register pressure */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 2;
    int d = seed ^ 0x55;
    int e = seed | 0xAA;
    int f = 0;
    int g = 0;
    int h = 0;
    
    /* Pointer arithmetic to create complex addressing */
    int *ptr1 = &a;
    int *ptr2 = &b;
    
    /* Loop with data-dependent computations */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating data dependencies */
        a = b + c;
        b = c * d;
        c = d ^ e;
        d = e + a;
        e = a * b;
        
        /* Mix with pointer arithmetic */
        f = *ptr1 + *ptr2;
        g = (*ptr1) * (*ptr2);
        h = (*ptr1) ^ (*ptr2);
        
        /* Rotate pointers to prevent optimization */
        if (i % 3 == 0) {
            ptr1 = &a;
            ptr2 = &b;
        } else if (i % 3 == 1) {
            ptr1 = &c;
            ptr2 = &d;
        } else {
            ptr1 = &e;
            ptr2 = &f;
        }
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Combine results to prevent dead code elimination */
    return a + b + c + d + e + f + g + h;
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
