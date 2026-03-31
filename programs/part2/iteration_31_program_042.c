/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep RTL structure */
#define NOINLINE __attribute__((noinline))

/* Global volatile counter to prevent optimization */
volatile int counter = 0;

/* Helper to create NaN */
static double make_nan(void) {
    return 0.0 / 0.0;
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    int local_counter = 0;
    
    /* Loop to create more RTL instructions */
    for (int i = 0; i < 10; i++) {
        /* UNORDERED checks */
        if (__builtin_isunordered(a, b)) {
            local_counter++;
        }
        
        /* ORDERED checks */
        if (!__builtin_isunordered(a, c)) {
            local_counter += 2;
        }
        
        /* Direct NaN checks that may generate UNORDERED */
        if (a != a) {  /* NaN check */
            local_counter += 3;
        }
        
        /* UNEQ (unordered or equal) - a == b with NaN possibility */
        if (a == b) {
            local_counter += 4;
        }
        
        /* UNGE (!(a < b)) with NaN */
        if (!(a < b)) {
            local_counter += 5;
        }
        
        /* UNGT (!(a <= b)) with NaN */
        if (!(a <= b)) {
            local_counter += 6;
        }
        
        /* UNLE (unordered or less or equal) */
        if (a <= b) {
            local_counter += 7;
        }
        
        /* UNLT (unordered or less than) */
        if (a < b) {
            local_counter += 8;
        }
        
        /* LTGT (less than or greater than, but not equal/unordered) */
        if (a < b || a > b) {
            local_counter += 9;
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

int main(void) {
    /* Initialize volatile doubles with various values */
    volatile double nan_val = make_nan();
    volatile double inf_val = 1.0 / 0.0;
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Call test function multiple times with different combinations */
    counter += fp_test(nan_val, normal1, normal2, inf_val);
    counter += fp_test(normal1, nan_val, inf_val, zero);
    counter += fp_test(inf_val, zero, nan_val, normal1);
    counter += fp_test(zero, inf_val, normal1, nan_val);
    
    /* Use the result to prevent dead code elimination */
    if (counter > 100) {
        __builtin_printf("Result: %d\n", counter);
    } else {
        __builtin_printf("Fallback: %d\n", counter);
    }
    
    return 0;
}
