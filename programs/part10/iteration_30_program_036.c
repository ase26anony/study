/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = 0.0/0.0;
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = 0.0f/0.0f;

/* Function to generate NaN */
static double make_nan(void) {
    return __builtin_nan("");
}

static float make_nanf(void) {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code */
int test_unordered(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd_nan;
    float f1 = vf1;
    float f2 = vf_nan;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d2)) result |= 1;
    if (__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Using NaN property */
    if (d2 != d2) result |= 4;  /* NaN check */
    
    /* Mixed types */
    if (__builtin_isunordered(d1, f2)) result |= 8;
    
    return result;
}

/* Test ORDERED condition code */
int test_ordered(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd2;
    float f1 = vf1;
    float f2 = vf2;
    
    /* Ordered checks */
    if (!__builtin_isunordered(d1, d2)) result |= 1;
    if (!__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Compare with function return */
    double d3 = sqrt(4.0);
    if (!__builtin_isunordered(d1, d3)) result |= 4;
    
    return result;
}

/* Test UNEQ (unordered or equal) */
int test_uneq(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd1;  /* Same value */
    double d_nan = make_nan();
    
    /* Using builtin */
    if (__builtin_isunordered(d1, d_nan) || d1 == d2) result |= 1;
    
    /* Complex expression that might generate UNEQ */
    float f1 = vf1;
    float f2 = vf1;
    if (!(f1 < f2) && !(f1 > f2)) result |= 2;  /* Equal or unordered */
    
    return result;
}

/* Test UNGE (not less than) - unordered or greater than or equal */
int test_unge(void) {
    int result = 0;
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d_nan = make_nan();
    
    /* Inverse condition */
    if (!(d1 < d2)) result |= 1;  /* Should generate nlt */
    
    /* With NaN */
    if (!(d_nan < d1)) result |= 2;
    
    /* Float version */
    float f1 = vf2;
    float f2 = vf1;
    if (!(f1 < f2)) result |= 4;
    
    return result;
}

/* Test UNGT (not less than or equal) - unordered or greater than */
int test_ungt(void) {
    int result = 0;
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d_nan = make_nan();
    
    /* Inverse condition */
    if (!(d1 <= d2)) result |= 1;  /* Should generate nle */
    
    /* With NaN */
    if (!(d_nan <= d1)) result |= 2;
    
    /* Mixed precision */
    float f1 = vf2;
    double d3 = vd1;
    if (!(f1 <= d3)) result |= 4;
    
    return result;
}

/* Test UNLE (unordered or less than or equal) */
int test_unle(void) {
    int result = 0;
    double d1 = vd1;  /* 1.0 */
    double d2 = vd2;  /* 2.0 */
    double d_nan = make_nan();
    
    /* Direct comparison that might generate ule */
    if (d1 <= d2) result |= 1;
    
    /* With NaN - should be true */
    if (d_nan <= d1) result |= 2;
    
    /* Using volatile */
    volatile double v1 = 1.5;
    volatile double v2 = 2.5;
    if (v1 <= v2) result |= 4;
    
    return result;
}

/* Test UNLT (unordered or less than) */
int test_unlt(void) {
    int result = 0;
    double d1 = vd1;  /* 1.0 */
    double d2 = vd2;  /* 2.0 */
    double d_nan = make_nan();
    
    /* Direct comparison that might generate ult */
    if (d1 < d2) result |= 1;
    
    /* With NaN - should be true */
    if (d_nan < d1) result |= 2;
    
    /* Float comparison */
    float f1 = 0.5f;
    float f2 = 1.5f;
    if (f1 < f2) result |= 4;
    
    return result;
}

/* Test LTGT (less than or greater than - ordered and not equal) */
int test_ltgt(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    
    /* Using builtin */
    if (__builtin_islessgreater(d1, d2)) result |= 1;
    
    /* Manual version */
    if ((d1 < d2) || (d1 > d2)) result |= 2;
    
    /* With function calls */
    double d3 = sqrt(9.0);
    double d4 = sqrt(16.0);
    if (__builtin_islessgreater(d3, d4)) result |= 4;
    
    /* This should NOT trigger with NaN */
    if (!__builtin_islessgreater(d1, d_nan)) result |= 8;
    
    return result;
}

/* Test with conditional moves (might use different code generation) */
int test_cmov(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    
    /* Ternary operator that might generate cmov */
    result = (d1 < d2) ? 1 : 2;
    result += (__builtin_isunordered(d1, d_nan)) ? 4 : 8;
    
    /* Nested ternary */
    float f1 = vf1;
    float f2 = vf2;
    result += (f1 <= f2) ? 16 : 32;
    
    return result;
}

/* Test with loops */
int test_loop(void) {
    int result = 0;
    double values[] = {1.0, 2.0, make_nan(), 3.0, 4.0};
    float fvalues[] = {1.0f, 2.0f, make_nanf(), 3.0f, 4.0f};
    
    for (int i = 0; i < 4; i++) {
        /* Various comparisons in loop */
        if (__builtin_isunordered(values[i], values[i+1])) {
            result += (1 << i);
        }
        if (!(fvalues[i] < fvalues[i+1])) {  /* Might generate nlt */
            result += (1 << (i+4));
        }
    }
    
    return result;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Call all test functions */
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_cmov();
    checksum += test_loop();
    
    printf("Final checksum: %d\n", checksum);
    printf("(If this runs without crashing, comparisons were executed)\n");
    
    /* Additional forced comparisons in main */
    volatile double a = 0.0;
    volatile double b = -0.0;
    if (a == b) checksum += 1000;  /* Should be true (+0 == -0) */
    
    volatile double inf = 1.0 / 0.0;
    if (inf > 1e308) checksum += 2000;
    
    /* Test with isless, isgreater which are ordered comparisons */
    if (isless(vd1, vd2)) checksum += 3000;
    if (isgreater(vd2, vd1)) checksum += 4000;
    
    printf("Final value: %d\n", checksum);
    
    return 0;
}
