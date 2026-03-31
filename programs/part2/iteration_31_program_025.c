/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386 backend coverage testing.
 */

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Helper function with floating-point comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    
    /* Loop to create multiple RTL instructions */
    for (volatile int i = 0; i < 5; i++) {
        /* 1. UNORDERED checks */
        if (__builtin_isunordered(a, b)) {
            counter++;  /* Side effect to keep branch alive */
        }
        
        /* 2. ORDERED checks */
        if (!__builtin_isunordered(a, c)) {  /* Equivalent to ORDERED */
            counter++;
        }
        
        /* 3. Direct NaN checks that may generate UNORDERED */
        if (a != a) {  /* NaN check */
            counter++;
        }
        
        /* 4. UNEQ (unordered or equal) */
        if (__builtin_isunordered(a, b) || a == b) {
            counter++;
        }
        
        /* 5. UNGE (unordered or greater-or-equal) */
        if (__builtin_isunordered(a, b) || a >= b) {
            counter++;
        }
        
        /* 6. UNGT (unordered or greater-than) */
        if (__builtin_isunordered(a, b) || a > b) {
            counter++;
        }
        
        /* 7. UNLE (unordered or less-or-equal) */
        if (__builtin_isunordered(a, b) || a <= b) {
            counter++;
        }
        
        /* 8. UNLT (unordered or less-than) */
        if (__builtin_isunordered(a, b) || a < b) {
            counter++;
        }
        
        /* 9. LTGT (less-than or greater-than, but not equal/unordered) */
        if ((a < b) || (a > b)) {
            counter++;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values to create data dependencies */
        asm volatile("" : "+g"(a), "+g"(b));
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN and normal values */
    volatile double nan_val = 0.0 / 0.0;      /* Generate a NaN */
    volatile double normal1 = 1.5;
    volatile double normal2 = 2.7;
    
    /* Call test function multiple times */
    int total = 0;
    total += fp_test(nan_val, normal1, normal2);
    total += fp_test(normal1, nan_val, normal2);
    total += fp_test(normal1, normal2, nan_val);
    
    /* Use result to prevent dead code elimination */
    volatile int result = total;
    
    /* Simple output to prevent optimization */
    if (result > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Should never happen */
    }
}
