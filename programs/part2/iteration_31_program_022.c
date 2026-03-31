/* test_i386_condition_codes.c
 * Designed to generate RTL with specific floating-point condition codes
 * (UNORDERED, ORDERED, UNEQ, etc.) for i386 x87 backend coverage.
 */

/* Prevent inlining to keep function boundaries clear */
#define NOINLINE __attribute__((noinline))

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Helper to create NaN */
static double make_nan(void) {
    return 0.0 / 0.0;
}

/* Core function with floating-point comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int local_counter = 0;
    
    /* Loop to create more RTL instructions */
    for (int i = 0; i < 10; i++) {
        /* 1. UNORDERED checks */
        if (__builtin_isunordered(a, b)) {
            local_counter += 1;
        }
        
        /* 2. ORDERED checks */
        if (!__builtin_isunordered(a, c)) {
            local_counter += 2;
        }
        
        /* 3. UNEQ (unordered or equal) - using isnan */
        if (__builtin_isnan(a) || a == b) {
            local_counter += 3;
        }
        
        /* 4. UNGE (not less than) */
        if (!(a < b)) {
            local_counter += 4;
        }
        
        /* 5. UNGT (not less than or equal) */
        if (!(a <= b)) {
            local_counter += 5;
        }
        
        /* 6. UNLE (unordered or less than or equal) */
        if (__builtin_isunordered(a, d) || a <= d) {
            local_counter += 6;
        }
        
        /* 7. UNLT (unordered or less than) */
        if (__builtin_isunordered(a, b) || a < b) {
            local_counter += 7;
        }
        
        /* 8. LTGT (less than or greater than, but not equal/unordered) */
        if ((a < b) || (a > b)) {
            local_counter += 8;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values to create data dependencies */
        a = b + 1.0;
        b = c * 2.0;
        c = d - 1.0;
        d = a / 2.0;
    }
    
    return local_counter;
}

/* Another function with different comparison patterns */
NOINLINE static int fp_test2(volatile double x, volatile double y) {
    volatile int counter = 0;
    
    /* Direct NaN comparisons */
    if (x != x) {  /* Generates unordered check */
        counter += 100;
    }
    
    if (x == x) {  /* Generates ordered check */
        counter += 200;
    }
    
    /* Mixed comparisons */
    if (__builtin_isnan(x) && __builtin_isnan(y)) {
        counter += 300;
    }
    
    if (!__builtin_isnan(x) && !__builtin_isnan(y)) {
        counter += 400;
    }
    
    /* Complex expression */
    if ((x < y) || (x > y) || __builtin_isnan(x) || __builtin_isnan(y)) {
        counter += 500;
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN and normal values */
    volatile double nan_val = make_nan();
    volatile double val1 = 3.14159;
    volatile double val2 = 2.71828;
    volatile double val3 = 1.41421;
    
    /* Call test functions */
    int result1 = fp_test(nan_val, val1, val2, val3);
    int result2 = fp_test2(nan_val, val1);
    
    /* Use results to prevent dead code elimination */
    global_counter = result1 + result2;
    
    /* Additional comparisons in main */
    if (__builtin_isunordered(nan_val, val1)) {
        global_counter += 1000;
    }
    
    if (!__builtin_isunordered(val2, val3)) {
        global_counter += 2000;
    }
    
    /* Print to ensure side effects */
    printf("Result: %d\n", global_counter);
    
    return global_counter > 0 ? 0 : 1;
}
