/* test_i386_condition_codes.c
 * Designed to generate RTL with various floating-point condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep function boundaries clear */
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
    int i;
    
    /* Use asm to prevent optimization of loop */
    asm volatile("" : "+g"(counter));
    
    /* Loop to generate more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(a, c)) {
            counter += 2;
            asm volatile("" : : : "memory");
        }
        
        /* UNEQ (unordered or equal) - via explicit check */
        if (__builtin_isunordered(a, d) || a == d) {
            counter += 3;
        }
        
        /* UNGE (unordered or greater than or equal) */
        if (__builtin_isunordered(b, c) || b >= c) {
            counter += 4;
        }
        
        /* UNGT (unordered or greater than) */
        if (__builtin_isunordered(b, d) || b > d) {
            counter += 5;
        }
        
        /* UNLE (unordered or less than or equal) */
        if (__builtin_isunordered(c, d) || c <= d) {
            counter += 6;
        }
        
        /* UNLT (unordered or less than) */
        if (__builtin_isunordered(c, a) || c < a) {
            counter += 7;
        }
        
        /* LTGT (less than or greater than, but not equal/unordered) */
        if ((c < d || c > d) && !__builtin_isunordered(c, d)) {
            counter += 8;
        }
        
        /* Standard comparisons mixed in */
        if (a < b) counter += 9;
        if (a > c) counter += 10;
        if (a == d) counter += 11;
        if (b <= c) counter += 12;
        if (c >= d) counter += 13;
        
        /* Modify values slightly to prevent complete optimization */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
    }
    
    return counter;
}

/* Another test function with different patterns */
NOINLINE static int fp_test2(volatile double x, volatile double y) {
    volatile int result = 0;
    
    /* Direct NaN checks */
    if (x != x) {  /* This checks for NaN (unordered) */
        result |= 1;
    }
    
    if (y == y) {  /* This checks for ordered (not NaN) */
        result |= 2;
    }
    
    /* Complex expression that might generate UNORDERED */
    if (__builtin_isnan(x) || __builtin_isnan(y)) {
        result |= 4;
    }
    
    /* Mix with integer operations to create more complex RTL */
    int *ptr = &result;
    asm volatile("" : "+r"(ptr));
    
    return result;
}

int main(void) {
    /* Initialize with various special FP values */
    volatile double nan1 = make_nan();
    volatile double nan2 = __builtin_nan("");
    volatile double inf_pos = make_inf();
    volatile double inf_neg = -make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    volatile double neg_zero = -0.0;
    
    /* Use asm to prevent constant propagation */
    asm volatile("" : "+g"(nan1), "+g"(nan2), "+g"(inf_pos), 
                       "+g"(inf_neg), "+g"(normal1), "+g"(normal2),
                       "+g"(zero), "+g"(neg_zero));
    
    /* Call test functions with various combinations */
    int result1 = fp_test(nan1, normal1, inf_pos, zero);
    int result2 = fp_test2(nan2, normal2);
    int result3 = fp_test(inf_neg, nan2, normal1, inf_pos);
    int result4 = fp_test2(zero, neg_zero);  /* +0 and -0 compare equal */
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 + result2 + result3 + result4;
    
    /* Print to ensure side effect */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
