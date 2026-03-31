/* test_conditions.c - Program to trigger x86 floating-point condition code generation */
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
    
    /* Using !(a == a) pattern */
    double nan_val = make_nan();
    if (!(nan_val == nan_val)) result |= 4;
    
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
    
    /* Compare normal numbers */
    double d3 = sqrt(4.0);  /* Should be 2.0 */
    if (!__builtin_isunordered(d1, d3)) result |= 4;
    
    /* With constants */
    if (!__builtin_isunordered(3.14159, 2.71828)) result |= 8;
    
    return result;
}

/* Test UNEQ (unordered or equal) */
int test_uneq(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd1;  /* Same value */
    double d_nan = make_nan();
    
    /* Using builtin */
    if (__builtin_isunordered(d1, d_nan) || d1 == d1) result |= 1;
    
    /* Pattern that might generate UNEQ */
    float f1 = vf1;
    float f_self = vf1;
    if (!(f1 != f_self) || __builtin_isunordered(f1, f_self)) result |= 2;
    
    /* With NaN on one side */
    if (__builtin_isunordered(d1, d_nan) || d1 == d1) result |= 4;
    
    return result;
}

/* Test UNGE (not less than) - unordered or greater or equal */
int test_unge(void) {
    int result = 0;
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d_nan = make_nan();
    
    /* Inverse condition: !(a < b) */
    if (!(d2 < d1)) result |= 1;      /* Should be true: 1.0 < 2.0 */
    
    /* With NaN */
    if (!(d_nan < d1)) result |= 2;   /* Should be true (unordered) */
    
    /* Float version */
    float f1 = vf2;
    float f2 = vf1;
    if (!(f2 < f1)) result |= 4;
    
    /* Mixed with constant */
    if (!(1.5 < 2.5)) result |= 8;
    
    return result;
}

/* Test UNGT (not less or equal) - unordered or greater */
int test_ungt(void) {
    int result = 0;
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d_nan = make_nan();
    
    /* Inverse condition: !(a <= b) */
    if (!(d2 <= d1)) result |= 1;      /* Should be false: 1.0 <= 2.0 */
    
    /* Test with equal values */
    if (!(d1 <= d1)) result |= 2;      /* Should be false: 2.0 <= 2.0 */
    
    /* With NaN */
    if (!(d_nan <= d1)) result |= 4;   /* Should be true (unordered) */
    
    /* Float with different values */
    float f1 = 3.0f;
    float f2 = 2.0f;
    if (!(f2 <= f1)) result |= 8;      /* Should be false */
    
    return result;
}

/* Test UNLE (unordered or less or equal) */
int test_unle(void) {
    int result = 0;
    double d1 = vd1;  /* 1.0 */
    double d2 = vd2;  /* 2.0 */
    double d_nan = make_nan();
    
    /* Direct comparison that might use UNLE */
    if (__builtin_isunordered(d1, d_nan) || d1 <= d2) result |= 1;
    
    /* With equal values */
    if (__builtin_isunordered(d1, d_nan) || d1 <= d1) result |= 2;
    
    /* Float version */
    float f1 = vf1;
    float f2 = vf2;
    if (__builtin_isunordered(f1, make_nanf()) || f1 <= f2) result |= 4;
    
    return result;
}

/* Test UNLT (unordered or less than) */
int test_unlt(void) {
    int result = 0;
    double d1 = vd1;  /* 1.0 */
    double d2 = vd2;  /* 2.0 */
    double d_nan = make_nan();
    
    /* Direct comparison */
    if (__builtin_isunordered(d1, d_nan) || d1 < d2) result |= 1;
    
    /* With NaN on right side */
    if (__builtin_isunordered(d1, d_nan) || d1 < d_nan) result |= 2;
    
    /* Float with normal values */
    float f1 = 1.5f;
    float f2 = 2.5f;
    if (__builtin_isunordered(f1, make_nanf()) || f1 < f2) result |= 4;
    
    return result;
}

/* Test LTGT (unordered or not equal) */
int test_ltgt(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    
    /* Using __builtin_islessgreater */
    if (__builtin_islessgreater(d1, d2)) result |= 1;
    
    /* Manual: (a < b) || (a > b) with ordered check */
    if ((!__builtin_isunordered(d1, d2) && (d1 < d2 || d1 > d2))) result |= 2;
    
    /* With NaN - should be false */
    if (__builtin_islessgreater(d1, d_nan)) result |= 4;
    
    /* Float version */
    float f1 = vf1;
    float f2 = vf2;
    if (__builtin_islessgreater(f1, f2)) result |= 8;
    
    return result;
}

/* Test mixed precision comparisons */
int test_mixed_precision(void) {
    int result = 0;
    double d_val = 3.14159;
    float f_val = 2.71828f;
    double d_nan = make_nan();
    float f_nan = make_nanf();
    
    /* Mixed with NaN */
    if (__builtin_isunordered(d_val, f_nan)) result |= 1;
    if (!__builtin_isunordered(f_val, d_nan)) result |= 2;
    
    /* Mixed comparisons */
    if (d_val > f_val) result |= 4;
    if (!(f_val >= d_val)) result |= 8;
    
    return result;
}

/* Test with function returns */
int test_function_returns(void) {
    int result = 0;
    
    /* sqrt(-1) returns NaN */
    double nan_from_sqrt = sqrt(-1.0);
    if (__builtin_isunordered(nan_from_sqrt, 0.0)) result |= 1;
    
    /* acos(2.0) returns NaN */
    double nan_from_acos = acos(2.0);
    if (!__builtin_isunordered(1.0, nan_from_acos)) result |= 2;
    
    /* Normal function result */
    double normal_sqrt = sqrt(4.0);
    if (__builtin_islessgreater(normal_sqrt, 1.0)) result |= 4;
    
    return result;
}

/* Main driver that uses all test results */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests */
    checksum ^= test_unordered();
    printf("test_unordered: %d\n", test_unordered());
    
    checksum ^= test_ordered();
    printf("test_ordered: %d\n", test_ordered());
    
    checksum ^= test_uneq();
    printf("test_uneq: %d\n", test_uneq());
    
    checksum ^= test_unge();
    printf("test_unge: %d\n", test_unge());
    
    checksum ^= test_ungt();
    printf("test_ungt: %d\n", test_ungt());
    
    checksum ^= test_unle();
    printf("test_unle: %d\n", test_unle());
    
    checksum ^= test_unlt();
    printf("test_unlt: %d\n", test_unlt());
    
    checksum ^= test_ltgt();
    printf("test_ltgt: %d\n", test_ltgt());
    
    checksum ^= test_mixed_precision();
    printf("test_mixed_precision: %d\n", test_mixed_precision());
    
    checksum ^= test_function_returns();
    printf("test_function_returns: %d\n", test_function_returns());
    
    printf("Final checksum: %d\n", checksum);
    
    /* Use checksum in a way that can't be optimized away */
    volatile int* dummy = (volatile int*)&checksum;
    return *dummy & 0xFF;
}
