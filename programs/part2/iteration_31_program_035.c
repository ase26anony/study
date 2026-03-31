/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep RTL structure intact */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN values */
static double make_nan(void) {
    return 0.0 / 0.0;
}

/* Helper to create infinity */
static double make_inf(void) {
    return 1.0 / 0.0;
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* 1. UNORDERED checks - using various methods */
        if (__builtin_isunordered(a, b)) {
            counter++;  /* Side effect to keep branch */
        }
        
        /* Another unordered check with different syntax */
        if (a != a) {  /* This is true for NaN */
            counter++;
        }
        
        /* 2. ORDERED checks */
        if (!__builtin_isunordered(c, d)) {
            counter++;
        }
        
        /* 3. UNEQ (unordered or equal) - simulate with isnan || == */
        if (__builtin_isnan(a) || a == b) {
            counter++;
        }
        
        /* 4. UNGE (unordered or greater or equal) */
        if (__builtin_isunordered(a, c) || a >= c) {
            counter++;
        }
        
        /* 5. UNGT (unordered or greater than) */
        if (__builtin_isunordered(b, d) || b > d) {
            counter++;
        }
        
        /* 6. UNLE (unordered or less or equal) */
        if (__builtin_isunordered(c, a) || c <= a) {
            counter++;
        }
        
        /* 7. UNLT (unordered or less than) */
        if (__builtin_isunordered(d, b) || d < b) {
            counter++;
        }
        
        /* 8. LTGT (less than or greater than, but not equal/unordered) */
        if ((a < b) || (a > b)) {  /* Equivalent to a != b but both ordered */
            counter++;
        }
        
        /* Standard comparisons to mix in */
        if (a < b) counter++;
        if (c > d) counter++;
        if (a == c) counter++;
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to prevent loop unrolling */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
    }
    
    return counter;
}

int main(void) {
    /* Initialize with various FP values */
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double inf_pos = make_inf();
    volatile double inf_neg = -make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    volatile double neg = -1.0;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs NaN */
    result += fp_test(nan1, nan2, nan1, nan2);
    
    /* Test 2: NaN vs normal */
    result += fp_test(nan1, normal1, normal2, nan2);
    
    /* Test 3: Infinity vs normal */
    result += fp_test(inf_pos, normal1, inf_neg, normal2);
    
    /* Test 4: Normal vs normal */
    result += fp_test(normal1, normal2, zero, neg);
    
    /* Test 5: Mixed */
    result += fp_test(nan1, inf_pos, zero, nan2);
    
    /* Use result to prevent dead code elimination */
    if (result > 1000) {
        /* This should never happen, but prevents optimization */
        asm volatile("" : : "r"(result));
    }
    
    return result != 0 ? 0 : 1;
}
