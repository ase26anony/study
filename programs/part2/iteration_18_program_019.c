/* mcf_coverage.c - Program to trigger MCF fixup graph dumping with special nodes */
#include <stdio.h>
#include <stdlib.h>

/* Force function to not be inlined for more complex CFG */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 19;
    int e = seed | 0xFF;
    int f = seed << 2;
    int g = seed >> 1;
    int h = seed % 17;
    int result = 0;
    
    /* Complex loop with data dependencies to stress register allocation */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        b = c - d;
        c = d * e;
        d = e ^ f;
        e = f | g;
        f = g & h;
        g = h + a;
        h = a - b;
        
        /* More complex operations with mixed types */
        int* ptr = &result;
        *ptr += (a * b) + (c / (d != 0 ? d : 1)) - (e & f) | (g ^ h);
        
        /* Array access with pointer arithmetic */
        int arr[8] = {a, b, c, d, e, f, g, h};
        for (int j = 0; j < 8; j++) {
            result += arr[j] * (j + 1);
        }
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
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
    
    /* Use command-line argument to prevent constant propagation */
    int seed = iterations * 37;
    
    int result = mcf_stress(iterations, seed);
    
    printf("Result: %d\n", result);
    return 0;
}
