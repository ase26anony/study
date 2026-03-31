/* test_i386_condition_codes.c
 * Designed to trigger UNORDERED, ORDERED, and other FP condition codes
 * in GCC's i386 RTL output printer.
 */

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN values */
static volatile double make_nan(void) {
    return 0.0 / 0.0;  /* Produces a quiet NaN */
}

/* Helper to create infinity */
static volatile double make_inf(void) {
    return 1.0 / 0.0;  /* Produces infinity */
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create multiple RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        if (a != a) {  /* Classic NaN check - often generates UNORDERED */
            counter++;
        }
        
        /* ORDERED checks */
        if (__builtin_isless(a, b)) {  /* a < b, both ordered */
            counter++;
        }
        
        if (!__builtin_isunordered(c, d)) {  /* ORDERED check */
            counter++;
        }
        
        /* UNEQ (unordered or equal) - via == comparison with NaN */
        if (__builtin_isnan(a) || a == b) {
            counter++;
        }
        
        /* UNGE (not less than) - via !(a < b) with possible NaN */
        if (!(a < b)) {
            counter++;
        }
        
        /* UNGT (not less than or equal) */
        if (!(a <= b)) {
            counter++;
        }
        
        /* UNLE (unordered or less than or equal) */
        if (__builtin_isunordered(a, b) || a <= b) {
            counter++;
        }
        
        /* UNLT (unordered or less than) */
        if (__builtin_isunordered(a, b) || a < b) {
            counter++;
        }
        
        /* LTGT (less than or greater than, but not equal/unordered) */
        if (a < b || a > b) {
            counter++;
        }
        
        /* Use inline assembly as compiler barrier and to create
         * data dependencies that keep comparisons live */
        asm volatile("" : "+g"(a), "+g"(b) : : "memory");
        
        /* Mix in some integer operations to prevent pure FP optimization */
        counter += (i & 1);
    }
    
    return counter;
}

int main(void) {
    /* Initialize volatile doubles with various special values */
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double inf_pos = make_inf();
    volatile double inf_neg = -make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    /* Test 1: NaN vs NaN */
    result += fp_test(nan1, nan2, normal1, normal2);
    
    /* Test 2: NaN vs normal */
    result += fp_test(nan1, normal1, normal2, zero);
    
    /* Test 3: Normal vs normal */
    result += fp_test(normal1, normal2, inf_pos, inf_neg);
    
    /* Test 4: Infinity vs normal */
    result += fp_test(inf_pos, normal1, inf_neg, normal2);
    
    /* Test 5: Infinity vs infinity */
    result += fp_test(inf_pos, inf_neg, inf_pos, inf_pos);
    
    /* Test 6: Zero vs NaN */
    result += fp_test(zero, nan1, zero, nan2);
    
    /* Use result to prevent dead code elimination */
    if (result > 100) {
        /* This should never happen, but prevents optimization */
        asm volatile("" : : "r"(result) : "memory");
    }
    
    return result > 0 ? 0 : 1;
}
