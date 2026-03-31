/* mcf_test.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 2;
    int d = seed * 5;
    int e = seed / 3;
    int f = seed % 11;
    int g = seed ^ 0x55;
    int h = seed | 0xAA;
    int result = 0;
    
    /* Complex loop with data dependencies to pressure register allocation */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f | c;
        e = h ^ b;
        g = e - a;
        c = g * d;
        b = c / (f + 1);
        
        /* Mix with pointer-like arithmetic */
        int* ptr = (int*)&a;
        *ptr = *ptr + i;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
        
        /* Accumulate result to prevent dead code elimination */
        result += a + b + c + d + e + f + g + h;
    }
    
    return result;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Use command-line argument to prevent constant propagation */
    int seed = iterations * 17;
    
    int result = mcf_stress(iterations, seed);
    
    printf("MCF stress test result: %d\n", result);
    return 0;
}
