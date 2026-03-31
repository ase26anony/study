/* mcf_coverage.c - Program to trigger MCF fixup graph dumping */
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
        d = a * e;
        f = d & g;
        h = f | c;
        e = h ^ a;
        b = e - d;
        c = b * 2;
        g = c % 17;
        
        /* Mix of integer and pointer-like arithmetic */
        int* ptr = (int*)&a;
        int offset = (i * 4) % 32;
        int val = *(ptr) + offset;
        
        /* More operations to increase live ranges */
        result += val + a + b + c + d + e + f + g + h;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Dynamic seed to prevent constant propagation */
    int seed = iterations * 12345;
    
    /* Call the stress function */
    int result = mcf_stress(iterations, seed);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
