/* mcf_coverage.c - Program to trigger MCF fixup graph dumping with special nodes */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure complex control flow */
__attribute__((noinline))
static int mcf_stress(int iterations, int seed) {
    /* Declare many variables to create register pressure */
    int a = seed * 2;
    int b = seed + 1;
    int c = seed ^ 0x55;
    int d = seed << 3;
    int e = seed >> 1;
    int f = seed | 0xFF;
    int g = seed & 0x7F;
    int h = seed % 17;
    int i = seed * 3;
    int j = seed + 11;
    
    /* Pointer arithmetic to create complex addressing */
    int *ptr1 = &a;
    int *ptr2 = &b;
    int *ptr3 = &c;
    
    /* Loop with data-dependent computations */
    for (int count = 0; count < iterations; count++) {
        /* Chain of arithmetic operations creating data dependencies */
        a = b + c + count;
        b = c * d - a;
        c = d ^ e ^ count;
        d = e + f + a;
        e = f & g & b;
        f = g | h | c;
        g = h * i + d;
        h = i ^ j ^ e;
        i = j + a + f;
        j = b * c + g;
        
        /* Pointer arithmetic with the variables */
        *ptr1 = (*ptr2 + *ptr3) * count;
        ptr1 = (ptr1 == &a) ? &b : &a;
        ptr2 = (ptr2 == &b) ? &c : &b;
        ptr3 = (ptr3 == &c) ? &a : &c;
        
        /* Memory barrier to prevent excessive optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Complex final computation using all variables */
    int result = (a + b) * (c - d) + (e & f) | (g ^ h) + (i * j);
    
    /* More pointer arithmetic */
    *ptr1 = result;
    result += *ptr2 - *ptr3;
    
    return result;
}

/* Another function to create more interprocedural complexity */
__attribute__((noinline))
static int process_result(int value) {
    int temp = value;
    for (int i = 0; i < 10; i++) {
        temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
        asm volatile("" : : : "memory");
    }
    return temp % 1000;
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
    
    /* Use command-line argument to prevent constant propagation */
    int seed = iterations * 17;
    
    /* Call the stress function multiple times */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        int result = mcf_stress(iterations + i, seed + i);
        result = process_result(result);
        total += result;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    return 0;
}
