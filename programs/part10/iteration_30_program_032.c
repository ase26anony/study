/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;

/* Function to generate NaN */
static double make_nan(void) {
    return __builtin_nan("");
}

/* Function to generate infinity */
static double make_inf(void) {
    return __builtin_inf();
}

/* Test UNORDERED condition code (unord) */
int test_unordered(void) {
    double nan = make_nan();
    float nanf = __builtin_nanf("");
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(vd1, nan)) result |= 1;
    if (__builtin_isunordered(nanf, vf2)) result |= 2;
    
    /* Using !(a == a) pattern */
    volatile double d = nan;
    if (!(d == d)) result |= 4;
    
    /* Mixed types */
    if (__builtin_isunordered(vd1, nanf)) result |= 8;
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Ordered checks */
    if (!__builtin_isunordered(vd1, vd2)) result |= 1;
    if (!__builtin_isunordered(vf1, vf2)) result |= 2;
    
    /* Using function return values */
    double sqrt_neg = sqrt(-1.0);
    if (!__builtin_isunordered(sqrt_neg, 0.0)) result |= 4;
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq(void) {
    double nan = make_nan();
    float nanf = __builtin_nanf("");
    int result = 0;
    
    /* Comparisons that should generate UNEQ */
    if (!(vd1 > vd2) && !(vd1 < vd2)) result |= 1;
    
    /* With NaN */
    volatile double d = nan;
    if (!(d > 0.0) && !(d < 0.0)) result |= 2;
    
    /* Using builtin */
    if (!__builtin_islessgreater(vf1, vf1)) result |= 4;
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge(void) {
    int result = 0;
    
    /* Inverse of less than */
    if (!(vd1 < vd2)) result |= 1;
    if (!(vf1 < vf2)) result |= 2;
    
    /* With constants */
    if (!(vd1 < 0.0)) result |= 4;
    if (!(vf2 < 1.5f)) result |= 8;
    
    /* Using ternary operator */
    result += (!(vd1 < 3.14)) ? 16 : 0;
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt(void) {
    int result = 0;
    
    /* Inverse of less than or equal */
    if (!(vd1 <= vd2)) result |= 1;
    if (!(vf1 <= vf2)) result |= 2;
    
    /* Mixed precision after promotion */
    float f = 1.5f;
    double d = 2.5;
    if (!(f <= d)) result |= 4;
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Unordered or less than or equal */
    if ((vd1 != vd1) || (vd1 <= vd2)) result |= 1;
    
    /* With NaN */
    volatile double d = nan;
    if ((d != d) || (d <= 0.0)) result |= 2;
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Unordered or less than */
    if ((vd1 != vd1) || (vd1 < vd2)) result |= 1;
    
    /* With function return */
    double inf = make_inf();
    if ((inf != inf) || (inf < 1000.0)) result |= 2;
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt(void) {
    int result = 0;
    
    /* Ordered and not equal - should generate LTGT */
    if (__builtin_islessgreater(vd1, vd2)) result |= 1;
    
    /* Using explicit comparison */
    if ((vd1 < vd2) || (vd1 > vd2)) result |= 2;
    
    /* With constants */
    if (__builtin_islessgreater(vf1, 0.0f)) result |= 4;
    
    /* Mixed types */
    double d = 1.5;
    float f = 2.5f;
    if (__builtin_islessgreater(d, f)) result |= 8;
    
    return result;
}

/* Test all conditions in control flow */
void test_control_flow(void) {
    double nan = make_nan();
    double inf = make_inf();
    float nanf = __builtin_nanf("");
    
    /* Complex if-else chain with various conditions */
    volatile double x = 3.14;
    volatile double y = 2.71;
    
    if (__builtin_isunordered(x, nan)) {
        printf("U");
    } else if (!__builtin_isunordered(y, inf)) {
        printf("O");
    }
    
    if (!(x < y)) {
        printf("N");
    }
    
    if (!(x <= y)) {
        printf("G");
    }
    
    if (__builtin_islessgreater(x, y)) {
        printf("T");
    }
    
    /* While loop with condition */
    volatile int count = 0;
    while (!(x < 0.0) && count < 3) {
        x -= 1.0;
        count++;
    }
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
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
    test_control_flow();
    
    /* Use array indexing based on comparison results */
    int results[8];
    results[0] = __builtin_isunordered(vd1, make_nan()) ? 1 : 0;
    results[1] = !__builtin_isunordered(vf1, vf2) ? 2 : 0;
    results[2] = !__builtin_islessgreater(vd1, vd1) ? 4 : 0;
    results[3] = !(vd1 < vd2) ? 8 : 0;
    results[4] = !(vd1 <= vd2) ? 16 : 0;
    results[5] = (vd1 != vd1) || (vd1 <= vd2) ? 32 : 0;
    results[6] = (vd1 != vd1) || (vd1 < vd2) ? 64 : 0;
    results[7] = __builtin_islessgreater(vd1, vd2) ? 128 : 0;
    
    for (int i = 0; i < 8; i++) {
        checksum += results[i];
    }
    
    printf("\nFinal checksum: %d\n", checksum);
    
    /* Return checksum as exit code for verification */
    return checksum & 0xFF;
}
