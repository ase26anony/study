/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep RTL structure intact */
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
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            counter++;
            asm volatile("" : "+g"(counter) : : "memory");
        }
        
        /* ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(a, c)) {
            counter += 2;
            asm volatile("" : "+g"(counter) : : "memory");
        }
        
        /* Direct NaN checks - may generate UNORDERED/ORDERED */
        if (a != a) {  /* NaN check */
            counter += 3;
        }
        
        if (b == b) {  /* Not NaN check */
            counter += 4;
        }
        
        /* Standard comparisons that might generate UNEQ, UNGE, UNGT, etc. */
        if (a == c) {  /* EQ - may become UNEQ with NaN handling */
            counter += 5;
        }
        
        if (a >= d) {  /* GE - may become UNGE with NaN handling */
            counter += 6;
        }
        
        if (a > b) {   /* GT - may become UNGT with NaN handling */
            counter += 7;
        }
        
        if (a <= c) {  /* LE - may become UNLE with NaN handling */
            counter += 8;
        }
        
        if (a < d) {   /* LT - may become UNLT with NaN handling */
            counter += 9;
        }
        
        if (a < b || a > b) {  /* LTGT - "not equal and ordered" */
            counter += 10;
        }
        
        /* Use inline asm to prevent optimization */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
    }
    
    return counter;
}

int main(void) {
    /* Initialize with various FP values */
    volatile double nan_val = make_nan();
    volatile double inf_val = make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs normal */
    result += fp_test(nan_val, normal1, normal2, zero);
    
    /* Test 2: normal vs NaN */
    result += fp_test(normal1, nan_val, inf_val, normal2);
    
    /* Test 3: NaN vs NaN */
    result += fp_test(nan_val, nan_val, normal1, inf_val);
    
    /* Test 4: Infinity vs normal */
    result += fp_test(inf_val, normal1, nan_val, zero);
    
    /* Test 5: normal vs infinity */
    result += fp_test(normal1, inf_val, normal2, nan_val);
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+g"(result));
    
    /* Print to ensure side effect */
    if (result > 1000) {
        __builtin_printf("Result: %d\n", result);
    }
    
    return result > 0 ? 0 : 1;
}
