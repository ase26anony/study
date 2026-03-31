/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep function boundaries clear in RTL */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN values */
static double make_nan(void) {
    return 0.0 / 0.0;
}

/* Helper to create infinity */
static double make_inf(void) {
    return 1.0 / 0.0;
}

/* Function with various FP comparisons that should generate different condition codes */
NOINLINE static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* Another unordered check using different method */
        if (a != a) {  /* NaN check */
            counter++;
        }
        
        /* ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(a, c)) {
            counter++;
        }
        
        /* Standard comparisons that might generate UNEQ, UNGE, UNGT, etc. */
        if (a == b) {  /* EQ */
            counter++;
        }
        
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
        
        /* More complex comparisons */
        if (!(a < b) && !(a > b)) {  /* UNEQ? */
            counter++;
        }
        
        if (!(a < b)) {  /* UNLT? (nlt) */
            counter++;
        }
        
        if (!(a > b)) {  /* UNGT? (nle) */
            counter++;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to prevent complete optimization */
        asm volatile("" : "+g"(a), "+g"(b));
    }
    
    return counter;
}

/* Another test function focusing on ordered/unordered comparisons */
NOINLINE static int fp_test2(volatile double x, volatile double y) {
    volatile int result = 0;
    
    /* Direct unordered comparison */
    if (__builtin_isunordered(x, y)) {
        result |= 1;
    }
    
    /* Ordered comparison */
    if (!__builtin_isunordered(x, y)) {
        result |= 2;
    }
    
    /* LTGT comparison (une) */
    if (x < y || x > y) {
        result |= 4;
    }
    
    /* UNLE comparison (ule) */
    if (!(x > y)) {
        result |= 8;
    }
    
    /* UNGE comparison (nlt) */
    if (!(x < y)) {
        result |= 16;
    }
    
    return result;
}

int main(void) {
    volatile double nan_val = make_nan();
    volatile double inf_val = make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    int result1, result2, result3;
    
    /* Test 1: NaN vs normal number */
    result1 = fp_test(nan_val, normal1, normal2);
    
    /* Test 2: Infinity vs normal number */
    result2 = fp_test(inf_val, normal1, normal2);
    
    /* Test 3: Normal vs normal */
    result3 = fp_test(normal1, normal2, nan_val);
    
    /* Test 4: Various special comparisons */
    int result4 = fp_test2(nan_val, normal1);
    int result5 = fp_test2(normal1, nan_val);
    int result6 = fp_test2(inf_val, normal1);
    int result7 = fp_test2(zero, zero);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = 
        result1 + result2 + result3 + result4 + result5 + result6 + result7;
    
    /* Print to ensure side effects */
    printf("Result: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}
