/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Helper to generate NaN values */
static double get_nan(void) { return __builtin_nan(""); }
static float get_nanf(void) { return __builtin_nanf(""); }

/* Volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = 0.0;
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = 0.0f;

/* Function 1: Test UNORDERED (unord) */
int test_unordered(void) {
    double d_nan = get_nan();
    float f_nan = get_nanf();
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(vd1, d_nan)) result |= 1;
    if (__builtin_isunordered(f_nan, vf2)) result |= 2;
    
    /* Using NaN property */
    if (!(d_nan == d_nan)) result |= 4;
    
    /* Mixed types */
    if (__builtin_isunordered(vd1, (double)vf_nan)) result |= 8;
    
    return result;
}

/* Function 2: Test ORDERED (ord) */
int test_ordered(void) {
    double d_nan = get_nan();
    float f_nan = get_nanf();
    int result = 0;
    
    /* Direct ordered checks */
    if (!__builtin_isunordered(vd1, vd2)) result |= 1;
    if (!__builtin_isunordered(vf1, vf2)) result |= 2;
    
    /* Ordered check with NaN */
    if (!__builtin_isunordered(vd1, d_nan)) result |= 4;
    
    /* Function return value */
    double sqrt_neg = sqrt(-1.0);
    if (!__builtin_isunordered(sqrt_neg, 0.0)) result |= 8;
    
    return result;
}

/* Function 3: Test UNEQ (ueq) */
int test_uneq(void) {
    double d_nan = get_nan();
    float f_nan = get_nanf();
    int result = 0;
    
    /* Using !(a != b) which includes unordered case */
    if (!(vd1 != vd2)) result |= 1;
    if (!(vf1 != vf2)) result |= 2;
    
    /* With NaN - should be true for unordered */
    if (!(vd1 != d_nan)) result |= 4;
    if (!(f_nan != vf2)) result |= 8;
    
    return result;
}

/* Function 4: Test UNGE (nlt) */
int test_unge(void) {
    double d_nan = get_nan();
    int result = 0;
    
    /* Inverse of less than: !(a < b) */
    if (!(vd1 < vd2)) result |= 1;
    if (!(vd2 < vd1)) result |= 2;
    
    /* With NaN */
    if (!(vd1 < d_nan)) result |= 4;
    if (!(d_nan < vd1)) result |= 8;
    
    /* Mixed precision */
    if (!((double)vf1 < vd2)) result |= 16;
    
    return result;
}

/* Function 5: Test UNGT (nle) */
int test_ungt(void) {
    float f_nan = get_nanf();
    int result = 0;
    
    /* Inverse of less or equal: !(a <= b) */
    if (!(vf1 <= vf2)) result |= 1;
    if (!(vf2 <= vf1)) result |= 2;
    
    /* With NaN */
    if (!(vf1 <= f_nan)) result |= 4;
    if (!(f_nan <= vf1)) result |= 8;
    
    /* Using constant */
    if (!(0.0 <= -0.0)) result |= 16;
    
    return result;
}

/* Function 6: Test UNLE (ule) */
int test_unle(void) {
    double d_nan = get_nan();
    int result = 0;
    
    /* Using <= which handles unordered */
    if (vd1 <= d_nan) result |= 1;
    if (d_nan <= vd2) result |= 2;
    
    /* Normal comparisons */
    if (vd1 <= vd2) result |= 4;
    if (vd2 <= vd1) result |= 8;
    
    /* With function return */
    double val = sin(3.14159);
    if (val <= 0.0) result |= 16;
    
    return result;
}

/* Function 7: Test UNLT (ult) */
int test_unlt(void) {
    float f_nan = get_nanf();
    int result = 0;
    
    /* Using < which handles unordered */
    if (vf1 < f_nan) result |= 1;
    if (f_nan < vf2) result |= 2;
    
    /* Normal comparisons */
    if (vf1 < vf2) result |= 4;
    if (vf2 < vf1) result |= 8;
    
    /* With volatile */
    if (vf_nan < vf1) result |= 16;
    
    return result;
}

/* Function 8: Test LTGT (une) */
int test_ltgt(void) {
    double d_nan = get_nan();
    float f_nan = get_nanf();
    int result = 0;
    
    /* Direct builtin */
    if (__builtin_islessgreater(vd1, vd2)) result |= 1;
    if (__builtin_islessgreater(vd2, vd1)) result |= 2;
    
    /* With NaN - should be false */
    if (__builtin_islessgreater(vd1, d_nan)) result |= 4;
    if (__builtin_islessgreater(f_nan, vf2)) result |= 8;
    
    /* Manual ordered inequality */
    if ((vd1 < vd2) || (vd1 > vd2)) result |= 16;
    
    return result;
}

/* Function 9: Mixed tests using ternary operator */
int test_ternary(void) {
    double d_nan = get_nan();
    float f_nan = get_nanf();
    int result = 0;
    
    /* Ternary with unordered check */
    result += __builtin_isunordered(vd1, d_nan) ? 1 : 0;
    result += __builtin_isunordered(f_nan, vf2) ? 2 : 0;
    
    /* Ternary with inverse comparisons */
    result += !(vd1 < vd2) ? 4 : 0;  /* UNGE */
    result += !(vf1 <= vf2) ? 8 : 0; /* UNGT */
    
    /* Ternary with ordered inequality */
    result += __builtin_islessgreater(vd1, vd2) ? 16 : 0;
    
    return result;
}

/* Function 10: Array indexing based on comparisons */
int test_array_index(void) {
    static const int values[] = {0, 1, 2, 3, 4, 5, 6, 7};
    double d_nan = get_nan();
    int idx = 0;
    
    /* Build index from comparison results */
    if (__builtin_isunordered(vd1, d_nan)) idx |= 1;
    if (!(vd1 < vd2)) idx |= 2;  /* UNGE */
    if (!(vd1 <= vd2)) idx |= 4; /* UNGT */
    if (__builtin_islessgreater(vd1, vd2)) idx |= 8;
    
    return values[idx & 7];
}

/* Main driver */
int main(void) {
    int checksum = 0;
    
    /* Initialize volatile NaN values */
    vd_nan = get_nan();
    vf_nan = get_nanf();
    
    /* Run all tests */
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    checksum ^= test_ternary();
    checksum ^= test_array_index();
    
    /* Output result to ensure code isn't dead */
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
