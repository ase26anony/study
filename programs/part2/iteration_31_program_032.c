/* test_i386_condition_codes.c
 * Designed to generate RTL with various floating-point condition codes
 * for i386 x87 FPU backend coverage.
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

/* Function with various FP comparisons to generate different condition codes */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED condition: check if either operand is NaN */
        if (__builtin_isunordered(a, b)) {
            counter++;
            /* Use inline asm to prevent optimization */
            asm volatile("" : "+g"(counter) : : "memory");
        }
        
        /* ORDERED condition: check if both operands are NOT NaN */
        if (!__builtin_isunordered(a, c)) {
            counter += 2;
        }
        
        /* UNEQ: unordered or equal (a == b, including NaN == NaN?) */
        /* Actually, UNEQ is "unordered or equal" - not equal when ordered */
        /* We'll generate this through specific comparisons */
        
        /* UNGE: unordered or greater than or equal (not less than when ordered) */
        if (!(a < b)) {  /* This can generate UNGE */
            counter += 3;
        }
        
        /* UNGT: unordered or greater than (not less than or equal) */
        if (!(a <= b)) {  /* This can generate UNGT */
            counter += 4;
        }
        
        /* UNLE: unordered or less than or equal */
        if (!(a > b)) {  /* This can generate UNLE */
            counter += 5;
        }
        
        /* UNLT: unordered or less than */
        if (!(a >= b)) {  /* This can generate UNLT */
            counter += 6;
        }
        
        /* LTGT: less than or greater than (ordered and not equal) */
        if (a != b && !__builtin_isunordered(a, b)) {
            counter += 7;
        }
        
        /* Standard comparisons that might generate different patterns */
        if (a == c) {
            counter += 8;
        }
        
        if (a < d) {
            counter += 9;
        }
        
        if (a > b) {
            counter += 10;
        }
        
        /* Check for NaN explicitly */
        if (__builtin_isnan(a)) {
            counter += 11;
        }
        
        /* Check for finite values */
        if (__builtin_isfinite(b)) {
            counter += 12;
        }
        
        /* Mix in some integer operations to create more complex CFG */
        asm volatile("" : : "r"(counter) : "memory");
        
        /* Modify values slightly to prevent complete loop unrolling */
        a += 0.1;
        b -= 0.1;
    }
    
    return counter;
}

int main(void) {
    /* Use volatile to prevent constant folding */
    volatile double nan_val = make_nan();
    volatile double inf_val = make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Call the test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs normal */
    result += fp_test(nan_val, normal1, normal2, zero);
    
    /* Test 2: normal vs NaN */
    result += fp_test(normal1, nan_val, inf_val, normal2);
    
    /* Test 3: Inf vs normal */
    result += fp_test(inf_val, normal1, nan_val, zero);
    
    /* Test 4: normal vs Inf */
    result += fp_test(normal1, inf_val, normal2, nan_val);
    
    /* Test 5: zero vs zero (with NaN in other positions) */
    result += fp_test(zero, zero, nan_val, inf_val);
    
    /* Use the result to prevent dead code elimination */
    volatile int use_result = result;
    
    /* Print to ensure side effects */
    asm volatile("" : : "r"(use_result) : "memory");
    
    return use_result > 0 ? 0 : 1;
}
