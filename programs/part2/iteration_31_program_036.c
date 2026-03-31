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

/* Core function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* 1. UNORDERED checks - using various methods */
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        if (a != a) {  /* Classic NaN check */
            counter += 2;
        }
        
        if (__builtin_isnan(c)) {
            counter += 3;
        }
        
        /* 2. ORDERED checks */
        if (!__builtin_isunordered(a, b)) {
            counter += 4;
        }
        
        /* 3. Standard comparisons that might generate UNEQ, UNGE, etc. */
        if (a == b) {
            counter += 5;
        }
        
        if (a < b) {
            counter += 6;
        }
        
        if (a > b) {
            counter += 7;
        }
        
        if (a <= b) {
            counter += 8;
        }
        
        if (a >= b) {
            counter += 9;
        }
        
        /* 4. More complex comparisons with different operands */
        if (__builtin_isunordered(c, d)) {
            counter += 10;
        }
        
        if (c != d) {
            counter += 11;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to prevent constant folding */
        asm volatile("" : "+g"(a), "+g"(b));
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
    volatile double neg_inf = -1.0 / 0.0;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    result += fp_test(nan_val, normal1, inf_val, zero);
    result += fp_test(normal1, nan_val, zero, inf_val);
    result += fp_test(inf_val, neg_inf, nan_val, normal2);
    result += fp_test(zero, zero, normal1, normal2);
    
    /* Use result to prevent dead code elimination */
    volatile int use_result = result;
    
    /* Simple output to ensure program runs */
    if (use_result > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Shouldn't happen */
    }
}
