/* test_i386_condition_codes.c
 * Generates RTL with various floating-point condition codes for i386 backend
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

/* Force compiler to keep variable in register/memory */
#define KEEP(var) asm volatile("" : "+g"(var))

/* Memory barrier to prevent reordering */
#define BARRIER() asm volatile("" : : : "memory")

/* Function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            counter++;
            BARRIER();
        }
        
        /* ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(a, c)) {
            counter += 2;
            BARRIER();
        }
        
        /* UNEQ (unordered or equal) - via isnan check */
        if (__builtin_isnan(a) || a == b) {
            counter += 3;
        }
        
        /* UNGE (not less than) - unordered or greater-or-equal */
        if (!(a < b)) {
            counter += 4;
            BARRIER();
        }
        
        /* UNGT (not less-or-equal) - unordered or greater */
        if (!(a <= b)) {
            counter += 5;
        }
        
        /* UNLE (unordered or less-or-equal) */
        if (__builtin_isunordered(a, d) || a <= d) {
            counter += 6;
            BARRIER();
        }
        
        /* UNLT (unordered or less than) */
        if (__builtin_isunordered(a, b) || a < b) {
            counter += 7;
        }
        
        /* LTGT (less than or greater than, but not equal/unordered) */
        if ((a < b) || (a > b)) {
            counter += 8;
            BARRIER();
        }
        
        /* Direct comparisons that might generate various codes */
        if (a == c) counter += 9;
        if (a != d) counter += 10;
        if (a < c)  counter += 11;
        if (a > d)  counter += 12;
        if (a <= b) counter += 13;
        if (a >= c) counter += 14;
        
        /* Mix in some integer math to create more complex CFG */
        KEEP(counter);
    }
    
    return counter;
}

/* Another test function with different patterns */
NOINLINE static int fp_test2(volatile double x, volatile double y) {
    volatile int result = 0;
    
    /* Test all condition code variants in switch-like pattern */
    if (__builtin_isunordered(x, y)) {
        result = 1;  /* UNORDERED */
    } else if (x == y) {
        result = 2;  /* EQ */
    } else if (x < y) {
        result = 3;  /* LT */
    } else if (x > y) {
        result = 4;  /* GT */
    } else if (x <= y) {
        result = 5;  /* LE */
    } else if (x >= y) {
        result = 6;  /* GE */
    }
    
    /* More complex expression that might generate UNEQ */
    if (!__builtin_isnan(x) && x == y) {
        result += 10;
    }
    
    /* Expression that might generate LTGT */
    if (x != y && !__builtin_isunordered(x, y)) {
        result += 20;
    }
    
    return result;
}

int main(void) {
    /* Use volatile to prevent constant folding */
    volatile double nan_val = make_nan();
    volatile double inf_val = make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    volatile double neg = -1.0;
    
    int result1, result2;
    
    /* Test with NaN involved */
    result1 = fp_test(nan_val, normal1, normal2, inf_val);
    
    /* Test with normal values */
    result2 = fp_test2(normal1, normal2);
    
    /* Test with infinity */
    result1 += fp_test(inf_val, normal1, nan_val, zero);
    
    /* Test with zero and negative */
    result2 += fp_test2(zero, neg);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 + result2;
    
    /* Print to ensure side effects */
    __builtin_printf("Result: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}
