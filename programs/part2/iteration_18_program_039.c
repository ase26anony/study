/* mcf_coverage.c - Program to trigger MCF fixup graph special node dumping */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Create multiple variables with data dependencies */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 19;
    int e = seed | 0xFF;
    int f = seed << 2;
    int g = seed >> 1;
    int h = seed % 31;
    
    int result = 0;
    
    /* Loop with data-dependent computations */
    for (int i = 0; i < iterations; ++i) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f ^ c;
        b = h - a;
        c = d | f;
        e = b * h;
        g = c + e;
        
        /* Mix in pointer-like arithmetic */
        int* ptr = &a;
        int offset = (b + i) % 16;
        int val = *(ptr + offset % 4);
        
        /* More computations using the loaded value */
        a = val + g;
        d = a ^ h;
        
        /* Memory barrier to prevent optimization */
        asm volatile ("" : : : "memory");
        
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
    if (iterations <= 0) {
        iterations = 1000;
    }
    
    /* Use command-line argument to prevent constant propagation */
    int result = mcf_stress(iterations, argc);
    
    printf("Result: %d\n", result);
    return 0;
}
