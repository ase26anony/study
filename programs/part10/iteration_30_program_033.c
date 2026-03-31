/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Helper to generate NaN values */
static double get_nan(void) {
    return __builtin_nan("");
}

static float get_nanf(void) {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code */
int test_unordered(void) {
    volatile double d1 = get_nan();
    volatile double d2 = 3.14;
    volatile float f1 = get_nanf();
    volatile float f2 = 2.71f;
    
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d2)) result |= 1;
    if (__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Alternative unordered check */
    if (!(d1 == d1)) result |= 4;  /* NaN check */
    
    /* Mixed types */
    if (__builtin_isunordered(d1, f2)) result |= 8;
    
    return result;
}

/* Test ORDERED condition code */
int test_ordered(void) {
    volatile double d1 = 1.0;
    volatile double d2 = 2.0;
    volatile float f1 = 3.0f;
    volatile float f2 = get_nanf();
    
    int result = 0;
    
    /* Ordered checks */
    if (!__builtin_isunordered(d1, d2)) result |= 1;
    if (!__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Ordered comparison after function call */
    double sqrt_val = sqrt(-1.0);  /* Returns NaN */
    if (!__builtin_isunordered(d1, sqrt_val)) result |= 4;
    
    return result;
}

/* Test UNEQ (unordered or equal) */
int test_uneq(void) {
    volatile double a = get_nan();
    volatile double b = 5.0;
    volatile double c = 5.0;
    volatile double d = get_nan();
    
    int result = 0;
    
    /* Using !(a != b) which for floating-point includes unordered case */
    if (!(a != b)) result |= 1;    /* a is NaN, so unordered -> true */
    if (!(c != d)) result |= 2;    /* d is NaN, so unordered -> true */
    if (!(c != b)) result |= 4;    /* Both equal -> true */
    
    /* With constants */
    if (!(a != 0.0)) result |= 8;
    if (!(0.0 != -0.0)) result |= 16;  /* +0 == -0 */
    
    return result;
}

/* Test UNGE (not less than) - generates "nlt" */
int test_unge(void) {
    volatile double x = 3.0;
    volatile double y = 2.0;
    volatile double z = get_nan();
    volatile float f1 = 4.0f;
    volatile float f2 = 4.0f;
    
    int result = 0;
    
    /* Inverse conditions that should generate "nlt" */
    if (!(x < y)) result |= 1;     /* 3.0 < 2.0 is false, so true */
    if (!(f1 < f2)) result |= 2;   /* 4.0 < 4.0 is false, so true */
    
    /* With NaN - important for unordered behavior */
    if (!(z < x)) result |= 4;     /* NaN < 3.0 is false (unordered), so true */
    if (!(x < z)) result |= 8;     /* 3.0 < NaN is false (unordered), so true */
    
    /* Mixed precision */
    if (!((double)f1 < y)) result |= 16;
    
    return result;
}

/* Test UNGT (not less than or equal) - generates "nle" */
int test_ungt(void) {
    volatile double a = 5.0;
    volatile double b = 3.0;
    volatile double nan = get_nan();
    volatile float c = 2.0f;
    volatile float d = 2.0f;
    
    int result = 0;
    
    /* Inverse conditions that should generate "nle" */
    if (!(a <= b)) result |= 1;    /* 5.0 <= 3.0 is false, so true */
    if (!(c <= d)) result |= 2;    /* 2.0 <= 2.0 is true, so false */
    
    /* With NaN */
    if (!(nan <= a)) result |= 4;  /* NaN <= 5.0 is false (unordered), so true */
    if (!(b <= nan)) result |= 8;  /* 3.0 <= NaN is false (unordered), so true */
    
    /* Using function return value */
    if (!(sqrt(4.0) <= 1.0)) result |= 16;
    
    return result;
}

/* Test UNLE (unordered or less than or equal) - generates "ule" */
int test_unle(void) {
    volatile double p = 1.0;
    volatile double q = 2.0;
    volatile double r = get_nan();
    volatile double s = 2.0;
    
    int result = 0;
    
    /* Various comparisons that should use "ule" */
    if (p <= q || __builtin_isunordered(p, q)) result |= 1;
    if (r <= s || __builtin_isunordered(r, s)) result |= 2;
    if (s <= r || __builtin_isunordered(s, r)) result |= 4;
    
    /* With constants */
    if (1.0 <= 2.0 || __builtin_isunordered(1.0, 2.0)) result |= 8;
    if (get_nan() <= 0.0 || __builtin_isunordered(get_nan(), 0.0)) result |= 16;
    
    return result;
}

/* Test UNLT (unordered or less than) - generates "ult" */
int test_unlt(void) {
    volatile double u = 1.5;
    volatile double v = 2.5;
    volatile double w = get_nan();
    
    int result = 0;
    
    /* Comparisons that should use "ult" */
    if (u < v || __builtin_isunordered(u, v)) result |= 1;
    if (w < v || __builtin_isunordered(w, v)) result |= 2;
    if (u < w || __builtin_isunordered(u, w)) result |= 4;
    
    /* Mixed float/double */
    volatile float f1 = 1.0f;
    volatile float f2 = 2.0f;
    if (f1 < f2 || __builtin_isunordered(f1, f2)) result |= 8;
    
    return result;
}

/* Test LTGT (less than or greater than, ordered) - generates "une" */
int test_ltgt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = get_nan();
    volatile double d = 1.0;
    
    int result = 0;
    
    /* Direct use of __builtin_islessgreater */
    if (__builtin_islessgreater(a, b)) result |= 1;
    if (__builtin_islessgreater(b, a)) result |= 2;
    
    /* With NaN - should be false */
    if (__builtin_islessgreater(a, c)) result |= 4;
    if (__builtin_islessgreater(c, b)) result |= 8;
    
    /* Equal values - should be false */
    if (__builtin_islessgreater(a, d)) result |= 16;
    
    /* Alternative: (a < b) || (a > b) with ordered check */
    if ((a < b) || (a > b)) result |= 32;
    
    return result;
}

/* Test various condition codes in control flow */
void test_control_flow(int *results) {
    volatile double x = 0.0;
    volatile double y = 1.0;
    volatile double nan = get_nan();
    volatile double inf = INFINITY;
    
    /* Complex if-else chain using different conditions */
    if (__builtin_isunordered(x, nan)) {
        results[0] = 1;  /* UNORDERED */
    } else if (!__builtin_isunordered(y, inf)) {
        results[0] = 2;  /* ORDERED */
    }
    
    /* Ternary operator with unordered check */
    results[1] = __builtin_isunordered(nan, y) ? 3 : 4;
    
    /* While loop with ordered comparison */
    int i = 0;
    while (i < 3 && !__builtin_isunordered(x + i, y)) {
        results[2] += i;
        i++;
    }
    
    /* Switch-like behavior using comparison results */
    int cmp_res = (x < y) ? 0 : (__builtin_isunordered(x, nan) ? 1 : 2);
    results[3] = cmp_res;
}

/* Main driver that uses all test results */
int main(void) {
    int results[8] = {0};
    int checksum = 0;
    
    /* Run all condition code tests */
    results[0] = test_unordered();
    results[1] = test_ordered();
    results[2] = test_uneq();
    results[3] = test_unge();
    results[4] = test_ungt();
    results[5] = test_unle();
    results[6] = test_unlt();
    results[7] = test_ltgt();
    
    /* Additional control flow tests */
    test_control_flow(results);
    
    /* Compute checksum to ensure all comparisons affect output */
    for (int i = 0; i < 8; i++) {
        checksum ^= results[i];
        printf("Result[%d] = 0x%x\n", i, results[i]);
    }
    
    printf("Final checksum: 0x%x\n", checksum);
    
    /* Return checksum as exit code (masked to 0-255) */
    return checksum & 0xFF;
}
