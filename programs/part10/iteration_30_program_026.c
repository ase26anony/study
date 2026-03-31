/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent constant folding */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = 0.0/0.0;
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = 0.0f/0.0f;

/* Generate NaN in a way compiler can't easily predict */
static double get_nan(void) {
    return __builtin_nan("");
}

static float get_nanf(void) {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code (unord) */
int test_unordered(void) {
    double d1 = vd1;
    double d2 = vd_nan;
    float f1 = vf1;
    float f2 = vf_nan;
    
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d2)) result |= 1;
    if (__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Alternative unordered check */
    double d3 = get_nan();
    if (!(d3 == d3)) result |= 4;  /* NaN check */
    
    /* Mixed types */
    if (__builtin_isunordered(d1, f2)) result |= 8;
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered(void) {
    double d1 = vd1;
    double d2 = vd2;
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Direct ordered checks */
    if (__builtin_isordered(d1, d2)) result |= 1;
    if (__builtin_isordered(f1, f2)) result |= 2;
    
    /* Ordered check with NaN */
    double d3 = get_nan();
    if (!__builtin_isunordered(d1, d3)) result |= 4;
    
    /* Function return comparison */
    if (__builtin_isordered(sqrt(d1), sqrt(d2))) result |= 8;
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq(void) {
    double d1 = vd1;
    double d2 = vd1;  /* Same value */
    float f1 = vf1;
    float f2 = vf1;
    
    int result = 0;
    
    /* Unordered or equal */
    if (!__builtin_islessgreater(d1, d2)) result |= 1;
    if (!__builtin_islessgreater(f1, f2)) result |= 2;
    
    /* With NaN - should be true for unordered */
    double d3 = get_nan();
    if (!__builtin_islessgreater(d1, d3)) result |= 4;
    
    /* Using volatile to prevent optimization */
    volatile double vd = 3.0;
    if (!__builtin_islessgreater(vd, vd)) result |= 8;
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge(void) {
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    float f1 = vf2;
    float f2 = vf1;
    
    int result = 0;
    
    /* Not less than (greater or equal or unordered) */
    if (!(d1 < d2)) result |= 1;
    if (!(f1 < f2)) result |= 2;
    
    /* With NaN */
    double d3 = get_nan();
    if (!(d1 < d3)) result |= 4;
    if (!(d3 < d1)) result |= 8;
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt(void) {
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    float f1 = vf2;
    float f2 = vf1;
    
    int result = 0;
    
    /* Not less or equal (greater or unordered) */
    if (!(d1 <= d2)) result |= 1;
    if (!(f1 <= f2)) result |= 2;
    
    /* Inverse of <= */
    double d3 = get_nan();
    if (!(d1 <= d3)) result |= 4;
    if (!(d3 <= d1)) result |= 8;
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle(void) {
    double d1 = vd1;  /* 1.0 */
    double d2 = vd2;  /* 2.0 */
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Unordered or less or equal */
    if (!(d1 > d2)) result |= 1;  /* Not greater = less or equal or unordered */
    if (!(f1 > f2)) result |= 2;
    
    /* With NaN */
    double d3 = get_nan();
    if (!(d1 > d3)) result |= 4;
    if (!(d3 > d1)) result |= 8;
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt(void) {
    double d1 = vd1;  /* 1.0 */
    double d2 = vd2;  /* 2.0 */
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Unordered or less than */
    if (!(d1 >= d2)) result |= 1;  /* Not greater or equal = less or unordered */
    if (!(f1 >= f2)) result |= 2;
    
    /* With NaN */
    double d3 = get_nan();
    if (!(d1 >= d3)) result |= 4;
    if (!(d3 >= d1)) result |= 8;
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt(void) {
    double d1 = vd1;
    double d2 = vd2;
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Less or greater (ordered and not equal) */
    if (__builtin_islessgreater(d1, d2)) result |= 1;
    if (__builtin_islessgreater(f1, f2)) result |= 2;
    
    /* Alternative: (a < b) || (a > b) for ordered values */
    if ((d1 < d2) || (d1 > d2)) result |= 4;
    
    /* With function calls */
    if (__builtin_islessgreater(sqrt(d1), sqrt(d2))) result |= 8;
    
    return result;
}

/* Test mixed conditions in control flow */
int test_control_flow(void) {
    double a = vd1;
    double b = vd2;
    double c = get_nan();
    float fa = vf1;
    float fb = vf2;
    float fc = get_nanf();
    
    int result = 0;
    int i;
    
    /* Complex if-else chain with various conditions */
    for (i = 0; i < 4; i++) {
        double x = (i & 1) ? a : c;
        double y = (i & 2) ? b : c;
        
        if (__builtin_isunordered(x, y)) {
            result += 1;
        } else if (!__builtin_islessgreater(x, y)) {
            result += 2;
        } else if (!(x < y)) {
            result += 4;
        } else if (!(x <= y)) {
            result += 8;
        }
    }
    
    /* Ternary operator usage */
    result += __builtin_isordered(fa, fb) ? 16 : 0;
    result += !__builtin_islessgreater(fa, fc) ? 32 : 0;
    result += !(fa >= fb) ? 64 : 0;
    result += !(fa > fb) ? 128 : 0;
    
    return result;
}

/* Main driver that uses all results */
int main(void) {
    int checksum = 0;
    
    /* Call all test functions */
    checksum ^= test_unordered();
    checksum ^= (test_ordered() << 1);
    checksum ^= (test_uneq() << 2);
    checksum ^= (test_unge() << 3);
    checksum ^= (test_ungt() << 4);
    checksum ^= (test_unle() << 5);
    checksum ^= (test_unlt() << 6);
    checksum ^= (test_ltgt() << 7);
    checksum ^= test_control_flow();
    
    /* Use results in array indexing to prevent dead code elimination */
    static const char *results[] = {
        "zero", "one", "two", "three", "four", "five", "six", "seven"
    };
    
    int index = checksum & 7;
    printf("Result: %s (checksum: 0x%08x)\n", results[index], checksum);
    
    return checksum != 0 ? 0 : 1;
}
