/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386 backend coverage testing.
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
    
    /* Use inline assembly to prevent optimization */
    asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
    
    /* Loop to generate more RTL instructions */
    for (int i = 0; i < 10; i++) {
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* Another unordered check with different variables */
        if (a != a) {  /* Classic NaN check */
            counter += 2;
        }
        
        /* ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(c, d)) {
            counter += 3;
        }
        
        /* UNEQ (unordered or equal) - using isnan() */
        if (__builtin_isnan(a) || (a == b)) {
            counter += 4;
        }
        
        /* UNGE (not less than) - unordered or greater-or-equal */
        if (!(a < b)) {
            counter += 5;
        }
        
        /* UNGT (not less-or-equal) - unordered or greater */
        if (!(a <= b)) {
            counter += 6;
        }
        
        /* UNLE (unordered or less-or-equal) */
        if (__builtin_isunordered(a, b) || (a <= b)) {
            counter += 7;
        }
        
        /* UNLT (unordered or less than) */
        if (__builtin_isunordered(a, b) || (a < b)) {
            counter += 8;
        }
        
        /* LTGT (less than or greater than, but not equal/unordered) */
        if ((a < b) || (a > b)) {
            counter += 9;
        }
        
        /* Regular comparisons mixed in */
        if (c == d) {
            counter += 10;
        }
        
        if (c < d) {
            counter += 11;
        }
        
        if (c > d) {
            counter += 12;
        }
        
        if (c <= d) {
            counter += 13;
        }
        
        if (c >= d) {
            counter += 14;
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to create data dependencies */
        a += 1.0;
        b -= 0.5;
        c *= 1.1;
        d /= 1.05;
    }
    
    return counter;
}

int main(void) {
    /* Initialize with various FP values including NaN and Inf */
    volatile double nan1 = make_nan();
    volatile double nan2 = __builtin_nan("0xdead");
    volatile double inf1 = make_inf();
    volatile double inf2 = -make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    volatile double neg_zero = -0.0;
    
    /* Prevent constant folding */
    asm volatile("" : "+g"(nan1), "+g"(nan2), "+g"(inf1), 
                        "+g"(inf2), "+g"(normal1), "+g"(normal2),
                        "+g"(zero), "+g"(neg_zero));
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs Normal */
    result += fp_test(nan1, normal1, normal2, zero);
    
    /* Test 2: NaN vs NaN */
    result += fp_test(nan1, nan2, inf1, inf2);
    
    /* Test 3: Inf vs Normal */
    result += fp_test(inf1, normal1, normal2, inf2);
    
    /* Test 4: Normal vs Normal */
    result += fp_test(normal1, normal2, zero, neg_zero);
    
    /* Test 5: Mixed */
    result += fp_test(nan1, inf1, normal1, zero);
    
    /* Use result to prevent dead code elimination */
    volatile int use_result = result;
    asm volatile("" : "+g"(use_result));
    
    return use_result > 0 ? 0 : 1;
}
