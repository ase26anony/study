/* mcf_coverage.c - Test program to trigger MCF fixup graph dumping */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure MCF analyzes a proper function */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 11;
    int d = seed * 5;
    int e = seed / 2;
    int f = seed % 13;
    int g = seed ^ 0x55AA;
    int h = seed | 0xFF00;
    
    int *ptr_array[8];
    int local_array[16];
    
    /* Initialize arrays to create addressing complexity */
    for (int i = 0; i < 16; i++) {
        local_array[i] = seed + i;
        if (i < 8) ptr_array[i] = &local_array[i];
    }
    
    /* Complex loop with data dependencies and pointer arithmetic */
    for (int i = 0; i < iterations; i++) {
        /* Chain of integer operations creating register pressure */
        a = b + c;
        b = c * d;
        c = d ^ e;
        d = e + f;
        e = f - g;
        f = g & h;
        g = h | a;
        h = a * b;
        
        /* Pointer arithmetic with complex addressing */
        int idx = (a + b + c) % 16;
        int *ptr1 = &local_array[idx];
        int *ptr2 = ptr_array[idx % 8];
        
        /* Memory operations with dependencies */
        *ptr1 = *ptr2 + a;
        *ptr2 = *ptr1 - b;
        
        /* Conditional that depends on computations */
        if ((a + b + c) % 1000 == 0) {
            d = d * 2;
            e = e / 2;
        }
        
        /* Compiler memory barrier - prevents excessive optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Final computation using all variables */
    int result = (a + b + c + d + e + f + g + h) ^ 
                 (local_array[0] + local_array[15]);
    
    return result;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 1000;
    
    /* Use command-line argument to prevent constant propagation */
    int seed = iterations * 12345;
    
    int result = mcf_stress(iterations, seed);
    
    printf("MCF stress test result: %d\n", result);
    return 0;
}
