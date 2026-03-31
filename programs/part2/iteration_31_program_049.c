/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386 x87 floating-point backend coverage.
 */

/* Prevent inlining to ensure function boundaries in RTL */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN values */
static volatile double make_nan(void) {
    return 0.0 / 0.0;
}

/* Helper to create infinity */
static volatile double make_inf(void) {
    return 1.0 / 0.0;
}

/* Core function with floating-point comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create multiple basic blocks with FP comparisons */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            counter++;
            /* Use inline asm to prevent optimization */
            asm volatile("" : "+g"(counter) : : "memory");
        }
        
        /* ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(a, c)) {
            counter += 2;
        }
        
        /* Direct NaN checks - may generate UNORDERED */
        if (a != a) {  /* NaN check */
            counter += 3;
        }
        
        /* UNEQ (unordered or equal) - via == comparison */
        if (a == b) {
            counter += 4;
        }
        
        /* UNGE (unordered or greater or equal) - via >= comparison */
        if (a >= b) {
            counter += 5;
        }
        
        /* UNGT (unordered or greater) - via > comparison */
        if (a > b) {
            counter += 6;
        }
        
        /* UNLE (unordered or less or equal) - via <= comparison */
        if (a <= b) {
            counter += 7;
        }
        
        /* UNLT (unordered or less) - via < comparison */
        if (a < b) {
            counter += 8;
        }
        
        /* LTGT (less or greater) - via != comparison */
        if (a != b) {
            counter += 9;
        }
        
        /* Mix with ordered comparisons on different variables */
        if (c < d) {
            counter += 10;
        }
        
        if (c > d) {
            counter += 11;
        }
        
        /* Use inline assembly to modify values unpredictably */
        asm volatile("" : "+g"(a), "+g"(b) : : "memory");
        
        /* Compiler barrier */
        asm volatile("" : : : "memory");
    }
    
    return counter;
}

int main(void) {
    /* Initialize volatile doubles with special values */
    volatile double nan1 = make_nan();
    volatile double nan2 = __builtin_nan("");
    volatile double inf_pos = make_inf();
    volatile double inf_neg = -make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    int result = 0;
    
    /* Test various combinations to trigger different condition codes */
    result += fp_test(nan1, normal1, normal2, zero);
    result += fp_test(inf_pos, inf_neg, nan1, normal1);
    result += fp_test(zero, nan2, inf_pos, inf_neg);
    result += fp_test(normal1, normal2, zero, nan1);
    
    /* Use result to prevent dead code elimination */
    volatile int use_result = result;
    asm volatile("" : "+g"(use_result));
    
    return use_result > 0 ? 0 : 1;
}
