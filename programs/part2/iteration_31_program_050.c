/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage testing.
 */

/* Prevent inlining to keep RTL structure intact */
#define NOINLINE __attribute__((noinline))

/* Compiler barrier to prevent optimization */
#define BARRIER() asm volatile("" : : : "memory")

/* Global volatile counter to prevent dead code elimination */
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
        BARRIER();
        
        /* 1. UNORDERED checks (should generate UNORDERED condition code) */
        if (__builtin_isunordered(a, b)) {
            local_counter += 1;  /* Side effect to keep branch */
        }
        
        if (a != a) {  /* Classic NaN check - often generates unordered */
            local_counter += 2;
        }
        
        /* 2. ORDERED checks */
        if (__builtin_isless(a, b)) {  /* Ordered less-than */
            local_counter += 3;
        }
        
        if (__builtin_isgreater(c, d)) {  /* Ordered greater-than */
            local_counter += 4;
        }
        
        /* 3. Mixed ordered/unordered comparisons */
        if (!__builtin_isunordered(a, c)) {  /* Ordered */
            local_counter += 5;
        }
        
        /* 4. Equality comparisons that may generate UNEQ */
        if (__builtin_islessequal(a, b)) {
            local_counter += 6;
        }
        
        if (__builtin_isgreaterequal(c, d)) {
            local_counter += 7;
        }
        
        /* 5. Direct comparisons that may generate various condition codes */
        if (a < b) {  /* LT */
            local_counter += 8;
        }
        
        if (c > d) {  /* GT */
            local_counter += 9;
        }
        
        if (a == b) {  /* EQ */
            local_counter += 10;
        }
        
        /* 6. UNORDERED or ORDERED with builtins */
        if (__builtin_isnan(a)) {
            local_counter += 11;
        }
        
        /* 7. Complex expression that might generate LTGT */
        if ((a < b) || (a > b)) {  /* Not equal, but ordered */
            local_counter += 12;
        }
        
        BARRIER();
        
        /* Modify values slightly to prevent complete optimization */
        a += 0.1;
        b -= 0.1;
        c *= 1.01;
        d /= 1.01;
    }
    
    return local_counter;
}

int main(void) {
    /* Initialize with volatile to prevent constant folding */
    volatile double nan_val = make_nan();
    volatile double inf_val = 1.0 / 0.0;
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs normal */
    result += fp_test(nan_val, normal1, normal2, zero);
    
    /* Test 2: NaN vs NaN */
    result += fp_test(nan_val, nan_val, normal1, normal2);
    
    /* Test 3: Infinity vs normal */
    result += fp_test(inf_val, normal1, normal2, inf_val);
    
    /* Test 4: Normal vs normal */
    result += fp_test(normal1, normal2, zero, inf_val);
    
    /* Test 5: Zero vs NaN */
    result += fp_test(zero, nan_val, inf_val, normal1);
    
    /* Use result to prevent optimization */
    counter = result;
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result > 0 ? 0 : 1;
}
