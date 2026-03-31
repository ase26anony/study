/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep RTL structure intact */
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
        /* UNORDERED checks - using various methods */
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        if (a != a) {  /* Classic NaN check */
            counter++;
        }
        
        if (__builtin_isnan(a)) {
            counter++;
        }
        
        /* ORDERED checks */
        if (__builtin_isordered(a, b)) {
            counter++;
        }
        
        if (a == a) {  /* Ordered check (not NaN) */
            counter++;
        }
        
        /* Standard comparisons that may generate various condition codes */
        if (a < b) {   /* LT */
            counter++;
        }
        
        if (a > b) {   /* GT */
            counter++;
        }
        
        if (a <= b) {  /* LE */
            counter++;
        }
        
        if (a >= b) {  /* GE */
            counter++;
        }
        
        if (a == b) {  /* EQ */
            counter++;
        }
        
        if (a != b) {  /* NE */
            counter++;
        }
        
        /* More complex comparisons with different operands */
        if (__builtin_isunordered(c, d)) {
            counter++;
        }
        
        if (__builtin_isordered(c, d)) {
            counter++;
        }
        
        /* LTGT (unordered but not equal) */
        if (c < d || c > d) {
            counter++;
        }
        
        /* UNEQ (unordered or equal) */
        if (!(c < d) && !(c > d)) {
            counter++;
        }
        
        /* UNGE (not less than) */
        if (!(c < d)) {
            counter++;
        }
        
        /* UNGT (not less than or equal) */
        if (!(c <= d)) {
            counter++;
        }
        
        /* UNLE (unordered or less than or equal) */
        if (c <= d || __builtin_isunordered(c, d)) {
            counter++;
        }
        
        /* UNLT (unordered or less than) */
        if (c < d || __builtin_isunordered(c, d)) {
            counter++;
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to prevent complete optimization */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
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
    volatile double neg_inf = -1.0 / 0.0;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs normal */
    result += fp_test(nan_val, normal1, normal2, nan_val);
    
    /* Test 2: Inf vs normal */
    result += fp_test(inf_val, normal1, normal2, inf_val);
    
    /* Test 3: Normal vs normal */
    result += fp_test(normal1, normal2, normal1, normal2);
    
    /* Test 4: NaN vs Inf */
    result += fp_test(nan_val, inf_val, neg_inf, nan_val);
    
    /* Test 5: Zero vs NaN */
    result += fp_test(zero, nan_val, nan_val, zero);
    
    /* Use result to prevent dead code elimination */
    if (result > 1000) {
        /* This should never happen, but prevents optimization */
        asm volatile("" : : "r"(result));
    }
    
    return result > 0 ? 0 : 1;
}
