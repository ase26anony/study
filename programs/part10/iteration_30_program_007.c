/* test_conditions.c - Program to exercise x86 floating-point condition codes */
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
double make_nan() {
    return __builtin_nan("");
}

float make_nanf() {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code */
int test_unordered() {
    double d1 = vd1;
    double d2 = vd_nan;
    float f1 = vf1;
    float f2 = vf_nan;
    
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d2)) result |= 1;
    if (__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Alternative unordered check */
    if (!(d1 == d1) || !(d2 == d2)) result |= 4;
    
    /* Compare NaN with normal value */
    if (__builtin_isunordered(d1, make_nan())) result |= 8;
    
    return result;
}

/* Test ORDERED condition code */
int test_ordered() {
    double d1 = vd1;
    double d2 = vd2;
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Ordered checks */
    if (!__builtin_isunordered(d1, d2)) result |= 1;
    if (!__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Ordered comparison after function call */
    double nan_val = make_nan();
    if (!__builtin_isunordered(d1, nan_val)) result |= 4;
    else result |= 8;
    
    return result;
}

/* Test UNEQ (unordered or equal) */
int test_uneq() {
    double d1 = vd1;
    double d2 = vd1;  /* Same value */
    double d3 = vd_nan;
    
    int result = 0;
    
    /* This should generate UNEQ when optimized */
    if (!(d1 < d2) && !(d1 > d2)) result |= 1;  /* Equal case */
    
    /* With NaN */
    if (!(d1 < d3) && !(d1 > d3)) result |= 2;  /* Unordered case */
    
    /* Mixed types */
    float f1 = vf1;
    float f2 = vf1;
    if (!(f1 < f2) && !(f1 > f2)) result |= 4;
    
    return result;
}

/* Test UNGE (not less than) - generates "nlt" */
int test_unge() {
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d3 = vd_nan;
    
    int result = 0;
    
    /* Inverse condition: !(a < b) */
    if (!(d2 < d1)) result |= 1;  /* 1.0 < 2.0 is true, inverse is false */
    if (!(d1 < d2)) result |= 2;  /* 2.0 < 1.0 is false, inverse is true */
    
    /* With NaN - should be unordered */
    if (!(d1 < d3)) result |= 4;
    if (!(d3 < d1)) result |= 8;
    
    /* Float version */
    float f1 = vf2;
    float f2 = vf1;
    if (!(f2 < f1)) result |= 16;
    
    return result;
}

/* Test UNGT (not less than or equal) - generates "nle" */
int test_ungt() {
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d3 = vd1;  /* 1.0 - equal case */
    
    int result = 0;
    
    /* Inverse condition: !(a <= b) */
    if (!(d2 <= d1)) result |= 1;  /* 1.0 <= 2.0 is true, inverse is false */
    if (!(d1 <= d2)) result |= 2;  /* 2.0 <= 1.0 is false, inverse is true */
    if (!(d1 <= d3)) result |= 4;  /* 1.0 <= 1.0 is true, inverse is false */
    
    /* With function return */
    double val = sqrt(-1.0);  /* Returns NaN */
    if (!(d1 <= val)) result |= 8;
    
    return result;
}

/* Test UNLE (unordered or less than or equal) - generates "ule" */
int test_unle() {
    double d1 = vd1;
    double d2 = vd2;
    double d3 = vd_nan;
    
    int result = 0;
    
    /* Using ordered comparison */
    if (__builtin_islessequal(d1, d2)) result |= 1;
    if (__builtin_islessequal(d2, d1)) result |= 2;
    
    /* With NaN */
    if (__builtin_islessequal(d1, d3)) result |= 4;
    if (__builtin_islessequal(d3, d1)) result |= 8;
    
    return result;
}

/* Test UNLT (unordered or less than) - generates "ult" */
int test_unlt() {
    double d1 = vd1;
    double d2 = vd2;
    double d3 = vd_nan;
    
    int result = 0;
    
    /* Using ordered comparison */
    if (__builtin_isless(d1, d2)) result |= 1;
    if (__builtin_isless(d2, d1)) result |= 2;
    
    /* With NaN */
    if (__builtin_isless(d1, d3)) result |= 4;
    if (__builtin_isless(d3, d1)) result |= 8;
    
    /* Float version with constants */
    float f1 = 1.5f;
    float f2 = 2.5f;
    if (__builtin_isless(f1, f2)) result |= 16;
    
    return result;
}

/* Test LTGT (less than or greater than) - generates "une" */
int test_ltgt() {
    double d1 = vd1;
    double d2 = vd2;
    double d3 = vd1;  /* Equal to d1 */
    double d4 = vd_nan;
    
    int result = 0;
    
    /* Direct builtin */
    if (__builtin_islessgreater(d1, d2)) result |= 1;
    if (__builtin_islessgreater(d2, d1)) result |= 2;
    if (__builtin_islessgreater(d1, d3)) result |= 4;  /* Equal, should be false */
    
    /* With NaN */
    if (__builtin_islessgreater(d1, d4)) result |= 8;  /* Should be false (unordered) */
    
    /* Manual expansion: (a < b) || (a > b) */
    if ((d1 < d2) || (d1 > d2)) result |= 16;
    if ((d1 < d3) || (d1 > d3)) result |= 32;
    
    return result;
}

/* Test mixed precision comparisons */
int test_mixed_precision() {
    float f1 = vf1;
    double d1 = vd2;
    float f_nan = make_nanf();
    double d_nan = make_nan();
    
    int result = 0;
    
    /* float vs double */
    if (__builtin_isunordered(f1, d1)) result |= 1;
    if (!__builtin_isunordered(f1, d_nan)) result |= 2;
    if (__builtin_islessgreater(f1, d1)) result |= 4;
    if (!(f1 < d1)) result |= 8;  /* Should generate nlt */
    
    /* double vs float */
    if (__builtin_isless(d1, f1)) result |= 16;
    if (!(d1 <= f_nan)) result |= 32;  /* Should generate nle */
    
    return result;
}

/* Test with array indexing based on comparisons */
int test_with_arrays() {
    static const int values[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = vd_nan;
    
    int result = 0;
    
    /* Use comparison results to index array */
    if (__builtin_isunordered(d1, d_nan)) {
        result += values[0];
    }
    
    if (!__builtin_isunordered(d1, d2)) {
        result += values[1];
    }
    
    if (!(d1 < d2)) {  /* nlt */
        result += values[2];
    }
    
    if (__builtin_islessgreater(d1, d2)) {  /* une */
        result += values[3];
    }
    
    return result;
}

/* Main driver function */
int main() {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Call all test functions */
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
    
    checksum ^= test_with_arrays();
    printf("test_with_arrays: %d\n", test_with_arrays());
    
    printf("Final checksum: %d\n", checksum);
    
    /* Ensure all comparisons have side effects */
    volatile int dummy = checksum;
    
    return checksum == 0 ? 0 : 1;
}
