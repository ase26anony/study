/* mcf_test.c - Test program to trigger MCF fixup graph dumping */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 5;
    int d = seed * 2;
    int e = seed / 3;
    int f = seed % 11;
    int g = seed ^ 0x55AA;
    int h = seed | 0xFF00;
    int result = 0;
    
    /* Complex loop with data dependencies to stress register allocation */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        b = c * d;
        c = d - e;
        d = e ^ f;
        e = f | g;
        f = g & h;
        g = h + a;
        h = a * b;
        
        /* Pointer arithmetic to create addressing modes */
        int *ptr = &a;
        int val = *(ptr + (i & 3));
        result += val;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Mix all variables to prevent dead code elimination */
    return result + a + b + c + d + e + f + g + h;
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
    
    int result = mcf_stress(iterations, 42);
    printf("Result: %d\n", result);
    
    return 0;
}
