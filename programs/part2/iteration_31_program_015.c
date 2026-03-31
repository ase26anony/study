/* test_i386_condition_codes.c
 * Designed to trigger UNORDERED, ORDERED, and other floating-point
 * condition code generation in GCC's i386 backend RTL printer.
 */

/* Prevent inlining to preserve RTL structure */
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
    
    /* Loop to create more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks - using various methods */
        if (__builtin_isunordered(a, b)) {
            counter++;
            asm volatile("" : : : "memory"); /* Barrier */
        }
        
        if (a != a) { /* NaN check, should generate unordered */
            counter += 2;
        }
        
        if (__builtin_isnan(c)) {
            counter += 3;
        }
        
        /* ORDERED checks */
        if (!__builtin_isunordered(a, b)) {
            counter += 4;
        }
        
        if (a == a) { /* Not NaN, should generate ordered */
            counter += 5;
        }
        
        /* Standard comparisons that may generate UNEQ, UNGE, UNGT, etc. */
        if (a == b) {
            counter += 6;
        }
        
        if (a >= b) {
            counter += 7;
        }
        
        if (a > b) {
            counter += 8;
        }
        
        if (a <= b) {
            counter += 9;
        }
        
        if (a < b) {
            counter += 10;
        }
        
        /* LTGT (unordered not equal) */
        if (a != b) {
            counter += 11;
        }
        
        /* Mix in some ordered comparisons with NaN operands */
        if (c < d) {
            counter += 12;
        }
        
        if (c > d) {
            counter += 13;
        }
        
        /* Use inline assembly to create data dependencies */
        asm volatile("" : "+g"(a), "+g"(b) : : "memory");
        
        /* Create more complex expressions */
        volatile double tmp = a + b;
        if (__builtin_isunordered(tmp, c)) {
            counter += 14;
        }
        
        if (!__builtin_isunordered(tmp, d)) {
            counter += 15;
        }
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
    result += fp_test(nan1, normal1, nan2, normal2);
    
    /* Test 3: normal vs normal */
    result += fp_test(normal1, normal2, nan1, inf_pos);
    
    /* Test 4: Infinity comparisons */
    result += fp_test(inf_pos, inf_neg, normal1, nan1);
    
    /* Test 5: Zero comparisons */
    result += fp_test(zero, neg_zero, inf_pos, inf_neg);
    
    /* Test 6: Mixed */
    result += fp_test(nan1, inf_pos, zero, normal1);
    
    /* Use result to prevent optimization */
    volatile int use_result = result;
    
    /* Print to prevent dead code elimination */
    if (use_result > 1000000) {
        /* This should never happen, but prevents optimization */
        __builtin_unreachable();
    }
    
    return use_result & 0xFF; /* Return non-zero to indicate success */
}
