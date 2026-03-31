/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to ensure separate function RTL */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN */
static double make_nan(void) {
    return 0.0 / 0.0;
}

/* Helper to create infinity */
static double make_inf(void) {
    return 1.0 / 0.0;
}

/* Core function with FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks - using various methods */
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        if (a != a) {  /* Classic NaN check - should generate unordered */
            counter++;
        }
        
        /* ORDERED checks */
        if (!__builtin_isunordered(a, c)) {
            counter++;
        }
        
        /* UNEQ (unordered or equal) */
        if (__builtin_isunordered(a, d) || a == d) {
            counter++;
        }
        
        /* UNGE (unordered or greater than or equal) */
        if (__builtin_isunordered(b, c) || b >= c) {
            counter++;
        }
        
        /* UNGT (unordered or greater than) */
        if (__builtin_isunordered(b, d) || b > d) {
            counter++;
        }
        
        /* UNLE (unordered or less than or equal) */
        if (__builtin_isunordered(c, a) || c <= a) {
            counter++;
        }
        
        /* UNLT (unordered or less than) */
        if (__builtin_isunordered(c, b) || c < b) {
            counter++;
        }
        
        /* LTGT (less than or greater than, but not equal and not unordered) */
        if ((c < d || c > d) && !__builtin_isunordered(c, d)) {
            counter++;
        }
        
        /* Standard comparisons mixed in */
        if (a < b) counter++;
        if (c > d) counter++;
        if (a == c) counter++;
        
        /* Use inline asm to prevent optimization */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return counter;
}

int main(void) {
    /* volatile to prevent constant folding */
    volatile double nan_val = make_nan();
    volatile double inf_val = make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    int result;
    
    /* Test various combinations */
    result = fp_test(nan_val, inf_val, normal1, normal2);
    result += fp_test(inf_val, nan_val, zero, normal1);
    result += fp_test(normal1, normal2, nan_val, inf_val);
    result += fp_test(zero, nan_val, inf_val, normal2);
    
    /* Use result to prevent dead code elimination */
    volatile int use_result = result;
    
    /* Print to ensure side effect */
    if (use_result > 1000) {
        /* This should never happen, but prevents optimization */
        __builtin_unreachable();
    }
    
    return use_result & 0xFF;  /* Return non-zero to indicate execution */
}
