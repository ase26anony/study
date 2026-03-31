/* test_mcf_coverage.c */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 5;
    int d = seed ^ 0x1234;
    int e = seed | 0xABCD;
    int f = 1;
    int g = 0;
    int h = 0;
    
    /* Complex loop with data dependencies to stress register allocation */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        g = f ^ h;
        h = g | a;
        c = h - b;
        b = c * 2;
        e = b / 3;
        
        /* Memory barrier to prevent optimization and keep variables live */
        asm volatile ("" : : : "memory");
        
        /* Additional pointer arithmetic to create complex addressing */
        int* ptr = &a;
        *ptr += i;
        ptr = &b;
        *ptr ^= i;
    }
    
    /* Return a value based on all computations to prevent elimination */
    return a + b + c + d + e + f + g + h;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    int result = mcf_stress(iterations, 42);
    printf("Result: %d\n", result);
    
    return 0;
}
