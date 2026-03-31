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
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    float f1 = vf1;
    float f_nan = make_nanf();
    
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d_nan)) result |= 1;
    if (__builtin_isunordered(d_nan, d2)) result |= 2;
    if (__builtin_isunordered(f1, f_nan)) result |= 4;
    
    /* Using !(a == a) pattern */
    if (!(d_nan == d_nan)) result |= 8;
    if (!(f_nan == f_nan)) result |= 16;
    
    /* Mixed precision */
    if (__builtin_isunordered(d1, f_nan)) result |= 32;
    
    return result;
}

/* Test ORDERED condition code */
int test_ordered(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Direct ordered checks */
    if (__builtin_isordered(d1, d2)) result |= 1;
    if (__builtin_isordered(f1, f2)) result |= 2;
    
    /* Ordered after NaN check */
    if (!__builtin_isunordered(d1, d2)) result |= 4;
    if (!__builtin_isunordered(f1, f_nan)) result |= 8;
    
    /* Compare with constants */
    if (__builtin_isordered(3.14, 2.71)) result |= 16;
    if (__builtin_isordered(1.0f, 0.0f)) result |= 32;
    
    return result;
}

/* Test UNEQ (unordered or equal) condition code */
int test_uneq(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Using !(a < b) && !(a > b) which includes NaN case */
    if (!(d1 < d2) && !(d1 > d2)) result |= 1;  /* Equal case */
    if (!(d_nan < d1) && !(d_nan > d1)) result |= 2;  /* NaN case (unordered) */
    
    /* Float version */
    if (!(f1 < f2) && !(f1 > f2)) result |= 4;
    if (!(f_nan < f1) && !(f_nan > f1)) result |= 8;
    
    /* Compare with zero */
    if (!(0.0 < 0.0) && !(0.0 > 0.0)) result |= 16;
    if (!(-0.0 < 0.0) && !(-0.0 > 0.0)) result |= 32;
    
    return result;
}

/* Test UNGE (not less than) condition code */
int test_unge(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Inverse conditions to generate "nlt" */
    if (!(d1 < d2)) result |= 1;      /* d1 >= d2 or unordered */
    if (!(d2 < d1)) result |= 2;
    if (!(f1 < f2)) result |= 4;
    
    /* With NaN */
    if (!(d1 < d_nan)) result |= 8;   /* Always true (unordered) */
    if (!(d_nan < d1)) result |= 16;  /* Always true (unordered) */
    
    /* Compare with constants */
    if (!(3.0 < 2.0)) result |= 32;
    if (!(1.0f < 0.0f)) result |= 64;
    
    return result;
}

/* Test UNGT (not less than or equal) condition code */
int test_ungt(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Inverse conditions to generate "nle" */
    if (!(d1 <= d2)) result |= 1;      /* d1 > d2 or unordered */
    if (!(d2 <= d1)) result |= 2;
    if (!(f1 <= f2)) result |= 4;
    
    /* With NaN */
    if (!(d1 <= d_nan)) result |= 8;   /* Always true (unordered) */
    if (!(d_nan <= d1)) result |= 16;  /* Always true (unordered) */
    
    /* Using builtin for greater (unordered-aware) */
    if (__builtin_isgreater(d1, d2)) result |= 32;
    if (__builtin_isgreater(f1, f2)) result |= 64;
    
    return result;
}

/* Test UNLE (unordered or less than or equal) condition code */
int test_unle(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Direct comparisons that should generate "ule" */
    if (d1 <= d2) result |= 1;
    if (d2 <= d1) result |= 2;
    if (f1 <= f2) result |= 4;
    
    /* With NaN (always false for ordered, true for unordered) */
    if (d_nan <= d1) result |= 8;
    if (d1 <= d_nan) result |= 16;
    
    /* Compare function results */
    if (sin(0.0) <= cos(0.0)) result |= 32;
    if (sqrt(4.0) <= 2.0) result |= 64;
    
    return result;
}

/* Test UNLT (unordered or less than) condition code */
int test_unlt(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Direct comparisons that should generate "ult" */
    if (d1 < d2) result |= 1;
    if (d2 < d1) result |= 2;
    if (f1 < f2) result |= 4;
    
    /* With NaN */
    if (d_nan < d1) result |= 8;
    if (d1 < d_nan) result |= 16;
    
    /* Using builtin for less (unordered-aware) */
    if (__builtin_isless(d1, d2)) result |= 32;
    if (__builtin_isless(f1, f2)) result |= 64;
    
    return result;
}

/* Test LTGT (less than or greater than, ordered) condition code */
int test_ltgt(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Using __builtin_islessgreater */
    if (__builtin_islessgreater(d1, d2)) result |= 1;
    if (__builtin_islessgreater(f1, f2)) result |= 2;
    
    /* Manual ordered comparison: (a < b) || (a > b) */
    if ((d1 < d2) || (d1 > d2)) result |= 4;
    if ((f1 < f2) || (f1 > f2)) result |= 8;
    
    /* With NaN (should be false) */
    if (__builtin_islessgreater(d_nan, d1)) result |= 16;
    if ((d_nan < d1) || (d_nan > d1)) result |= 32;
    
    /* Compare with zero */
    if (__builtin_islessgreater(1.0, 0.0)) result |= 64;
    if (__builtin_islessgreater(0.0, -1.0)) result |= 128;
    
    return result;
}

/* Test mixed condition codes in control flow */
void test_control_flow(int *result) {
    double a = vd1;
    double b = vd2;
    double nan = make_nan();
    float fa = vf1;
    float fb = vf2;
    
    /* Complex if-else chain with various conditions */
    if (__builtin_isunordered(a, nan)) {
        *result += 1;
    } else if (__builtin_isordered(a, b)) {
        *result += 2;
    }
    
    /* Ternary operator with unordered check */
    int x = __builtin_isunordered(fa, fb) ? 4 : 8;
    *result += x;
    
    /* While loop with ordered check */
    int count = 0;
    while (count < 3 && __builtin_isordered(a + count, b)) {
        *result += 16;
        count++;
    }
    
    /* Switch based on comparison results */
    int cmp = 0;
    if (a < b) cmp = 1;
    else if (!(a >= b)) cmp = 2;  /* Inverse for UNGE */
    else if (a <= b) cmp = 3;
    
    switch (cmp) {
        case 1: *result += 32; break;
        case 2: *result += 64; break;
        case 3: *result += 128; break;
    }
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
    
    /* Test control flow patterns */
    int flow_result = 0;
    test_control_flow(&flow_result);
    checksum ^= flow_result;
    
    /* Additional direct comparisons in main */
    volatile double x = 1.5;
    volatile double y = 2.5;
    volatile double z = make_nan();
    
    /* Array indexing based on comparisons */
    int array[8] = {0};
    int idx = 0;
    
    if (__builtin_isunordered(x, z)) idx |= 1;
    if (!(x < y)) idx |= 2;           /* UNGE */
    if (!(x <= y)) idx |= 4;          /* UNGT */
    if (x <= y) idx |= 8;             /* UNLE */
    if (x < y) idx |= 16;             /* UNLT */
    if (__builtin_islessgreater(x, y)) idx |= 32; /* LTGT */
    
    if (idx < 8) {
        array[idx] = 1;
        checksum += array[idx];
    }
    
    /* Output checksum to ensure all code executes */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
