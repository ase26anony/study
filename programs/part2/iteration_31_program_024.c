/* test_i386_condition_codes.c
 * Designed to generate RTL with various floating-point condition codes
 * for i386/x87 backend coverage testing.
 */

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;

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
    volatile int local_counter = 0;
    
    /* Use a loop to create more RTL instructions */
    for (int i = 0; i < 10; i++) {
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            local_counter += 1;
        }
        
        /* ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(c, d)) {
            local_counter += 2;
        }
        
        /* Direct NaN checks - may generate UNORDERED */
        if (a != a) {  /* NaN check */
            local_counter += 3;
        }
        
        /* UNEQ (unordered or equal) - via isnan OR equality */
        if (__builtin_isnan(a) || a == b) {
            local_counter += 4;
        }
        
        /* UNGE (unordered or greater or equal) - !(a < b) */
        if (!(a < b)) {
            local_counter += 5;
        }
        
        /* UNGT (unordered or greater than) - !(a <= b) */
        if (!(a <= b)) {
            local_counter += 6;
        }
        
        /* UNLE (unordered or less or equal) - a <= b (handles NaN) */
        if (a <= b) {
            local_counter += 7;
        }
        
        /* UNLT (unordered or less than) - a < b (handles NaN) */
        if (a < b) {
            local_counter += 8;
        }
        
        /* LTGT (less than or greater than, but not equal and not unordered) */
        if (a != b && !__builtin_isunordered(a, b)) {
            local_counter += 9;
        }
        
        /* Standard comparisons mixed with unordered checks */
        if (c > d && !__builtin_isunordered(c, d)) {
            local_counter += 10;
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to create different conditions */
        a += 1.0;
        b -= 1.0;
    }
    
    return local_counter;
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
    
    /* Test various combinations to trigger different condition codes */
    int result = 0;
    
    /* NaN vs NaN - should trigger UNORDERED */
    result += fp_test(nan1, nan2, normal1, normal2);
    
    /* NaN vs normal - should trigger UNORDERED */
    result += fp_test(nan1, normal1, normal2, nan2);
    
    /* Normal vs normal - should trigger ORDERED comparisons */
    result += fp_test(normal1, normal2, nan1, nan2);
    
    /* Infinity comparisons */
    result += fp_test(inf_pos, inf_neg, normal1, inf_pos);
    
    /* Zero comparisons */
    result += fp_test(zero, normal1, zero, -zero);
    
    /* Mixed cases */
    result += fp_test(nan1, inf_pos, inf_neg, nan2);
    
    /* Use result to prevent dead code elimination */
    global_counter = result;
    
    /* Print to ensure side effect */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
