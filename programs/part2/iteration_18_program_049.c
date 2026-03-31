/* mcf_test.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55AA;
    int d = seed >> 4;
    int e = seed * seed;
    int f = 1;
    int g = 0xFFFFFFFF;
    int h = 0;
    
    /* Complex loop with data dependencies to stress register allocation */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f ^ (i * 2);
        c = h - b;
        e = d | (a << 3);
        b = c ^ e;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Mix pointer arithmetic to create addressing complexity */
    int *ptr = &a;
    for (int i = 0; i < 5; i++) {
        *ptr += b;
        ptr = (int *)((char *)ptr + 1);
    }
    
    /* Return a value based on all computations */
    return a + b + c + d + e + f + h;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Use command-line argument to prevent constant propagation */
    int result = mcf_stress(iterations, iterations * 3);
    
    printf("Result: %d\n", result);
    return 0;
}
