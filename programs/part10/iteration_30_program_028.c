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
volatile double vd_nan = __builtin_nan("");

/* Test UNORDERED condition code (unord) */
int test_unordered(float a, float b) {
    /* Direct unordered check - should generate "unord" */
    int res1 = __builtin_isunordered(a, b);
    
    /* Alternative unordered check */
    int res2 = !(a == a) || !(b == b);
    
    /* Compare with NaN */
    float nan = __builtin_nanf("");
    int res3 = __builtin_isunordered(a, nan);
    
    return res1 + res2 + res3;
}

/* Test ORDERED condition code (ord) */
int test_ordered(double a, double b) {
    /* Ordered check - should generate "ord" */
    int res1 = !__builtin_isunordered(a, b);
    
    /* Alternative ordered check */
    int res2 = (a == a) && (b == b);
    
    /* Complex ordered expression */
    int res3 = (a < b) || (a >= b); /* Both sides assume ordered */
    
    return res1 + res2 + res3;
}

/* Test UNEQ condition code (ueq) */
int test_uneq(float a, float b) {
    /* Unordered or equal - should generate "ueq" */
    int res1 = __builtin_isunordered(a, b) || (a == b);
    
    /* Using !(a != b) which includes unordered case */
    int res2 = !(a != b);
    
    return res1 + res2;
}

/* Test UNGE condition code (nlt) */
int test_unge(double a, double b) {
    /* Not less than (unordered or greater or equal) - should generate "nlt" */
    int res1 = !(a < b);
    
    /* Alternative: greater than or equal or unordered */
    int res2 = (a >= b) || __builtin_isunordered(a, b);
    
    return res1 + res2;
}

/* Test UNGT condition code (nle) */
int test_ungt(float a, float b) {
    /* Not less than or equal (unordered or greater) - should generate "nle" */
    int res1 = !(a <= b);
    
    /* Alternative: greater than or unordered */
    int res2 = (a > b) || __builtin_isunordered(a, b);
    
    return res1 + res2;
}

/* Test UNLE condition code (ule) */
int test_unle(double a, double b) {
    /* Unordered or less or equal - should generate "ule" */
    int res1 = __builtin_isunordered(a, b) || (a <= b);
    
    /* Using !(a > b) which includes unordered case */
    int res2 = !(a > b);
    
    return res1 + res2;
}

/* Test UNLT condition code (ult) */
int test_unlt(float a, float b) {
    /* Unordered or less than - should generate "ult" */
    int res1 = __builtin_isunordered(a, b) || (a < b);
    
    /* Using !(a >= b) which includes unordered case */
    int res2 = !(a >= b);
    
    return res1 + res2;
}

/* Test LTGT condition code (une) */
int test_ltgt(double a, double b) {
    /* Less than or greater than (but not equal, not unordered) - should generate "une" */
    int res1 = __builtin_islessgreater(a, b);
    
    /* Alternative: (a < b) || (a > b) with ordered assumption */
    int res2 = (a < b) || (a > b);
    
    /* Complex expression that might trigger LTGT */
    int res3 = (a != b) && !__builtin_isunordered(a, b);
    
    return res1 + res2 + res3;
}

/* Mixed precision tests */
int test_mixed_precision(float f, double d) {
    int result = 0;
    
    /* Mixed precision unordered check */
    result += __builtin_isunordered(f, (float)d);
    
    /* Mixed precision ordered comparison */
    result += !__builtin_isunordered((double)f, d);
    
    /* Mixed with constants */
    result += __builtin_isunordered(f, 0.0f);
    result += !__builtin_isunordered(d, 1.0);
    
    return result;
}

/* Function calls that may return NaN */
double maybe_nan(int flag) {
    if (flag) {
        return sqrt(-1.0); /* Returns NaN */
    }
    return 3.14159;
}

float maybe_nanf(int flag) {
    if (flag) {
        return 0.0f / 0.0f; /* Returns NaN */
    }
    return 2.71828f;
}

/* Test with function return values */
int test_function_calls() {
    int result = 0;
    
    double d1 = maybe_nan(0);
    double d2 = maybe_nan(1); /* NaN */
    
    result += __builtin_isunordered(d1, d2);
    result += !__builtin_isunordered(d1, 0.0);
    result += __builtin_islessgreater(d1, d2);
    
    float f1 = maybe_nanf(0);
    float f2 = maybe_nanf(1); /* NaN */
    
    result += __builtin_isunordered(f1, f2);
    result += !(f1 <= f2); /* Should generate "nle" */
    
    return result;
}

/* Control flow based on comparisons */
void control_flow_tests() {
    float f = 1.5f;
    double d = 2.5;
    
    /* if statements that should generate conditional jumps */
    if (__builtin_isunordered(f, vf_nan)) {
        printf("f is unordered with NaN\n");
    }
    
    if (!__builtin_isunordered(d, 0.0)) {
        printf("d is ordered with 0.0\n");
    }
    
    /* Ternary operator */
    int x = (__builtin_islessgreater(f, 2.0f)) ? 1 : 0;
    
    /* While loop with comparison */
    float counter = 0.0f;
    while (!__builtin_isunordered(counter, 10.0f)) {
        counter += 1.0f;
    }
}

/* Main driver function */
int main() {
    int checksum = 0;
    
    /* Test with various inputs including NaN */
    float f_nan = __builtin_nanf("");
    double d_nan = __builtin_nan("");
    
    printf("Starting floating-point condition code tests...\n");
    
    /* Test all condition codes with different value combinations */
    checksum += test_unordered(1.0f, 2.0f);
    checksum += test_unordered(vf_nan, vf1);
    checksum += test_unordered(vf1, vf_nan);
    
    checksum += test_ordered(3.0, 4.0);
    checksum += test_ordered(vd_nan, vd1);
    checksum += test_ordered(vd1, vd_nan);
    
    checksum += test_uneq(1.0f, 1.0f);
    checksum += test_uneq(vf_nan, vf2);
    checksum += test_uneq(vf1, vf_nan);
    
    checksum += test_unge(5.0, 3.0);
    checksum += test_unge(vd_nan, vd1);
    checksum += test_unge(vd1, vd_nan);
    
    checksum += test_ungt(4.0f, 3.0f);
    checksum += test_ungt(vf_nan, vf1);
    checksum += test_ungt(vf1, vf_nan);
    
    checksum += test_unle(2.0, 3.0);
    checksum += test_unle(vd_nan, vd1);
    checksum += test_unle(vd1, vd_nan);
    
    checksum += test_unlt(1.0f, 2.0f);
    checksum += test_unlt(vf_nan, vf1);
    checksum += test_unlt(vf1, vf_nan);
    
    checksum += test_ltgt(2.0, 3.0);
    checksum += test_ltgt(vd_nan, vd1);
    checksum += test_ltgt(vd1, vd_nan);
    
    /* Mixed precision tests */
    checksum += test_mixed_precision(1.0f, 2.0);
    checksum += test_mixed_precision(vf_nan, vd1);
    checksum += test_mixed_precision(vf1, vd_nan);
    
    /* Function call tests */
    checksum += test_function_calls();
    
    /* Control flow tests */
    control_flow_tests();
    
    /* Array indexing based on comparisons */
    int array[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++) {
        float f1 = i * 1.0f;
        float f2 = (i + 1) * 1.0f;
        
        /* Use comparison result to determine array index */
        int idx = __builtin_isunordered(f1, f2) ? 0 : 
                 __builtin_islessgreater(f1, f2) ? 1 :
                 (f1 == f2) ? 2 : 3;
        
        array[idx]++;
    }
    
    checksum += array[0] + array[1] + array[2] + array[3];
    
    printf("Final checksum: %d\n", checksum);
    
    /* Return checksum modulo 256 to avoid overflow issues */
    return checksum & 0xFF;
}
