/* mcf_coverage.c - Program to trigger MCF fixup graph dumping with special nodes */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure complex control flow */
__attribute__((noinline)) 
static int mcf_stress(int iterations, int seed) {
    /* Declare multiple variables to create register pressure */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 19;
    int e = seed | 0xFF;
    int f = seed & 0xAA;
    int g = seed << 2;
    int h = seed >> 1;
    int result = 0;
    
    /* Pointer arithmetic to create complex addressing */
    int *ptr = &a;
    int *ptr2 = &b;
    
    /* Loop with data-dependent condition */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating data dependencies */
        a = b + c + i;
        b = c * d - a;
        c = d ^ e ^ b;
        d = e | f | c;
        e = f & g & d;
        f = g << 1 ^ e;
        g = h >> 2 + f;
        h = a * b + g;
        
        /* Pointer arithmetic with data dependencies */
        *ptr = (*ptr2 + i) * 2;
        ptr2 = ptr;
        ptr = (ptr == &a) ? &b : &a;
        
        /* Complex addressing mode */
        result += *(ptr + (i & 1)) * (i + 1);
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Mix all results to prevent dead code elimination */
    return result + a + b + c + d + e + f + g + h;
}

/* Another function to create interprocedural analysis opportunities */
__attribute__((noinline))
static int helper_func(int x, int y) {
    int sum = 0;
    for (int i = 0; i < (x % 10) + 5; i++) {
        sum += y * i;
        y = (y * 13 + 7) & 0xFF;
        asm volatile("" : : : "memory");
    }
    return sum;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 100;
    
    /* Dynamic loop bounds prevent optimization */
    int seed = iterations * 17;
    
    /* Call stress function multiple times */
    int total = 0;
    for (int j = 0; j < 3; j++) {
        total += mcf_stress(iterations + j, seed + j);
        total += helper_func(iterations + j, seed - j);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
