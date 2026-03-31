/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386 (32-bit) with x87 FPU math.
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
        /* 1. UNORDERED checks - using __builtin_isunordered */
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* 2. Another unordered check with explicit NaN test */
        if (a != a) {  /* This is true if a is NaN */
            counter += 2;
        }
        
        /* 3. ORDERED check - using __builtin_isnan negation */
        if (!__builtin_isnan(a) && !__builtin_isnan(b)) {
            counter += 3;
        }
        
        /* 4. Direct ordered comparison */
        if (a < b) {
            counter += 4;
        }
        
        /* 5. UNEQ (unordered or equal) - simulate with isnan OR equality */
        if (__builtin_isunordered(a, c) || a == c) {
            counter += 5;
        }
        
        /* 6. UNGE (not less than) - unordered or greater-or-equal */
        if (!(a < b) || __builtin_isunordered(a, b)) {
            counter += 6;
        }
        
        /* 7. UNGT (not less-or-equal) */
        if (!(a <= b) || __builtin_isunordered(a, b)) {
            counter += 7;
        }
        
        /* 8. UNLE (unordered or less-or-equal) */
        if (a <= b || __builtin_isunordered(a, b)) {
            counter += 8;
        }
        
        /* 9. UNLT (unordered or less than) */
        if (a < b || __builtin_isunordered(a, b)) {
            counter += 9;
        }
        
        /* 10. LTGT (less than or greater than, but not equal/unordered) */
        if ((a < b || a > b) && !__builtin_isunordered(a, b)) {
            counter += 10;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to prevent complete optimization */
        a += 0.1;
        b -= 0.1;
        c = (c * 1.1) + 0.5;
        d = (d / 1.1) - 0.5;
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
    volatile double neg = -1.0;
    
    int result;
    
    /* Test various combinations */
    result = fp_test(nan_val, normal1, zero, inf_val);
    result += fp_test(inf_val, nan_val, normal2, neg);
    result += fp_test(normal1, normal2, nan_val, zero);
    result += fp_test(zero, inf_val, normal1, nan_val);
    
    /* Use result to prevent dead code elimination */
    volatile int use_result = result;
    
    /* Print to ensure side effect */
    if (use_result > 1000) {
        /* This should never happen, but prevents optimization */
        __builtin_unreachable();
    }
    
    return use_result & 0xFF;  /* Return non-zero to indicate success */
}
