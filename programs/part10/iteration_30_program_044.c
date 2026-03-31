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
    double d1 = vd1;
    double d2 = vd_nan;
    float f1 = vf1;
    float f2 = vf_nan;
    
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d2)) result |= 1;
    if (__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Using NaN property */
    if (d1 != d1 || d2 != d2) result |= 4;
    if (f1 != f1 || f2 != f2) result |= 8;
    
    /* Mixed types */
    if (__builtin_isunordered(d1, f2)) result |= 16;
    
    return result;
}

/* Test ORDERED condition code */
int test_ordered(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    
    int result = 0;
    
    /* Ordered checks */
    if (__builtin_isordered(d1, d2)) result |= 1;
    if (__builtin_isordered(d1, d_nan)) result |= 2;
    
    /* Using !unordered */
    if (!__builtin_isunordered(d1, d2)) result |= 4;
    
    /* With constants */
    if (__builtin_isordered(1.0, 2.0)) result |= 8;
    if (__builtin_isordered(1.0, NAN)) result |= 16;
    
    return result;
}

/* Test UNEQ condition code (unordered or equal) */
int test_uneq(void) {
    double d1 = vd1;
    double d2 = vd1;  /* Same value */
    double d_nan = make_nan();
    
    int result = 0;
    
    /* This should generate UNEQ when optimized */
    if (!(d1 > d2) && !(d1 < d2)) result |= 1;
    
    /* With NaN */
    if (!(d1 > d_nan) && !(d1 < d_nan)) result |= 2;
    
    /* Using == with potential NaN */
    if (d1 == d2) result |= 4;
    
    /* Float version */
    float f1 = vf1;
    float f2 = vf1;
    if (!(f1 > f2) && !(f1 < f2)) result |= 8;
    
    return result;
}

/* Test UNGE condition code (unordered or greater or equal) - generates "nlt" */
int test_unge(void) {
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d_nan = make_nan();
    
    int result = 0;
    
    /* Inverse of less than */
    if (!(d1 < d2)) result |= 1;      /* Should generate UNGE/nlt */
    
    /* With NaN operand */
    if (!(d_nan < d2)) result |= 2;
    
    /* Using >= with potential NaN */
    if (d1 >= d2) result |= 4;
    
    /* Float version */
    float f1 = vf2;
    float f2 = vf1;
    if (!(f1 < f2)) result |= 8;
    
    return result;
}

/* Test UNGT condition code (unordered or greater) - generates "nle" */
int test_ungt(void) {
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d_nan = make_nan();
    
    int result = 0;
    
    /* Inverse of less than or equal */
    if (!(d1 <= d2)) result |= 1;     /* Should generate UNGT/nle */
    
    /* With NaN operand */
    if (!(d_nan <= d2)) result |= 2;
    
    /* Using > with potential NaN */
    if (d1 > d2) result |= 4;
    
    /* Mixed float/double */
    if (!(d1 <= vf1)) result |= 8;
    
    return result;
}

/* Test UNLE condition code (unordered or less or equal) - generates "ule" */
int test_unle(void) {
    double d1 = vd1;  /* 1.0 */
    double d2 = vd2;  /* 2.0 */
    double d_nan = make_nan();
    
    int result = 0;
    
    /* This pattern should generate UNLE */
    if (!(d1 > d2)) result |= 1;
    
    /* With NaN */
    if (!(d_nan > d2)) result |= 2;
    
    /* Using <= with potential NaN */
    if (d1 <= d2) result |= 4;
    
    /* Function return comparison */
    if (!(sin(d1) > cos(d2))) result |= 8;
    
    return result;
}

/* Test UNLT condition code (unordered or less than) - generates "ult" */
int test_unlt(void) {
    double d1 = vd1;  /* 1.0 */
    double d2 = vd2;  /* 2.0 */
    double d_nan = make_nan();
    
    int result = 0;
    
    /* This pattern should generate UNLT */
    if (!(d1 >= d2)) result |= 1;
    
    /* With NaN */
    if (!(d_nan >= d2)) result |= 2;
    
    /* Using < with potential NaN */
    if (d1 < d2) result |= 4;
    
    /* Volatile comparison */
    volatile double v1 = d1;
    volatile double v2 = d2;
    if (!(v1 >= v2)) result |= 8;
    
    return result;
}

/* Test LTGT condition code (less or greater) - generates "une" */
int test_ltgt(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    
    int result = 0;
    
    /* Direct lessgreater builtin */
    if (__builtin_islessgreater(d1, d2)) result |= 1;
    
    /* Manual equivalent */
    if ((d1 < d2) || (d1 > d2)) result |= 2;
    
    /* With NaN - should be false */
    if (__builtin_islessgreater(d1, d_nan)) result |= 4;
    
    /* Float version */
    if (__builtin_islessgreater(vf1, vf2)) result |= 8;
    
    /* Ordered inequality */
    if (d1 != d2 && __builtin_isordered(d1, d2)) result |= 16;
    
    return result;
}

/* Test mixed conditions in control flow */
int test_control_flow(void) {
    double a = vd1;
    double b = vd2;
    double nan = make_nan();
    int result = 0;
    
    /* Complex if-else chain with various conditions */
    if (__builtin_isunordered(a, nan)) {
        result = 1;
    } else if (!(a < b)) {  /* UNGE */
        result = 2;
    } else if (!(b <= a)) { /* UNGT */
        result = 3;
    } else if (__builtin_islessgreater(a, b)) { /* LTGT */
        result = 4;
    }
    
    /* Ternary operator with unordered check */
    result += __builtin_isunordered(b, nan) ? 10 : 20;
    
    /* While loop with ordered check */
    int count = 0;
    while (count < 3 && __builtin_isordered(a + count, b)) {
        result += 100;
        count++;
    }
    
    return result;
}

/* Test with array indexing based on comparisons */
int test_array_indexing(void) {
    static const int values[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    double x = vd1;
    double y = vd2;
    double nan = make_nan();
    
    int idx = 0;
    
    /* Build index based on comparison results */
    if (__builtin_isunordered(x, nan)) idx |= 1;
    if (!(x < y)) idx |= 2;          /* UNGE */
    if (__builtin_islessgreater(x, y)) idx |= 4; /* LTGT */
    
    /* Ensure idx is in bounds */
    idx = idx & 7;
    
    return values[idx];
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Call all test functions */
    checksum += test_unordered();
    printf("test_unordered: %d\n", test_unordered());
    
    checksum += test_ordered();
    printf("test_ordered: %d\n", test_ordered());
    
    checksum += test_uneq();
    printf("test_uneq: %d\n", test_uneq());
    
    checksum += test_unge();
    printf("test_unge: %d\n", test_unge());
    
    checksum += test_ungt();
    printf("test_ungt: %d\n", test_ungt());
    
    checksum += test_unle();
    printf("test_unle: %d\n", test_unle());
    
    checksum += test_unlt();
    printf("test_unlt: %d\n", test_unlt());
    
    checksum += test_ltgt();
    printf("test_ltgt: %d\n", test_ltgt());
    
    checksum += test_control_flow();
    printf("test_control_flow: %d\n", test_control_flow());
    
    checksum += test_array_indexing();
    printf("test_array_indexing: %d\n", test_array_indexing());
    
    printf("Total checksum: %d\n", checksum);
    
    /* Return non-zero if any test failed (simplistic check) */
    return checksum == 0 ? 1 : 0;
}
