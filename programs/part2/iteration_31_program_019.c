/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep RTL structure intact */
#define NOINLINE __attribute__((noinline))

/* Global volatile counter to prevent optimization */
volatile int counter = 0;

/* Memory barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Helper to create NaN */
static volatile double make_nan(void) {
    return 0.0 / 0.0;
}

/* Core function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int local_counter = 0;
    
    /* Loop to create more RTL instructions */
    for (int i = 0; i < 10; i++) {
        /* 1. UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            local_counter += 1;
            COMPILER_BARRIER();
        }
        
        /* 2. Direct NaN check - may generate UNORDERED */
        if (a != a) {  /* Classic NaN check */
            local_counter += 2;
        }
        
        /* 3. ORDERED check - should generate ORDERED condition code */
        if (!__builtin_isunordered(c, d)) {
            local_counter += 3;
            COMPILER_BARRIER();
        }
        
        /* 4. UNEQ (unordered or equal) - via __builtin_isunordered or == */
        if (__builtin_isunordered(a, c) || a == c) {
            local_counter += 4;
        }
        
        /* 5. UNGE (not less than) - unordered or greater-or-equal */
        if (!(a < b)) {  /* May generate UNGE/nlt */
            local_counter += 5;
        }
        
        /* 6. UNGT (not less or equal) - unordered or greater */
        if (!(a <= b)) {  /* May generate UNGT/nle */
            local_counter += 6;
        }
        
        /* 7. UNLE (unordered or less-or-equal) */
        if (__builtin_isunordered(a, d) || a <= d) {
            local_counter += 7;
        }
        
        /* 8. UNLT (unordered or less than) */
        if (__builtin_isunordered(b, c) || b < c) {
            local_counter += 8;
        }
        
        /* 9. LTGT (less or greater, but not equal/unordered) - via != */
        if (a != b) {  /* May generate LTGT/une */
            local_counter += 9;
        }
        
        /* Mix in some ordered comparisons */
        if (c < d) {
            local_counter += 10;
        }
        
        if (c == d) {
            local_counter += 11;
        }
        
        if (c > d) {
            local_counter += 12;
        }
        
        /* Modify values slightly to prevent complete optimization */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
    }
    
    return local_counter;
}

int main(void) {
    /* Initialize with various values including NaN */
    volatile double nan_val = make_nan();
    volatile double inf_val = 1.0 / 0.0;
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    volatile double neg = -1.5;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs normal */
    result += fp_test(nan_val, normal1, normal2, zero);
    
    /* Test 2: normal vs normal */
    result += fp_test(normal1, normal2, zero, neg);
    
    /* Test 3: NaN vs NaN */
    result += fp_test(nan_val, nan_val, normal1, normal2);
    
    /* Test 4: normal vs inf */
    result += fp_test(normal1, inf_val, neg, zero);
    
    /* Test 5: inf vs inf */
    result += fp_test(inf_val, inf_val, normal1, nan_val);
    
    /* Use result to prevent dead code elimination */
    counter = result;
    
    /* Print to ensure side effect */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
