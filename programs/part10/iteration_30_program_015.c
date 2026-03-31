/* test_conditions.c - Program to trigger floating-point condition code generation */
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

/* Test UNORDERED condition code */
int test_unordered(void) {
    double nan = make_nan();
    float f_nan = (float)nan;
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(vd1, nan)) result |= 1;
    if (__builtin_isunordered(nan, vd2)) result |= 2;
    if (__builtin_isunordered(vf1, f_nan)) result |= 4;
    if (__builtin_isunordered(f_nan, vf2)) result |= 8;
    
    /* Using !(a == a) pattern */
    volatile double x = nan;
    if (!(x == x)) result |= 16;
    
    return result;
}

/* Test ORDERED condition code */
int test_ordered(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Ordered checks */
    if (!__builtin_isunordered(vd1, vd2)) result |= 1;
    if (!__builtin_isunordered(vd1, 0.0)) result |= 2;
    if (!__builtin_isunordered(vf1, vf2)) result |= 4;
    
    /* Compare with function return */
    double sqrt_result = sqrt(vd1);
    if (!__builtin_isunordered(sqrt_result, sqrt_result)) result |= 8;
    
    return result;
}

/* Test UNEQ (unordered or equal) */
int test_uneq(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Using !(a < b) && !(a > b) which includes unordered case */
    volatile double a = vd1;
    volatile double b = vd2;
    
    if (!(a < b) && !(a > b)) result |= 1;  /* Should be equal or unordered */
    
    /* Compare with NaN */
    if (!(nan < vd1) && !(nan > vd1)) result |= 2;
    
    /* Compare equal values */
    if (!(vd1 < vd1) && !(vd1 > vd1)) result |= 4;
    
    return result;
}

/* Test UNGE (not less than - unordered or greater or equal) */
int test_unge(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Using !(a < b) which generates nlt */
    if (!(vd1 < vd2)) result |= 1;
    if (!(vf1 < vf2)) result |= 2;
    
    /* With NaN operand */
    if (!(nan < vd1)) result |= 4;
    if (!(vd1 < nan)) result |= 8;
    
    /* Mixed precision */
    if (!((double)vf1 < vd2)) result |= 16;
    
    return result;
}

/* Test UNGT (not less or equal - unordered or greater) */
int test_ungt(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Using !(a <= b) which generates nle */
    if (!(vd1 <= vd2)) result |= 1;
    if (!(vf1 <= vf2)) result |= 2;
    
    /* With constants */
    if (!(0.0 <= vd1)) result |= 4;
    if (!(vd1 <= 0.0)) result |= 8;
    
    /* With function return */
    double sin_val = sin(vd1);
    if (!(sin_val <= 1.0)) result |= 16;
    
    return result;
}

/* Test UNLE (unordered or less or equal) */
int test_unle(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Direct comparison that might generate ule */
    volatile double a = vd1;
    volatile double b = vd2;
    
    /* Using ternary operator with comparison */
    result = (a <= b) ? 1 : 0;
    result |= (vf1 <= vf2) ? 2 : 0;
    
    /* With NaN */
    result |= (nan <= vd1) ? 4 : 0;
    result |= (vd1 <= nan) ? 8 : 0;
    
    return result;
}

/* Test UNLT (unordered or less than) */
int test_unlt(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Direct less-than comparisons */
    result = (vd1 < vd2) ? 1 : 0;
    result |= (vf1 < vf2) ? 2 : 0;
    
    /* With NaN operand */
    result |= (nan < vd1) ? 4 : 0;
    result |= (vd1 < nan) ? 8 : 0;
    
    /* Mixed types */
    result |= ((float)vd1 < vf2) ? 16 : 0;
    
    return result;
}

/* Test LTGT (less or greater - ordered and not equal) */
int test_ltgt(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Using __builtin_islessgreater */
    if (__builtin_islessgreater(vd1, vd2)) result |= 1;
    if (__builtin_islessgreater(vf1, vf2)) result |= 2;
    
    /* Manual ordered not-equal: (a < b) || (a > b) */
    volatile double a = 1.5;
    volatile double b = 2.5;
    if ((a < b) || (a > b)) result |= 4;
    
    /* With constants */
    if (__builtin_islessgreater(0.0, vd1)) result |= 8;
    
    /* This should be false with NaN */
    if (!__builtin_islessgreater(nan, vd1)) result |= 16;
    
    return result;
}

/* Test various condition codes in control flow */
void test_control_flow(int *results) {
    double nan = make_nan();
    double inf = make_inf();
    
    /* Complex if-else chain with various conditions */
    if (__builtin_isunordered(vd1, nan)) {
        results[0] = 1;
    } else if (!__builtin_isunordered(vd1, vd2)) {
        results[0] = 2;
    }
    
    /* While loop with condition */
    volatile int count = 0;
    volatile double x = 0.0;
    while (!(x >= 10.0) && count < 5) {
        results[1] += count;
        x += 2.0;
        count++;
    }
    
    /* Nested conditions */
    if (!(vd1 < vd2) || __builtin_isunordered(nan, vd1)) {
        results[2] = 3;
    }
    
    /* Switch-like behavior using comparisons */
    double val = vd1;
    if (__builtin_islessgreater(val, 0.0)) {
        results[3] = 4;
    } else if (!(val <= 0.0)) {
        results[3] = 5;
    }
}

/* Main driver function */
int main(void) {
    int results[10] = {0};
    int checksum = 0;
    
    /* Call all test functions */
    results[0] = test_unordered();
    results[1] = test_ordered();
    results[2] = test_uneq();
    results[3] = test_unge();
    results[4] = test_ungt();
    results[5] = test_unle();
    results[6] = test_unlt();
    results[7] = test_ltgt();
    
    /* Test control flow patterns */
    test_control_flow(&results[8]);
    
    /* Compute checksum to ensure all comparisons are live */
    for (int i = 0; i < 10; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Checksum: %08x\n", checksum);
    
    /* Additional volatile writes to prevent dead code elimination */
    volatile int dummy = checksum;
    
    return checksum != 0 ? 0 : 1;
}
