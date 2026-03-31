/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep RTL structure intact */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN values */
static volatile double make_nan(void) {
    return 0.0 / 0.0;
}

/* Helper to create infinity */
static volatile double make_inf(void) {
    return 1.0 / 0.0;
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks - should generate UNORDERED condition code */
        if (__builtin_isunordered(a, b)) {
            counter++;
            asm volatile("" : : : "memory"); /* Barrier */
        }
        
        /* ORDERED checks - should generate ORDERED condition code */
        if (!__builtin_isunordered(a, c)) {
            counter += 2;
            asm volatile("" : : : "memory");
        }
        
        /* Direct unordered comparison (a != a) */
        if (a != a) {
            counter += 3;
        }
        
        /* UNEQ: unordered or equal */
        if (!(a < b) && !(a > b)) {  /* a == b or unordered */
            counter += 4;
        }
        
        /* UNGE: unordered or greater-or-equal */
        if (!(a < b)) {
            counter += 5;
        }
        
        /* UNGT: unordered or greater */
        if (!(a <= b)) {
            counter += 6;
        }
        
        /* UNLE: unordered or less-or-equal */
        if (!(a > b)) {
            counter += 7;
        }
        
        /* UNLT: unordered or less */
        if (!(a >= b)) {
            counter += 8;
        }
        
        /* LTGT: less or greater (but not equal, not unordered) */
        if ((a < b) || (a > b)) {
            counter += 9;
        }
        
        /* Mix with regular comparisons */
        if (c < d) {
            counter += 10;
        }
        
        if (c == d) {
            counter += 11;
        }
        
        if (c > d) {
            counter += 12;
        }
        
        /* Use inline asm to modify values unpredictably */
        asm volatile("" : "+g"(a), "+g"(b) : : "memory");
        
        /* Check with isnan builtin */
        if (__builtin_isnan(a)) {
            counter += 13;
        }
        
        /* Check with isinf builtin */
        if (__builtin_isinf(c)) {
            counter += 14;
        }
        
        /* Complex expression mixing ordered/unordered */
        if ((a < b) || __builtin_isunordered(a, d)) {
            counter += 15;
        }
    }
    
    return counter;
}

int main(void) {
    /* Initialize with various FP values */
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double inf1 = make_inf();
    volatile double inf2 = -make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    volatile double neg_zero = -0.0;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    
    result += fp_test(nan1, normal1, inf1, normal2);
    result += fp_test(normal1, nan2, normal2, inf2);
    result += fp_test(inf1, inf2, nan1, zero);
    result += fp_test(zero, neg_zero, normal1, nan2);
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(result));
    
    return result > 0 ? 0 : 1;
}
