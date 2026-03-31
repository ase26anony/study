/* mcf_coverage.c - Program to trigger MCF fixup graph special node dumping */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure complex control flow */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    /* Declare many variables to pressure register allocation */
    int a = seed * 2;
    int b = seed + 1;
    int c = seed ^ 0x55;
    int d = seed - 100;
    int e = seed | 0xFF;
    int f = seed & 0x0F;
    int g = seed << 2;
    int h = seed >> 1;
    int i = seed % 17;
    int j = seed * 3;
    
    /* Complex loop with data dependencies */
    for (int k = 0; k < iterations; k++) {
        /* Chain of arithmetic operations creating register pressure */
        a = b + c;
        b = c - d;
        c = d * e;
        d = e ^ f;
        e = f | g;
        f = g & h;
        g = h << 1;
        h = i >> 2;
        i = j % 13;
        j = a + k;
        
        /* Pointer arithmetic to create addressing modes */
        int *ptr = &a;
        ptr += (k & 3);  /* Complex addressing */
        *ptr += b;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Combine results to prevent dead code elimination */
    return a + b + c + d + e + f + g + h + i + j;
}

/* Another function to create interprocedural analysis opportunities */
__attribute__((noinline))
static int mcf_helper(int x, int y) {
    int result = 0;
    for (int i = 0; i < x; i++) {
        result += y * i;
        result ^= (y << i);
        asm volatile("" : : : "memory");
    }
    return result;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Dynamic loop bounds prevent optimization */
    int seed = iterations * 12345;
    
    /* Call stress function multiple times */
    int sum = 0;
    sum += mcf_stress(iterations, seed);
    sum += mcf_helper(iterations / 2, seed + 1);
    sum += mcf_stress(iterations / 3, seed + 2);
    
    printf("Result: %d\n", sum);
    return 0;
}
