/* test_i386_condition_codes.c
 * Generates RTL with various floating-point condition codes for i386/x87
 * Compile with: gcc -m32 -mfpmath=387 -O2 -da test_i386_condition_codes.c -o test
 */

/* Prevent inlining to keep RTL structure intact */
#define NOINLINE __attribute__((noinline))

/* Global volatile counter to prevent optimization */
volatile int global_counter = 0;

/* Helper to create NaN */
static double make_nan(void) {
    return 0.0 / 0.0;
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int local_counter = 0;
    
    /* Loop to generate more RTL instructions */
    for (int i = 0; i < 10; i++) {
        /* 1. UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            local_counter++;
            asm volatile("" : : : "memory");  /* Barrier */
        }
        
        /* 2. ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(a, c)) {
            local_counter += 2;
        }
        
        /* 3. Direct NaN checks - may generate UNORDERED */
        if (a != a) {  /* NaN check */
            local_counter += 3;
        }
        
        /* 4. UNEQ (unordered or equal) - via __builtin_islessequal */
        if (__builtin_islessequal(a, b)) {
            local_counter += 4;
        }
        
        /* 5. UNGE (unordered or greater than or equal) */
        if (__builtin_isgreaterequal(a, c)) {
            local_counter += 5;
        }
        
        /* 6. UNGT (unordered or greater than) */
        if (__builtin_isgreater(a, d)) {
            local_counter += 6;
        }
        
        /* 7. UNLE (unordered or less than or equal) */
        if (__builtin_islessequal(b, c)) {
            local_counter += 7;
        }
        
        /* 8. UNLT (unordered or less than) */
        if (__builtin_isless(b, d)) {
            local_counter += 8;
        }
        
        /* 9. LTGT (less than or greater than, but not equal/unordered) */
        if (__builtin_islessgreater(c, d)) {
            local_counter += 9;
        }
        
        /* 10. Mixed ordered comparisons */
        if (c < d) {
            local_counter += 10;
        }
        
        if (c > d) {
            local_counter += 11;
        }
        
        if (c == d) {
            local_counter += 12;
        }
        
        /* Modify values slightly to prevent complete optimization */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
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
    int result = 0;
    
    /* Test 1: NaN vs normal */
    result += fp_test(nan_val, normal1, normal2, zero);
    
    /* Test 2: normal vs NaN */
    result += fp_test(normal1, nan_val, zero, inf_val);
    
    /* Test 3: NaN vs NaN */
    result += fp_test(nan_val, nan_val, normal1, normal2);
    
    /* Test 4: Infinity vs normal */
    result += fp_test(inf_val, normal1, nan_val, zero);
    
    /* Test 5: Normal values */
    result += fp_test(normal1, normal2, zero, inf_val);
    
    /* Store result to prevent dead code elimination */
    global_counter = result;
    
    /* Use result to prevent optimization */
    if (global_counter > 0) {
        return 0;
    } else {
        return 1;
    }
}
