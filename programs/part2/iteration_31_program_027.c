/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep RTL structure intact */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN */
static double make_nan(void) {
    return 0.0 / 0.0;
}

/* Force use of x87 FPU with volatile variables */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    
    /* Loop to create multiple RTL instructions */
    for (volatile int i = 0; i < 5; i++) {
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* Another unordered check with different variables */
        if (a != a) {  /* Classic NaN check */
            counter += 2;
        }
        
        /* ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(c, d)) {
            counter += 3;
        }
        
        /* UNEQ (unordered or equal) - via explicit comparison */
        if (!(c > d) && !(c < d)) {  /* Not greater AND not less */
            counter += 4;
        }
        
        /* UNGE (unordered or greater-or-equal) */
        if (!(c < d)) {
            counter += 5;
        }
        
        /* UNGT (unordered or greater) */
        if (!(c <= d)) {
            counter += 6;
        }
        
        /* UNLE (unordered or less-or-equal) */
        if (!(c > d)) {
            counter += 7;
        }
        
        /* UNLT (unordered or less) */
        if (!(c >= d)) {
            counter += 8;
        }
        
        /* LTGT (less or greater, but not equal/unordered) */
        if (c < d || c > d) {
            counter += 9;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify variables to create data dependencies */
        a = b + 1.0;
        b = c * 2.0;
        c = d - 1.0;
        d = a / 2.0;
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN and normal values */
    volatile double nan_val = make_nan();
    volatile double inf_val = 1.0 / 0.0;
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs normal */
    result += fp_test(nan_val, normal1, normal2, nan_val);
    
    /* Test 2: normal vs normal */
    result += fp_test(normal1, normal2, normal1, normal2);
    
    /* Test 3: NaN vs NaN */
    result += fp_test(nan_val, nan_val, nan_val, nan_val);
    
    /* Test 4: Infinity vs normal */
    result += fp_test(inf_val, normal1, normal2, inf_val);
    
    /* Use result to prevent dead code elimination */
    volatile int use_result = result;
    
    /* Print to ensure side effect */
    if (use_result > 0) {
        return 0;
    }
    return 1;
}
