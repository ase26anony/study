/* mcf_test.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 19;
    int e = seed | 0xFF;
    int f = 1;
    int g = seed * seed;
    int h = 0;
    
    /* Complex loop with data dependencies to stress register allocation */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f ^ (a << 2);
        c = h - b;
        e = d | (c * 3);
        g = f + (e >> 1);
        b = g ^ a;
        
        /* Memory barrier to prevent optimization and keep variables live */
        asm volatile("" : : : "memory");
    }
    
    /* Mix results to prevent dead code elimination */
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
