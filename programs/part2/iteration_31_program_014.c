/* test_i386_condition_codes.c
 * Designed to generate RTL with various floating-point condition codes
 * for i386/x87 backend coverage testing.
 */

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN values */
static volatile double make_nan(void) {
    return 0.0 / 0.0;  /* Generate a quiet NaN */
}

/* Helper to create infinity */
static volatile double make_inf(void) {
    return 1.0 / 0.0;  /* Generate infinity */
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create multiple RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(a, c)) {
            counter += 2;
        }
        
        /* Direct NaN checks that may generate UNORDERED */
        if (a != a) {  /* NaN check */
            counter += 3;
        }
        
        /* UNEQ (unordered or equal) - via == with NaN possibility */
        if (__builtin_isunordered(a, d) || a == d) {
            counter += 4;
        }
        
        /* UNGE (not less than) - !(a < b) with NaN handling */
        if (!(a < b)) {
            counter += 5;
        }
        
        /* UNGT (not less than or equal) - !(a <= b) with NaN handling */
        if (!(a <= b)) {
            counter += 6;
        }
        
        /* UNLE (unordered or less than or equal) */
        if (__builtin_isunordered(a, c) || a <= c) {
            counter += 7;
        }
        
        /* UNLT (unordered or less than) */
        if (__builtin_isunordered(a, d) || a < d) {
            counter += 8;
        }
        
        /* LTGT (less than or greater than, but not equal and not unordered) */
        if (a < b || a > b) {  /* Excludes equal and unordered cases */
            counter += 9;
        }
        
        /* Standard comparisons that may generate ordered condition codes */
        if (b < c) {
            counter += 10;
        }
        
        if (c > d) {
            counter += 11;
        }
        
        if (b == d) {
            counter += 12;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values to create data dependencies */
        a = b + 1.0;
        b = c * 2.0;
        c = d / 2.0;
        d = a - b;
    }
    
    return counter;
}

int main(void) {
    volatile double nan_val = make_nan();
    volatile double inf_val = make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    result += fp_test(nan_val, inf_val, normal1, normal2);
    result += fp_test(inf_val, nan_val, normal2, normal1);
    result += fp_test(normal1, nan_val, inf_val, normal2);
    result += fp_test(normal2, inf_val, nan_val, normal1);
    
    /* Use result to prevent dead code elimination */
    volatile int use_result = result;
    
    /* Simple output to ensure code isn't optimized away */
    if (use_result > 0) {
        return 0;
    } else {
        return 1;
    }
}
