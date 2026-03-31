/* Test program to generate UNORDERED, ORDERED, and other FP condition codes */
#include <stdio.h>
#include <math.h>

/* Prevent inlining to keep RTL structure intact */
__attribute__((noinline))
static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    
    /* Create multiple basic blocks with different FP comparisons */
    for (int i = 0; i < 10; i++) {
        /* UNORDERED checks (should generate UNORDERED/UNEQ condition codes) */
        if (__builtin_isunordered(a, b)) {
            counter++;
            asm volatile("" : : : "memory"); /* Barrier */
        }
        
        /* ORDERED check */
        if (!__builtin_isunordered(a, c)) {
            counter += 2;
        }
        
        /* Direct NaN check (unordered) */
        if (a != a) {  /* NaN != NaN is true */
            counter += 3;
        }
        
        /* Standard comparisons (generate LT, GT, EQ condition codes) */
        if (b < c) {
            counter += 4;
        }
        
        if (b > a) {
            counter += 5;
        }
        
        /* Equality with NaN operand (unordered) */
        if (__builtin_isnan(a)) {
            counter += 6;
        }
        
        /* Mixed ordered/unordered comparisons */
        if (b == c) {
            counter += 7;
        }
        
        /* UNGE/UNGT/UNLE/UNLT via explicit checks */
        if (!(b < c)) {  /* UNGE: not less than */
            counter += 8;
        }
        
        if (!(b <= c)) { /* UNGT: not less or equal */
            counter += 9;
        }
        
        /* LTGT (unordered not equal) */
        if (b != c) {
            counter += 10;
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+g"(i) : : "memory");
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN and normal values */
    volatile double nan_val = 0.0 / 0.0;  /* Quiet NaN */
    volatile double inf_val = 1.0 / 0.0;  /* Infinity */
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    
    /* Force values to be used unpredictably */
    asm volatile("" : "+g"(nan_val), "+g"(inf_val), 
                       "+g"(normal1), "+g"(normal2));
    
    /* Call test function multiple times with different combinations */
    int total = 0;
    total += fp_test(nan_val, normal1, normal2);
    total += fp_test(inf_val, nan_val, normal1);
    total += fp_test(normal1, normal2, nan_val);
    total += fp_test(normal2, inf_val, normal1);
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", total);
    
    return 0;
}
