/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386 backend coverage.
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

/* Core function with floating-point comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create multiple RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks - using various methods */
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        if (a != a) {  /* Classic NaN check - should generate unordered */
            counter += 2;
        }
        
        /* ORDERED checks */
        if (__builtin_isordered(a, c)) {
            counter += 3;
        }
        
        /* UNEQ (unordered or equal) */
        if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
            /* This can generate UNEQ in some cases */
            counter += 4;
        }
        
        /* UNGE (not less than) - using builtin */
        if (!__builtin_isless(a, d)) {
            counter += 5;
        }
        
        /* UNGT (not less than or equal) */
        if (!__builtin_islessequal(a, b)) {
            counter += 6;
        }
        
        /* UNLE (unordered or less than or equal) */
        if (!__builtin_isgreater(a, c)) {
            counter += 7;
        }
        
        /* UNLT (unordered or less than) */
        if (!__builtin_isgreaterequal(a, d)) {
            counter += 8;
        }
        
        /* LTGT (less than or greater than - ordered and not equal) */
        if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) {
            counter += 9;
        }
        
        /* Standard comparisons that might generate different codes */
        if (a < b) {
            counter += 10;
        }
        
        if (a > c) {
            counter += 11;
        }
        
        if (a == d) {
            counter += 12;
        }
        
        /* Use inline assembly to create data dependencies */
        asm volatile ("" : "+g" (a), "+g" (b), "+g" (c), "+g" (d));
        
        /* Memory barrier to prevent reordering */
        asm volatile ("" : : : "memory");
    }
    
    return counter;
}

int main(void) {
    /* Initialize with various special FP values */
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double inf_pos = make_inf();
    volatile double inf_neg = -make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    volatile double neg_zero = -0.0;
    
    /* Mix of comparisons with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs NaN */
    result += fp_test(nan1, nan2, normal1, normal2);
    
    /* Test 2: NaN vs normal */
    result += fp_test(nan1, normal1, normal2, inf_pos);
    
    /* Test 3: Normal vs normal */
    result += fp_test(normal1, normal2, inf_pos, inf_neg);
    
    /* Test 4: Infinity comparisons */
    result += fp_test(inf_pos, inf_neg, normal1, nan1);
    
    /* Test 5: Zero comparisons */
    result += fp_test(zero, neg_zero, normal1, inf_pos);
    
    /* Use result to prevent optimization */
    volatile int use_result = result;
    
    /* Print to prevent dead code elimination */
    if (use_result > 1000) {
        __builtin_printf("Result: %d\n", use_result);
    }
    
    return use_result > 0 ? 0 : 1;
}
