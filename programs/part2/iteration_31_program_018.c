/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386 backend coverage testing.
 */

/* Prevent inlining to keep control flow explicit */
#define NOINLINE __attribute__((noinline))

/* Compiler barrier to prevent optimization */
#define BARRIER() asm volatile("" : : : "memory")

/* Global volatile counter to prevent dead code elimination */
volatile int global_counter = 0;

/* Helper to create NaN */
static volatile double make_nan(void) {
    return 0.0 / 0.0;
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int local_counter = 0;
    
    /* Create a loop to generate more RTL instructions */
    for (int i = 0; i < 10; i++) {
        /* 1. UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            local_counter++;
        }
        
        if (a != a) {  /* Classic NaN check - often generates unordered */
            local_counter += 2;
        }
        
        BARRIER();
        
        /* 2. ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(c, d)) {
            local_counter += 3;
        }
        
        /* 3. UNEQ (unordered or equal) - through explicit checks */
        if (__builtin_isunordered(a, c) || a == c) {
            local_counter += 4;
        }
        
        BARRIER();
        
        /* 4. UNGE (unordered or greater-or-equal) */
        if (__builtin_isunordered(b, d) || b >= d) {
            local_counter += 5;
        }
        
        /* 5. UNGT (unordered or greater-than) */
        if (__builtin_isunordered(a, d) || a > d) {
            local_counter += 6;
        }
        
        BARRIER();
        
        /* 6. UNLE (unordered or less-or-equal) */
        if (__builtin_isunordered(c, b) || c <= b) {
            local_counter += 7;
        }
        
        /* 7. UNLT (unordered or less-than) */
        if (__builtin_isunordered(d, a) || d < a) {
            local_counter += 8;
        }
        
        BARRIER();
        
        /* 8. LTGT (less-than or greater-than, but not equal/unordered) */
        /* This is tricky - we need (a < b || a > b) but both false when unordered */
        if ((a < b || a > b) && !__builtin_isunordered(a, b)) {
            local_counter += 9;
        }
        
        /* Mix in some regular comparisons */
        if (b == c) {
            local_counter += 10;
        }
        
        if (d < a) {
            local_counter += 11;
        }
        
        BARRIER();
        
        /* Use inline asm to create data dependencies */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
    }
    
    return local_counter;
}

int main(void) {
    /* Initialize with various values including NaN */
    volatile double nan_val = make_nan();
    volatile double inf_val = 1.0 / 0.0;
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    volatile double neg = -1.5;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs normal */
    result += fp_test(nan_val, normal1, normal2, zero);
    
    /* Test 2: normal vs normal */
    result += fp_test(normal1, normal2, zero, neg);
    
    /* Test 3: NaN vs NaN */
    result += fp_test(nan_val, nan_val, normal1, inf_val);
    
    /* Test 4: Infinity vs normal */
    result += fp_test(inf_val, normal1, neg, nan_val);
    
    /* Test 5: Zero vs negative */
    result += fp_test(zero, neg, inf_val, normal2);
    
    /* Use result to prevent optimization */
    global_counter = result;
    
    /* Print to ensure code isn't eliminated */
    if (global_counter > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Should never happen */
    }
}
