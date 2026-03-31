/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = __builtin_nan("");
volatile double vd_inf = __builtin_inf();
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = __builtin_nanf("");

/* Function to generate NaN */
double get_nan(void) {
    return __builtin_nan("");
}

/* Function to generate infinity */
double get_inf(void) {
    return __builtin_inf();
}

/* Test UNORDERED condition code (unord) */
int test_unordered(void) {
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(vd_nan, vd1)) result |= 1;
    if (__builtin_isunordered(vd1, vd_nan)) result |= 2;
    
    /* Using NaN property */
    double d = get_nan();
    if (!(d == d)) result |= 4;  /* NaN != NaN */
    
    /* Mixed types */
    float f = vf_nan;
    if (__builtin_isunordered(f, vf1)) result |= 8;
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered(void) {
    int result = 0;
    
    /* Direct ordered checks */
    if (__builtin_isordered(vd1, vd2)) result |= 1;
    if (__builtin_isordered(vd2, vd1)) result |= 2;
    
    /* With constants */
    if (__builtin_isordered(3.14, 2.71)) result |= 4;
    
    /* After function calls */
    double d1 = sqrt(4.0);
    double d2 = sqrt(9.0);
    if (__builtin_isordered(d1, d2)) result |= 8;
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq(void) {
    int result = 0;
    
    /* Compare NaN with itself - unordered equal */
    double nan1 = get_nan();
    double nan2 = get_nan();
    
    /* This should generate UNEQ: unordered or equal */
    if (!(nan1 < nan2) && !(nan1 > nan2)) result |= 1;
    
    /* Regular equality that might be unordered */
    if (!__builtin_islessgreater(vd_nan, vd1)) result |= 2;
    
    /* Mixed precision */
    float f = vf_nan;
    double d = vd1;
    if (!(f < d) && !(f > d)) result |= 4;
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge(void) {
    int result = 0;
    
    /* Inverse of less than: !(a < b) */
    if (!(vd1 < vd2)) result |= 1;
    if (!(vd_nan < vd1)) result |= 2;
    
    /* With function return */
    double d = get_nan();
    if (!(d < 0.0)) result |= 4;
    
    /* Mixed types */
    if (!(vf1 < vf2)) result |= 8;
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt(void) {
    int result = 0;
    
    /* Inverse of less or equal: !(a <= b) */
    if (!(vd1 <= vd2)) result |= 1;
    if (!(vd_nan <= vd1)) result |= 2;
    
    /* With constants */
    if (!(3.14 <= 2.71)) result |= 4;
    
    /* After arithmetic */
    double d = vd1 * 2.0;
    if (!(d <= vd2)) result |= 8;
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle(void) {
    int result = 0;
    
    /* Using builtin for unordered less or equal */
    if (!__builtin_isgreater(vd_nan, vd1)) result |= 1;
    if (!__builtin_isgreater(vd1, vd_nan)) result |= 2;
    
    /* Regular comparison that might be unordered */
    double d = get_nan();
    if (!(d > 0.0)) result |= 4;
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt(void) {
    int result = 0;
    
    /* Using builtin for unordered less than */
    if (!__builtin_isgreaterequal(vd_nan, vd1)) result |= 1;
    if (!__builtin_isgreaterequal(vd1, vd_nan)) result |= 2;
    
    /* With mixed operations */
    double d = vd_nan + 1.0;
    if (!(d >= 0.0)) result |= 4;
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt(void) {
    int result = 0;
    
    /* Direct builtin for less or greater (ordered) */
    if (__builtin_islessgreater(vd1, vd2)) result |= 1;
    if (__builtin_islessgreater(vd2, vd1)) result |= 2;
    
    /* Manual ordered comparison */
    double d1 = get_nan();
    double d2 = vd1;
    if ((d1 < d2) || (d1 > d2)) result |= 4;
    
    /* With constants and NaN */
    if (__builtin_islessgreater(vd_nan, 0.0)) result |= 8;
    
    return result;
}

/* Test various comparison patterns in control flow */
int test_control_flow(void) {
    int result = 0;
    int i;
    
    /* Use comparisons in loops */
    for (i = 0; i < 10; i++) {
        double d = i * 0.1;
        if (__builtin_isunordered(d, vd_nan)) {
            result += i;
        }
    }
    
    /* Ternary operator with unordered check */
    double a = vd1;
    double b = vd_nan;
    double c = __builtin_isunordered(a, b) ? a : b;
    result += (int)c;
    
    /* Switch-like behavior based on comparison */
    if (__builtin_islessgreater(vd1, vd2)) {
        result += 100;
    } else if (!__builtin_isordered(vd1, vd_nan)) {
        result += 200;
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
    checksum += test_control_flow();
    
    /* Use results in array indexing to prevent dead code elimination */
    int results[9];
    results[0] = test_unordered();
    results[1] = test_ordered();
    results[2] = test_uneq();
    results[3] = test_unge();
    results[4] = test_ungt();
    results[5] = test_unle();
    results[6] = test_unlt();
    results[7] = test_ltgt();
    results[8] = test_control_flow();
    
    /* Compute final checksum */
    int final_checksum = 0;
    for (int i = 0; i < 9; i++) {
        final_checksum ^= results[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    
    /* Return non-zero if any test failed (simplified) */
    return (final_checksum == 0) ? 1 : 0;
}
