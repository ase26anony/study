/* Test program to generate floating-point condition codes for i386 RTL coverage */
#include <stdio.h>
#include <math.h>

/* Prevent inlining to ensure separate function RTL generation */
__attribute__((noinline)) 
static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    
    /* Loop to create multiple RTL instructions */
    for (int i = 0; i < 10; i++) {
        /* UNORDERED condition: check if either operand is NaN */
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* ORDERED condition: check if both are NOT NaN */
        if (!__builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* UNEQ condition: unordered or equal */
        if (__builtin_isunordered(a, b) || a == b) {
            counter++;
        }
        
        /* UNGE condition: unordered or greater-or-equal */
        if (__builtin_isunordered(a, b) || a >= b) {
            counter++;
        }
        
        /* UNGT condition: unordered or greater-than */
        if (__builtin_isunordered(a, b) || a > b) {
            counter++;
        }
        
        /* UNLE condition: unordered or less-or-equal */
        if (__builtin_isunordered(a, b) || a <= b) {
            counter++;
        }
        
        /* UNLT condition: unordered or less-than */
        if (__builtin_isunordered(a, b) || a < b) {
            counter++;
        }
        
        /* LTGT condition: less-than or greater-than (but not equal, not unordered) */
        if ((a < b) || (a > b)) {
            counter++;
        }
        
        /* Standard comparisons that might generate different condition codes */
        if (a == b) counter++;
        if (a != b) counter++;
        if (a < b) counter++;
        if (a > b) counter++;
        if (a <= b) counter++;
        if (a >= b) counter++;
        
        /* Check for NaN explicitly */
        if (__builtin_isnan(a)) counter++;
        if (__builtin_isnan(b)) counter++;
        
        /* Use inline assembly to create data dependencies */
        asm volatile("" : "+g"(a), "+g"(b) : : "memory");
        
        /* Mix in the third variable */
        if (__builtin_isunordered(b, c)) counter++;
        if (!__builtin_isunordered(c, a)) counter++;
    }
    
    return counter;
}

int main(void) {
    /* Initialize volatile doubles with NaN and normal values */
    volatile double nan_val = 0.0 / 0.0;  /* Generate a quiet NaN */
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    volatile double inf_val = 1.0 / zero;  /* Infinity */
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    result += fp_test(nan_val, normal1, normal2);
    result += fp_test(normal1, nan_val, inf_val);
    result += fp_test(normal1, normal2, nan_val);
    result += fp_test(inf_val, inf_val, normal1);
    result += fp_test(zero, nan_val, inf_val);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
