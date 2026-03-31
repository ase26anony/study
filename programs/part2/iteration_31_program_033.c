/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep RTL structure intact */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN values */
static volatile double make_nan(void) {
    return 0.0 / 0.0;
}

/* Helper to create infinity */
static volatile double make_inf(void) {
    return 1.0 / 0.0;
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    
    /* Use inline asm to prevent optimization of values */
    asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d) : : "memory");
    
    /* Loop to create more RTL instructions */
    for (int i = 0; i < 5; i++) {
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(c, d)) {
            counter += 2;
            asm volatile("" : : : "memory");
        }
        
        /* UNEQ (unordered or equal) - via isnan check */
        if (__builtin_isnan(a) || a == b) {
            counter += 3;
            asm volatile("" : : : "memory");
        }
        
        /* UNGE (unordered or greater or equal) */
        if (__builtin_isunordered(a, c) || a >= c) {
            counter += 4;
            asm volatile("" : : : "memory");
        }
        
        /* UNGT (unordered or greater than) */
        if (__builtin_isunordered(b, d) || b > d) {
            counter += 5;
            asm volatile("" : : : "memory");
        }
        
        /* UNLE (unordered or less or equal) */
        if (__builtin_isunordered(c, a) || c <= a) {
            counter += 6;
            asm volatile("" : : : "memory");
        }
        
        /* UNLT (unordered or less than) */
        if (__builtin_isunordered(d, b) || d < b) {
            counter += 7;
            asm volatile("" : : : "memory");
        }
        
        /* LTGT (less than or greater than, but not equal/unordered) */
        if ((a < b) || (a > b)) {
            counter += 8;
            asm volatile("" : : : "memory");
        }
        
        /* Direct comparisons that might generate various condition codes */
        if (a == b) counter += 9;
        if (a != c) counter += 10;
        if (b < d) counter += 11;
        if (c > a) counter += 12;
        if (d >= b) counter += 13;
        if (a <= c) counter += 14;
        
        /* Mix in some integer operations to create more complex CFG */
        asm volatile("" : "+g"(counter) : : "memory");
    }
    
    return counter;
}

int main(void) {
    /* Initialize with various FP values including NaN and Inf */
    volatile double nan_val = make_nan();
    volatile double inf_val = make_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_one = -1.0;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs normal numbers */
    result += fp_test(nan_val, one, zero, inf_val);
    
    /* Test 2: Inf vs normal numbers */
    result += fp_test(inf_val, neg_one, nan_val, zero);
    
    /* Test 3: Normal numbers only */
    result += fp_test(one, zero, neg_one, 2.0);
    
    /* Test 4: NaN vs NaN */
    result += fp_test(nan_val, nan_val, inf_val, inf_val);
    
    /* Test 5: Inf vs Inf */
    result += fp_test(inf_val, inf_val, nan_val, nan_val);
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+g"(result) : : "memory");
    
    /* Print result to ensure all code is live */
    if (result > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Should never happen */
    }
}
