/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep function boundaries clear */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN values */
static volatile double make_nan(void) {
    return 0.0 / 0.0;  /* Produces a quiet NaN */
}

/* Helper to create infinity */
static volatile double make_inf(void) {
    return 1.0 / 0.0;  /* Produces +Inf */
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* 1. UNORDERED checks (should generate UNORDERED condition code) */
        if (__builtin_isunordered(a, b)) {
            counter++;  /* Side effect to keep branch */
        }
        
        /* 2. ORDERED checks (should generate ORDERED condition code) */
        if (!__builtin_isunordered(c, d)) {
            counter += 2;
        }
        
        /* 3. Direct NaN checks (may generate UNORDERED) */
        if (a != a) {  /* NaN != NaN is true */
            counter += 3;
        }
        
        /* 4. Regular comparisons (generate standard condition codes) */
        if (c < d) {
            counter += 4;
        }
        
        /* 5. Equality with NaN operand (may generate UNEQ) */
        if (__builtin_isnan(a) || a == b) {
            counter += 5;
        }
        
        /* 6. Greater-than with potential NaN (may generate UNGT/UNGE) */
        if (c > d || __builtin_isunordered(c, d)) {
            counter += 6;
        }
        
        /* 7. Less-than-or-equal with NaN (may generate UNLE/UNLT) */
        if (a <= b) {
            counter += 7;
        }
        
        /* 8. Not-equal with ordered check (may generate LTGT) */
        if (c != d && !__builtin_isunordered(c, d)) {
            counter += 8;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to prevent constant folding */
        a += 0.1;
        b -= 0.1;
        c *= 1.01;
        d /= 1.01;
    }
    
    return counter;
}

int main(void) {
    /* Initialize with various special FP values */
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double inf1 = make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs NaN */
    result += fp_test(nan1, nan2, nan1, nan2);
    
    /* Test 2: NaN vs normal */
    result += fp_test(nan1, normal1, normal2, nan2);
    
    /* Test 3: Inf vs normal */
    result += fp_test(inf1, normal1, normal2, inf1);
    
    /* Test 4: Normal vs normal */
    result += fp_test(normal1, normal2, zero, normal1);
    
    /* Test 5: Inf vs NaN */
    result += fp_test(inf1, nan1, nan2, inf1);
    
    /* Use result to prevent dead code elimination */
    if (result > 1000) {
        /* This should never happen, but prevents optimization */
        return 1;
    }
    
    return 0;
}
