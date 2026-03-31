/* test_i386_condition_codes.c
 * Designed to generate RTL with specific floating-point condition codes
 * (UNORDERED, ORDERED, UNEQ, etc.) for i386 x87 backend coverage.
 */

/* Prevent inlining to keep function structure */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN */
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
    
    /* Loop to create more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* 1. UNORDERED checks */
        if (__builtin_isunordered(a, b)) {
            counter++;
            /* Use asm to prevent optimization */
            asm volatile("" : "+g"(counter) : : "memory");
        }
        
        /* 2. ORDERED checks */
        if (!__builtin_isunordered(a, c)) {  /* Equivalent to ORDERED */
            counter += 2;
        }
        
        /* 3. Direct comparisons that may generate UNEQ, UNGE, UNGT, etc. */
        if (a == b) {  /* May generate EQ or UNEQ */
            counter += 3;
        }
        
        /* 4. Less-than/greater-than with NaN involvement */
        if (a < c) {   /* Standard LT, but with NaN operand may affect CC */
            counter += 4;
        }
        
        if (c > d) {   /* Standard GT */
            counter += 5;
        }
        
        /* 5. Not-equal check (may generate NE or LTGT) */
        if (a != b) {
            counter += 6;
        }
        
        /* 6. More complex condition using isnan */
        if (__builtin_isnan(a) || __builtin_isnan(b)) {
            counter += 7;
        }
        
        /* 7. Ordered comparison after barrier */
        asm volatile("" : : : "memory");
        if (b <= d) {  /* LE */
            counter += 8;
        }
        
        /* 8. Greater-or-equal */
        if (c >= a) {  /* GE */
            counter += 9;
        }
        
        /* Modify values slightly to prevent complete optimization */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
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
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs NaN */
    result += fp_test(nan1, nan2, normal1, normal2);
    
    /* Test 2: NaN vs normal */
    result += fp_test(nan1, normal1, normal2, inf_pos);
    
    /* Test 3: Normal vs normal */
    result += fp_test(normal1, normal2, inf_pos, inf_neg);
    
    /* Test 4: Inf vs NaN */
    result += fp_test(inf_pos, nan1, inf_neg, normal1);
    
    /* Test 5: Special zeros */
    result += fp_test(zero, neg_zero, normal1, normal2);
    
    /* Test 6: Inf vs Inf */
    result += fp_test(inf_pos, inf_pos, inf_neg, inf_neg);
    
    /* Test 7: Normal vs Inf */
    result += fp_test(normal1, inf_pos, normal2, inf_neg);
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+r"(result));
    
    /* Print to ensure side effect */
    if (result > 1000) {
        __builtin_printf("Result: %d\n", result);
    }
    
    return 0;
}
