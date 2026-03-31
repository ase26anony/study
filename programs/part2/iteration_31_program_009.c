/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep function boundaries clear */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN */
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
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* Another unordered check with different operands */
        if (a != a) {  /* Classic NaN check */
            counter++;
        }
        
        /* ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(c, d)) {
            counter++;
        }
        
        /* Standard comparisons that might generate UNEQ, UNGE, etc. */
        if (a == b) {
            counter++;
        }
        
        if (a < b) {
            counter++;
        }
        
        if (a >= b) {
            counter++;
        }
        
        if (a > b) {
            counter++;
        }
        
        if (a <= b) {
            counter++;
        }
        
        /* UNEQ (unordered or equal) - using isnan check */
        if (__builtin_isnan(a) || a == b) {
            counter++;
        }
        
        /* LTGT (less than or greater than, but not equal/unordered) */
        if (a < b || a > b) {
            counter++;
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to prevent complete optimization */
        asm volatile("" : "+g"(a), "+g"(b));
    }
    
    return counter;
}

int main(void) {
    /* Use volatile to prevent constant folding */
    volatile double nan_val = make_nan();
    volatile double inf_val = make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs normal */
    result += fp_test(nan_val, normal1, normal1, normal2);
    
    /* Test 2: normal vs NaN */
    result += fp_test(normal1, nan_val, normal2, nan_val);
    
    /* Test 3: NaN vs NaN */
    result += fp_test(nan_val, nan_val, nan_val, nan_val);
    
    /* Test 4: Infinity vs normal */
    result += fp_test(inf_val, normal1, normal1, inf_val);
    
    /* Test 5: normal vs infinity */
    result += fp_test(normal1, inf_val, inf_val, normal1);
    
    /* Test 6: zero vs normal */
    result += fp_test(zero, normal1, normal2, zero);
    
    /* Use result to prevent dead code elimination */
    if (result > 100) {
        /* This should never happen, but prevents optimization */
        asm volatile("" : : "r"(result));
    }
    
    return result > 0 ? 0 : 1;
}
