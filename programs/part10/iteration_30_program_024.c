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
int test_unordered(float a, float b) {
    volatile float va = a;
    volatile float vb = b;
    
    /* Direct unordered check */
    int res1 = __builtin_isunordered(va, vb);
    
    /* Alternative unordered check */
    int res2 = !(va == va) || !(vb == vb);
    
    /* Using in control flow */
    if (__builtin_isunordered(va, vb)) {
        return res1 + res2 + 1;
    }
    return res1 + res2;
}

/* Test ORDERED condition code */
int test_ordered(double a, double b) {
    volatile double va = a;
    volatile double vb = b;
    
    /* Ordered check */
    int res = (va == va) && (vb == vb);
    
    /* In control flow */
    while ((va == va) && (vb == vb)) {
        res *= 2;
        break;
    }
    
    return res;
}

/* Test UNEQ (unordered or equal) */
int test_uneq(float a, float b) {
    volatile float va = a;
    volatile float vb = b;
    
    /* This should generate ueq */
    int res = !__builtin_islessgreater(va, vb);
    
    /* Alternative using control flow */
    if (!(va < vb || va > vb)) {
        res += 10;
    }
    
    return res;
}

/* Test UNGE (not less than) - should generate nlt */
int test_unge(double a, double b) {
    volatile double va = a;
    volatile double vb = b;
    
    /* Inverse condition */
    int res = !(va < vb);
    
    /* Using in ternary operator */
    return res ? 100 : 200;
}

/* Test UNGT (not less than or equal) - should generate nle */
int test_ungt(float a, float b) {
    volatile float va = a;
    volatile float vb = b;
    
    /* Inverse condition */
    int res = !(va <= vb);
    
    /* Complex expression to prevent optimization */
    static int counter = 0;
    counter += res;
    return counter;
}

/* Test UNLE (unordered or less than or equal) - should generate ule */
int test_unle(double a, double b) {
    volatile double va = a;
    volatile double vb = b;
    
    /* This pattern often generates ule */
    int res = __builtin_isunordered(va, vb) || (va <= vb);
    
    return res * 3;
}

/* Test UNLT (unordered or less than) - should generate ult */
int test_unlt(float a, float b) {
    volatile float va = a;
    volatile float vb = b;
    
    int res = __builtin_isunordered(va, vb) || (va < vb);
    
    /* Use in array indexing */
    int array[2] = {5, 10};
    return array[res & 1];
}

/* Test LTGT (less than or greater than) - should generate une */
int test_ltgt(double a, double b) {
    volatile double va = a;
    volatile double vb = b;
    
    /* Direct builtin */
    int res1 = __builtin_islessgreater(va, vb);
    
    /* Equivalent expression */
    int res2 = (va < vb) || (va > vb);
    
    /* Ordered comparison */
    int res3 = (va == va) && (vb == vb) && (va != vb);
    
    return res1 + res2 + res3;
}

/* Mixed precision tests */
int test_mixed_precision(float f, double d) {
    volatile float vf = f;
    volatile double vd = d;
    
    int res = 0;
    
    /* Mixed comparison */
    if (vf < vd) {
        res += 1;
    }
    
    /* Inverse mixed comparison */
    if (!(vf >= vd)) {
        res += 2;
    }
    
    /* Unordered check with mixed */
    if (__builtin_isunordered(vf, vd)) {
        res += 4;
    }
    
    return res;
}

/* Test with function returns */
int test_function_calls(void) {
    double nan_val = get_nan();
    float nanf_val = get_nanf();
    
    int res = 0;
    
    /* Compare function results */
    if (sqrt(-1.0) == nan_val) {  /* sqrt(-1) returns NaN */
        res += 1;
    }
    
    if (__builtin_isunordered(sin(nan_val), cos(0.0))) {
        res += 2;
    }
    
    return res;
}

/* Main driver with varied inputs */
int main(void) {
    float f1 = 1.0f;
    float f2 = 2.0f;
    float f_nan = get_nanf();
    float f_inf = INFINITY;
    
    double d1 = 3.0;
    double d2 = 4.0;
    double d_nan = get_nan();
    double d_inf = HUGE_VAL;
    
    int checksum = 0;
    
    /* Test with normal values */
    checksum += test_unordered(f1, f2);
    checksum += test_ordered(d1, d2);
    checksum += test_uneq(f1, f2);
    checksum += test_unge(d1, d2);
    checksum += test_ungt(f1, f2);
    checksum += test_unle(d1, d2);
    checksum += test_unlt(f1, f2);
    checksum += test_ltgt(d1, d2);
    
    /* Test with NaN values */
    checksum += test_unordered(f_nan, f1);
    checksum += test_ordered(d_nan, d1);
    checksum += test_uneq(f1, f_nan);
    checksum += test_unge(d_nan, d_nan);
    checksum += test_ungt(f_nan, f2);
    checksum += test_unle(d1, d_nan);
    checksum += test_unlt(f_nan, f_nan);
    checksum += test_ltgt(d_nan, d1);
    
    /* Test with infinity */
    checksum += test_unordered(f_inf, f1);
    checksum += test_ordered(d_inf, d2);
    checksum += test_uneq(f_inf, f_inf);
    
    /* Test mixed precision */
    checksum += test_mixed_precision(f1, d1);
    checksum += test_mixed_precision(f_nan, d2);
    checksum += test_mixed_precision(f_inf, d_nan);
    
    /* Test function calls */
    checksum += test_function_calls();
    
    /* Test with zero and negative zero */
    checksum += test_unordered(0.0f, -0.0f);
    checksum += test_uneq(0.0, -0.0);
    
    /* Test volatile variables */
    volatile float vf1 = 5.0f;
    volatile float vf2 = 6.0f;
    volatile double vd1 = 7.0;
    volatile double vd2 = 8.0;
    
    checksum += test_unordered(vf1, vf2);
    checksum += test_ordered(vd1, vd2);
    
    /* Output checksum to ensure all code executes */
    printf("Checksum: %d\n", checksum);
    
    /* Additional control flow based on comparisons */
    int final_result = 0;
    
    /* Complex conditional using multiple comparisons */
    if (test_unordered(f_nan, f1) > 0) {
        final_result += 1;
    }
    
    if (test_ltgt(d1, d2) > 0) {
        final_result += 2;
    }
    
    /* Ternary operator with comparison */
    final_result += test_uneq(f1, f2) ? 4 : 8;
    
    printf("Final result: %d\n", final_result);
    
    return 0;
}
