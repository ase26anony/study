/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile double vd_nan = __builtin_nan("");
volatile float vf_nan = __builtin_nanf("");

/* Function to generate UNORDERED condition code */
int test_unordered(double a, double b) {
    /* Direct unordered check - should generate "unord" */
    int res = __builtin_isunordered(a, b);
    
    /* Alternative unordered check */
    if (a != a || b != b) {
        res |= 1;
    }
    
    /* Use in control flow */
    if (__builtin_isunordered(a, b)) {
        return res + 1;
    }
    return res;
}

/* Function to generate ORDERED condition code */
int test_ordered(float a, float b) {
    /* Ordered check - should generate "ord" */
    int res = !__builtin_isunordered(a, b);
    
    /* Use ordered check in ternary operator */
    return (a == a && b == b) ? res + 10 : res;
}

/* Function to generate UNEQ condition code */
int test_uneq(double a, double b) {
    /* Unordered or equal - should generate "ueq" */
    int res = __builtin_isunordered(a, b) || (a == b);
    
    /* Use in while loop condition */
    int count = 0;
    while (!(__builtin_isunordered(a, b) || (a == b)) && count < 1) {
        count++;
    }
    
    return res + count;
}

/* Function to generate UNGE condition code */
int test_unge(float a, float b) {
    /* Unordered or greater-or-equal - should generate "nlt" */
    int res = __builtin_isunordered(a, b) || (a >= b);
    
    /* Inverse condition approach */
    if (!(a < b)) {
        res |= 2;
    }
    
    return res;
}

/* Function to generate UNGT condition code */
int test_ungt(double a, double b) {
    /* Unordered or greater - should generate "nle" */
    int res = __builtin_isunordered(a, b) || (a > b);
    
    /* Inverse condition */
    if (!(a <= b)) {
        res |= 4;
    }
    
    return res;
}

/* Function to generate UNLE condition code */
int test_unle(float a, float b) {
    /* Unordered or less-or-equal - should generate "ule" */
    int res = __builtin_isunordered(a, b) || (a <= b);
    
    /* Use in array indexing */
    static const int table[2] = {100, 200};
    int idx = (__builtin_isunordered(a, b) || (a <= b)) ? 0 : 1;
    return res + table[idx];
}

/* Function to generate UNLT condition code */
int test_unlt(double a, double b) {
    /* Unordered or less - should generate "ult" */
    int res = __builtin_isunordered(a, b) || (a < b);
    
    /* Complex expression to prevent optimization */
    return res + ((a < b) ? 0 : 1);
}

/* Function to generate LTGT condition code */
int test_ltgt(float a, float b) {
    /* Less or greater (ordered and not equal) - should generate "une" */
    int res = __builtin_islessgreater(a, b);
    
    /* Alternative implementation */
    if ((a < b) || (a > b)) {
        res |= 8;
    }
    
    return res;
}

/* Mixed precision tests */
int test_mixed_precision(double d, float f) {
    int result = 0;
    
    /* Mixed unordered check */
    result += __builtin_isunordered(d, f);
    
    /* Mixed ordered comparison */
    if (!__builtin_isunordered(d, f) && d > f) {
        result += 2;
    }
    
    return result;
}

/* Test with function returns that may produce NaN */
double maybe_nan(int source) {
    if (source == 0) return __builtin_nan("");
    if (source == 1) return sqrt(-1.0);
    if (source == 2) return 0.0 / 0.0;
    return 3.14;
}

float maybe_nanf(int source) {
    if (source == 0) return __builtin_nanf("");
    if (source == 1) return sqrtf(-1.0f);
    return 2.71f;
}

/* Main test driver */
int main() {
    int checksum = 0;
    
    /* Test with various inputs including NaN */
    double d1 = 1.5;
    double d2 = 2.5;
    double d_nan = __builtin_nan("");
    double d_inf = INFINITY;
    
    float f1 = 1.5f;
    float f2 = 2.5f;
    float f_nan = __builtin_nanf("");
    float f_zero = 0.0f;
    
    /* Test all condition codes with different inputs */
    checksum += test_unordered(d1, d_nan);
    checksum += test_unordered(d_nan, d2);
    checksum += test_unordered(d1, d2);
    checksum += test_unordered(vd_nan, vd1);
    
    checksum += test_ordered(f1, f2);
    checksum += test_ordered(f_nan, f1);
    checksum += test_ordered(f1, f_nan);
    checksum += test_ordered(vf1, vf2);
    
    checksum += test_uneq(d1, d1);
    checksum += test_uneq(d_nan, d2);
    checksum += test_uneq(d1, d2);
    checksum += test_uneq(d_inf, d_inf);
    
    checksum += test_unge(f1, f2);
    checksum += test_unge(f2, f1);
    checksum += test_unge(f_nan, f1);
    checksum += test_unge(f1, f_nan);
    
    checksum += test_ungt(d1, d2);
    checksum += test_ungt(d2, d1);
    checksum += test_ungt(d_nan, d1);
    checksum += test_ungt(d1, d_nan);
    
    checksum += test_unle(f1, f2);
    checksum += test_unle(f2, f1);
    checksum += test_unle(f_nan, f1);
    checksum += test_unle(f1, f_zero);
    
    checksum += test_unlt(d1, d2);
    checksum += test_unlt(d2, d1);
    checksum += test_unlt(d_nan, d1);
    checksum += test_unlt(d1, d_inf);
    
    checksum += test_ltgt(f1, f2);
    checksum += test_ltgt(f2, f1);
    checksum += test_ltgt(f1, f1);
    checksum += test_ltgt(f_nan, f1);
    
    /* Mixed precision tests */
    checksum += test_mixed_precision(d1, f1);
    checksum += test_mixed_precision(d_nan, f1);
    checksum += test_mixed_precision(d1, f_nan);
    
    /* Tests with function calls that may return NaN */
    checksum += test_unordered(maybe_nan(0), maybe_nan(1));
    checksum += test_ordered(maybe_nanf(0), maybe_nanf(1));
    checksum += test_ltgt(maybe_nan(2), maybe_nan(3));
    
    /* Use volatile variables in comparisons */
    checksum += test_unordered(vd1, vd_nan);
    checksum += test_ordered(vf1, vf_nan);
    checksum += test_uneq(vd_nan, vd2);
    
    /* Special cases: -0.0 vs 0.0 */
    checksum += test_uneq(0.0, -0.0);
    checksum += test_unordered(0.0, -0.0);
    
    /* Output checksum to ensure all comparisons are live */
    printf("Checksum: %d\n", checksum);
    
    /* Additional print to prevent dead code elimination */
    printf("Test completed. Generated checksum based on %ld comparisons.\n", 
           (long)(checksum % 1000));
    
    return 0;
}
