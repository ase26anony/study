/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep RTL structure intact */
#define NOINLINE __attribute__((noinline))

/* Compiler barrier to prevent optimization */
#define BARRIER() asm volatile("" : : : "memory")

/* Global volatile counter to prevent dead code elimination */
volatile int global_counter = 0;

/* Helper to create NaN */
static double make_nan(void) {
    return 0.0 / 0.0;
}

/* Helper to create infinity */
static double make_inf(void) {
    return 1.0 / 0.0;
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int local_counter = 0;
    
    /* Create a loop to generate more RTL instructions */
    for (int i = 0; i < 5; i++) {
        BARRIER();
        
        /* 1. UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            local_counter++;
        }
        
        if (a != a) {  /* Classic NaN check - often generates unordered */
            local_counter += 2;
        }
        
        /* 2. ORDERED checks */
        if (!__builtin_isunordered(c, d)) {
            local_counter += 3;
        }
        
        /* 3. UNEQ (unordered or equal) - using isnan OR equality */
        if (__builtin_isnan(a) || a == b) {
            local_counter += 4;
        }
        
        /* 4. UNGE (unordered or greater-or-equal) */
        if (__builtin_isunordered(a, c) || a >= c) {
            local_counter += 5;
        }
        
        /* 5. UNGT (unordered or greater-than) */
        if (__builtin_isunordered(b, d) || b > d) {
            local_counter += 6;
        }
        
        /* 6. UNLE (unordered or less-or-equal) */
        if (__builtin_isunordered(c, a) || c <= a) {
            local_counter += 7;
        }
        
        /* 7. UNLT (unordered or less-than) */
        if (__builtin_isunordered(d, b) || d < b) {
            local_counter += 8;
        }
        
        /* 8. LTGT (less-than or greater-than, but not equal/unordered) */
        if ((a < b) || (a > b)) {
            local_counter += 9;
        }
        
        /* Standard comparisons mixed in */
        if (a == c) local_counter += 10;
        if (b != d) local_counter += 11;
        if (a < d)  local_counter += 12;
        if (c > b)  local_counter += 13;
        if (a >= c) local_counter += 14;
        if (d <= b) local_counter += 15;
        
        /* Use inline assembly to modify values unpredictably */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
        
        BARRIER();
    }
    
    return local_counter;
}

int main(void) {
    /* Initialize with various special FP values */
    volatile double nan1 = make_nan();
    volatile double nan2 = __builtin_nan("0xdead");
    volatile double inf_pos = make_inf();
    volatile double inf_neg = -make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    volatile double neg_zero = -0.0;
    
    /* Mix arguments to create various comparison scenarios */
    int result = 0;
    
    /* Test case 1: NaN vs normal */
    result += fp_test(nan1, normal1, normal2, zero);
    
    /* Test case 2: NaN vs NaN */
    result += fp_test(nan1, nan2, normal1, normal2);
    
    /* Test case 3: Inf vs normal */
    result += fp_test(inf_pos, normal1, inf_neg, normal2);
    
    /* Test case 4: Normal vs normal with zeros */
    result += fp_test(normal1, zero, neg_zero, normal2);
    
    /* Test case 5: Mixed special values */
    result += fp_test(nan1, inf_pos, inf_neg, nan2);
    
    /* Use result to prevent optimization */
    global_counter = result;
    
    /* Print to ensure code isn't eliminated */
    if (global_counter > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Should never happen */
    }
}
