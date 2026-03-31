/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Helper to generate NaN */
static double make_nan() {
    return __builtin_nan("");
}

/* Helper to generate infinity */
static double make_inf() {
    return __builtin_inf();
}

/* Test UNORDERED condition code */
void test_unordered(float a, float b) {
    volatile float x = a;
    volatile float y = b;
    
    /* Direct unordered check - should generate "unord" */
    if (__builtin_isunordered(x, y)) {
        printf("U");
    } else {
        printf("O");
    }
    
    /* Alternative unordered check */
    int res = !(x == x) || !(y == y);
    if (res) {
        printf("N");
    }
}

/* Test ORDERED condition code */
void test_ordered(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    
    /* Ordered check - should generate "ord" */
    if (__builtin_isordered(x, y)) {
        printf("R");
    } else {
        printf("D");
    }
}

/* Test UNEQ (unordered or equal) */
void test_uneq(float a, float b) {
    volatile float x = a;
    volatile float y = b;
    
    /* This should generate "ueq" */
    if (!(x != y)) {  /* Equivalent to x == y including NaNs */
        printf("E");
    }
    
    /* Alternative using builtin */
    if (__builtin_isunordered(x, y) || x == y) {
        printf("Q");
    }
}

/* Test UNGE (not less than) - generates "nlt" */
void test_unge(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    
    /* !(x < y) generates "nlt" */
    if (!(x < y)) {
        printf("G");
    }
    
    /* Alternative: x >= y including unordered */
    if (x >= y || __builtin_isunordered(x, y)) {
        printf("N");
    }
}

/* Test UNGT (not less than or equal) - generates "nle" */
void test_ungt(float a, float b) {
    volatile float x = a;
    volatile float y = b;
    
    /* !(x <= y) generates "nle" */
    if (!(x <= y)) {
        printf("T");
    }
    
    /* x > y including unordered */
    if (x > y || __builtin_isunordered(x, y)) {
        printf("X");
    }
}

/* Test UNLE (unordered or less than or equal) - generates "ule" */
void test_unle(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    
    /* x <= y including unordered */
    if (x <= y || __builtin_isunordered(x, y)) {
        printf("L");
    }
}

/* Test UNLT (unordered or less than) - generates "ult" */
void test_unlt(float a, float b) {
    volatile float x = a;
    volatile float y = b;
    
    /* x < y including unordered */
    if (x < y || __builtin_isunordered(x, y)) {
        printf("S");
    }
}

/* Test LTGT (less than or greater than) - generates "une" */
void test_ltgt(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    
    /* Direct builtin for lessgreater */
    if (__builtin_islessgreater(x, y)) {
        printf("B");
    }
    
    /* Manual implementation: ordered and not equal */
    if ((x < y) || (x > y)) {
        printf("M");
    }
}

/* Mixed precision tests */
void test_mixed_precision(float f, double d) {
    volatile float vf = f;
    volatile double vd = d;
    
    /* Mixed precision comparisons */
    if (__builtin_isunordered(vf, vd)) {
        printf("M");
    }
    
    if (!(vf < vd)) {  /* Should generate "nlt" */
        printf("P");
    }
}

/* Test with function returns */
void test_function_calls() {
    /* sqrt(-1) returns NaN */
    double nan_val = sqrt(-1.0);
    double inf_val = 1.0 / 0.0;
    
    /* These should trigger various condition codes */
    if (__builtin_isunordered(nan_val, 0.0)) {
        printf("F");
    }
    
    if (!(inf_val <= 0.0)) {  /* Should generate "nle" */
        printf("I");
    }
    
    if (__builtin_islessgreater(inf_val, nan_val)) {
        printf("C");
    }
}

/* Test with constants */
void test_constants() {
    volatile float f1 = 0.0f;
    volatile float f2 = -0.0f;
    volatile double d1 = NAN;
    volatile double d2 = INFINITY;
    
    /* -0.0 == 0.0 is true, but let's test unordered */
    if (__builtin_isunordered(f1, f2)) {
        printf("Z");
    }
    
    /* NAN compared to INFINITY */
    if (!(d1 < d2)) {  /* Should generate "nlt" */
        printf("A");
    }
    
    /* INFINITY compared to itself */
    if (d2 >= d2 || __builtin_isunordered(d2, d2)) {  /* Should generate UNGE */
        printf("V");
    }
}

/* Main driver that calls all tests */
int main() {
    float f1 = 1.5f;
    float f2 = 2.5f;
    float f_nan = make_nan();
    float f_inf = make_inf();
    
    double d1 = 3.14159;
    double d2 = 2.71828;
    double d_nan = make_nan();
    double d_inf = make_inf();
    
    printf("Starting condition code tests...\n");
    
    /* Test with normal values */
    test_unordered(f1, f2);
    test_ordered(d1, d2);
    test_uneq(f1, f1);  /* Equal values */
    test_unge(d1, d2);
    test_ungt(f2, f1);
    test_unle(d2, d1);
    test_unlt(f1, f2);
    test_ltgt(d1, d2);
    
    /* Test with NaN */
    test_unordered(f_nan, f1);
    test_ordered(d_nan, d1);
    test_uneq(f_nan, f_nan);
    test_unge(d_nan, d_inf);
    test_ungt(f_inf, f_nan);
    test_unle(d1, d_nan);
    test_unlt(f_nan, f_inf);
    test_ltgt(d_inf, d_nan);
    
    /* Mixed tests */
    test_mixed_precision(f1, d1);
    test_mixed_precision(f_nan, d_inf);
    
    /* Function call tests */
    test_function_calls();
    
    /* Constant tests */
    test_constants();
    
    /* Create a checksum based on all comparisons */
    volatile float checksum_f = 0.0f;
    volatile double checksum_d = 0.0;
    
    /* Perform various comparisons that affect checksum */
    if (__builtin_isunordered(f_nan, f1)) checksum_f += 1.0f;
    if (!(d_inf <= d_nan)) checksum_d += 2.0;  /* nle */
    if (__builtin_islessgreater(f1, f2)) checksum_f += 3.0f;
    if (f_inf >= f_inf || __builtin_isunordered(f_inf, f_inf)) checksum_f += 4.0f;  /* UNGE */
    
    printf("\nChecksum: f=%f, d=%f\n", checksum_f, checksum_d);
    printf("Tests completed.\n");
    
    return (int)(checksum_f + checksum_d) % 256;
}
