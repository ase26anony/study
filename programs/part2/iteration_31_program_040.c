/* test_i386_condition_codes.c
 * Generate RTL with various floating-point condition codes for i386 x87 backend
 */

/* Prevent inlining to keep function boundaries clear */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN */
static volatile double make_nan(void) {
    return 0.0 / 0.0;
}

/* Force value to be used and prevent optimization */
static void use_result(int val) {
    asm volatile("" : : "r"(val) : "memory");
}

/* Core function with floating-point comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    int i;
    
    /* Loop to generate more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks - should generate "unord" */
        if (__builtin_isunordered(a, b)) {
            counter += 1;
        }
        
        /* ORDERED checks - should generate "ord" */
        if (!__builtin_isunordered(a, c)) {
            counter += 2;
        }
        
        /* UNEQ (unordered or equal) - via isnan check */
        if (__builtin_isnan(a) || a == b) {
            counter += 3;
        }
        
        /* UNGE (not less than) - should generate "nlt" */
        if (!(a < b)) {
            counter += 4;
        }
        
        /* UNGT (not less than or equal) - should generate "nle" */
        if (!(a <= b)) {
            counter += 5;
        }
        
        /* UNLE (unordered or less than or equal) - should generate "ule" */
        if (__builtin_isunordered(a, b) || a <= b) {
            counter += 6;
        }
        
        /* UNLT (unordered or less than) - should generate "ult" */
        if (__builtin_isunordered(a, b) || a < b) {
            counter += 7;
        }
        
        /* LTGT (less than or greater than, but not equal/unordered) - should generate "une" */
        if (a < b || a > b) {
            counter += 8;
        }
        
        /* Standard comparisons mixed in */
        if (a == c) counter += 9;
        if (a != c) counter += 10;
        if (a < c)  counter += 11;
        if (a > c)  counter += 12;
        if (a <= c) counter += 13;
        if (a >= c) counter += 14;
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to prevent complete optimization */
        a = a * 1.0001;
        b = b + 0.0001;
        c = c - 0.0001;
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN, normal, and infinity values */
    volatile double nan_val = make_nan();
    volatile double normal_val = 3.14159;
    volatile double inf_val = 1.0 / 0.0;
    volatile double neg_inf_val = -1.0 / 0.0;
    volatile double zero_val = 0.0;
    
    /* Test various combinations */
    int result = 0;
    
    result += fp_test(nan_val, normal_val, inf_val);
    result += fp_test(normal_val, nan_val, zero_val);
    result += fp_test(inf_val, neg_inf_val, normal_val);
    result += fp_test(zero_val, inf_val, nan_val);
    
    /* Use result to prevent dead code elimination */
    use_result(result);
    
    return result != 0 ? 0 : 1;
}
