/* mcf_test.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure complex control flow */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Declare multiple variables to pressure register allocation */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 5;
    int d = seed * 2;
    int e = seed / 3;
    int f = seed % 11;
    int g = seed ^ 0x55;
    int h = seed | 0xAA;
    int result = 0;
    
    /* Complex loop with data dependencies */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        d = a * e;
        f = d & g;
        h = f | (i * 2);
        c = h - b;
        e = c ^ a;
        g = e * d;
        b = g % (f + 1);
        
        /* Mix of integer and pointer-like arithmetic */
        int* ptr = (int*)(unsigned long)(a + b + c);
        int offset = (d + e + f) % 64;
        int val = (int)((unsigned long)ptr + offset);
        
        /* More operations to create complex live ranges */
        result += val & 0xFF;
        result ^= (a << 3);
        result |= (b >> 2);
        result *= (c + 1);
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Final computation to prevent dead code elimination */
    return result + a + b + c + d + e + f + g + h;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Use command-line argument to prevent constant propagation */
    int seed = iterations * 12345;
    
    int result = mcf_stress(iterations, seed);
    
    printf("Result: %d\n", result);
    return 0;
}
