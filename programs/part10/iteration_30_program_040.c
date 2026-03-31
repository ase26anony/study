/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = 0.0f / 0.0f; /* Generate NaN */
volatile double vd1 = 3.0;
volatile double vd2 = 4.0;
volatile double vd_nan = 0.0 / 0.0;

/* Function to generate NaN */
static double make_nan(void) {
    return __builtin_nan("");
}

static float make_nanf(void) {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code */
int test_unordered(void) {
    float f1 = vf1;
    float f2 = vf_nan;
    double d1 = vd1;
    double d2 = vd_nan;
    
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(f1, f2)) result |= 1;
    if (__builtin_isunordered(d1, d2)) result |= 2;
    
    /* Alternative unordered check */
    float f3 = make_nanf();
    if (!(f3 == f3)) result |= 4;  /* NaN check */
    
    /* Mixed types */
    if (__builtin_isunordered(f1, d2)) result |= 8;
    
    return result;
}

/* Test ORDERED condition code */
int test_ordered(void) {
    float f1 = vf1;
    float f2 = vf2;
    double d1 = vd1;
    double d2 = make_nan();
    
    int result = 0;
    
    /* Ordered checks */
    if (__builtin_isordered(f1, f2)) result |= 1;
    if (__builtin_isordered(d1, d2)) result |= 2;  /* One is NaN */
    
    /* Ordered after function call */
    float f3 = sqrtf(-1.0f);  /* Returns NaN */
    if (__builtin_isordered(f1, f3)) result |= 4;
    
    return result;
}

/* Test UNEQ (unordered or equal) */
int test_uneq(void) {
    float f1 = vf1;
    float f2 = vf1;  /* Same value */
    float f3 = vf_nan;
    
    int result = 0;
    
    /* Using builtin */
    if (__builtin_isunordered(f1, f2) || f1 == f2) result |= 1;
    
    /* With NaN */
    if (__builtin_isunordered(f1, f3) || f1 == f3) result |= 2;
    
    /* Double precision */
    double d1 = vd1;
    double d2 = vd_nan;
    if (__builtin_isunordered(d1, d2) || d1 == d2) result |= 4;
    
    return result;
}

/* Test UNGE (not less than) - generates "nlt" */
int test_unge(void) {
    float f1 = vf1;
    float f2 = vf2;
    float f3 = vf_nan;
    
    int result = 0;
    
    /* Inverse condition: !(a < b) */
    if (!(f1 < f2)) result |= 1;      /* f1 >= f2 */
    if (!(f2 < f1)) result |= 2;      /* f2 >= f1 */
    
    /* With NaN - should be false for ordered comparison */
    if (!(f1 < f3)) result |= 4;
    if (!(f3 < f1)) result |= 8;
    
    /* Double precision */
    double d1 = vd1;
    double d2 = vd2;
    if (!(d1 < d2)) result |= 16;
    
    return result;
}

/* Test UNGT (not less than or equal) - generates "nle" */
int test_ungt(void) {
    float f1 = vf1;
    float f2 = vf2;
    float f3 = vf1;  /* Equal */
    
    int result = 0;
    
    /* Inverse condition: !(a <= b) */
    if (!(f1 <= f2)) result |= 1;      /* f1 > f2 */
    if (!(f2 <= f1)) result |= 2;      /* f2 > f1 */
    if (!(f1 <= f3)) result |= 4;      /* Equal case */
    
    /* With function return */
    double d1 = sin(0.0);
    double d2 = cos(0.0);
    if (!(d1 <= d2)) result |= 8;
    
    return result;
}

/* Test UNLE (unordered or less than or equal) - generates "ule" */
int test_unle(void) {
    float f1 = vf1;
    float f2 = vf2;
    float f3 = vf_nan;
    
    int result = 0;
    
    /* Using explicit check */
    if (__builtin_isunordered(f1, f2) || f1 <= f2) result |= 1;
    if (__builtin_isunordered(f2, f1) || f2 <= f1) result |= 2;
    
    /* With NaN */
    if (__builtin_isunordered(f1, f3) || f1 <= f3) result |= 4;
    
    return result;
}

/* Test UNLT (unordered or less than) - generates "ult" */
int test_unlt(void) {
    float f1 = vf1;
    float f2 = vf2;
    float f3 = vf_nan;
    
    int result = 0;
    
    /* Using explicit check */
    if (__builtin_isunordered(f1, f2) || f1 < f2) result |= 1;
    if (__builtin_isunordered(f2, f1) || f2 < f1) result |= 2;
    
    /* With NaN */
    if (__builtin_isunordered(f1, f3) || f1 < f3) result |= 4;
    
    /* Double precision */
    double d1 = vd1;
    double d2 = vd_nan;
    if (__builtin_isunordered(d1, d2) || d1 < d2) result |= 8;
    
    return result;
}

/* Test LTGT (less than or greater than) - generates "une" */
int test_ltgt(void) {
    float f1 = vf1;
    float f2 = vf2;
    float f3 = vf1;  /* Equal */
    float f4 = vf_nan;
    
    int result = 0;
    
    /* Using builtin */
    if (__builtin_islessgreater(f1, f2)) result |= 1;
    if (__builtin_islessgreater(f2, f1)) result |= 2;
    if (__builtin_islessgreater(f1, f3)) result |= 4;  /* Equal case */
    
    /* With NaN */
    if (__builtin_islessgreater(f1, f4)) result |= 8;
    
    /* Alternative: (a < b) || (a > b) */
    double d1 = vd1;
    double d2 = vd2;
    if ((d1 < d2) || (d1 > d2)) result |= 16;
    
    return result;
}

/* Test mixed conditions in control flow */
void test_control_flow(int *results) {
    float a = vf1;
    float b = vf2;
    float c = vf_nan;
    
    /* Complex if-else chain with various conditions */
    if (__builtin_isunordered(a, c)) {
        results[0] = 1;
    } else if (!(a >= b)) {  /* Should generate nlt? Actually !(a >= b) is (a < b) */
        results[0] = 2;
    }
    
    /* Ternary operator with unordered check */
    results[1] = __builtin_isordered(b, c) ? 3 : 4;
    
    /* While loop with comparison */
    int i = 0;
    double x = 0.0;
    while (i < 3) {
        x += (__builtin_islessgreater(x, 1.0) ? 0.5 : 0.25);
        i++;
    }
    results[2] = (int)(x * 100);
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Call all test functions */
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    
    /* Test control flow */
    int flow_results[3] = {0};
    test_control_flow(flow_results);
    checksum ^= flow_results[0];
    checksum ^= flow_results[1];
    checksum ^= flow_results[2];
    
    /* Additional direct comparisons with volatile */
    volatile float v1 = 1.5f;
    volatile float v2 = 2.5f;
    volatile float v3 = make_nanf();
    
    /* Force various condition code usages */
    int r1 = (!(v1 < v2)) ? 1 : 0;      /* nlt */
    int r2 = (!(v2 <= v1)) ? 2 : 0;     /* nle */
    int r3 = (__builtin_isunordered(v1, v3)) ? 4 : 0;  /* unord */
    int r4 = (__builtin_isordered(v1, v2)) ? 8 : 0;    /* ord */
    
    checksum ^= r1 ^ r2 ^ r3 ^ r4;
    
    /* Print result to ensure code isn't dead */
    printf("Result checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
