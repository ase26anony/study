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
    
    /* Use inline asm to prevent optimization */
    asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
    
    /* Loop to generate more RTL instructions */
    for (int i = 0; i < 10; i++) {
        /* 1. UNORDERED checks (should generate UNORDERED condition code) */
        if (__builtin_isunordered(a, b)) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* 2. ORDERED checks (should generate ORDERED condition code) */
        if (!__builtin_isunordered(a, c)) {
            counter += 2;
            asm volatile("" : : : "memory");
        }
        
        /* 3. Direct NaN checks (may generate UNORDERED) */
        if (a != a) {  /* NaN check */
            counter += 3;
        }
        
        /* 4. UNEQ (unordered or equal) - via builtin */
        if (__builtin_isunordered(a, d) || a == d) {
            counter += 4;
        }
        
        /* 5. UNGE (unordered or greater-or-equal) */
        if (__builtin_isunordered(b, c) || b >= c) {
            counter += 5;
        }
        
        /* 6. UNGT (unordered or greater) */
        if (__builtin_isunordered(b, d) || b > d) {
            counter += 6;
        }
        
        /* 7. UNLE (unordered or less-or-equal) */
        if (__builtin_isunordered(c, d) || c <= d) {
            counter += 7;
        }
        
        /* 8. UNLT (unordered or less) */
        if (__builtin_isunordered(a, c) || a < c) {
            counter += 8;
        }
        
        /* 9. LTGT (less or greater, but not equal/unordered) */
        if ((a < b) || (a > b)) {
            counter += 9;
        }
        
        /* 10. Mixed ordered comparisons */
        if (a == b) {
            counter += 10;
        }
        if (a < c) {
            counter += 11;
        }
        if (b > d) {
            counter += 12;
        }
        
        /* Modify values slightly to prevent complete optimization */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
    }
    
    return counter;
}

int main(void) {
    /* Initialize with various FP values */
    volatile double nan1 = make_nan();
    volatile double nan2 = __builtin_nan("0xdead");
    volatile double inf_pos = make_inf();
    volatile double inf_neg = -make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    volatile double neg = -1.0;
    
    /* Prevent constant folding */
    asm volatile("" : "+g"(nan1), "+g"(nan2), "+g"(inf_pos), 
                       "+g"(inf_neg), "+g"(normal1), "+g"(normal2),
                       "+g"(zero), "+g"(neg));
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs NaN */
    result += fp_test(nan1, nan2, nan1, nan2);
    
    /* Test 2: NaN vs normal */
    result += fp_test(nan1, normal1, normal2, zero);
    
    /* Test 3: Normal vs normal */
    result += fp_test(normal1, normal2, zero, neg);
    
    /* Test 4: Infinity vs NaN */
    result += fp_test(inf_pos, nan1, inf_neg, normal1);
    
    /* Test 5: Mixed */
    result += fp_test(zero, inf_pos, nan2, neg);
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+r"(result));
    
    return result != 0 ? 0 : 1;
}
