/* test_conditions.c - Program to trigger x86 floating-point condition code generation */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = 0.0/0.0;  /* Will become NAN */
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = 0.0f/0.0f;

/* Function to generate NAN */
double get_nan() {
    return __builtin_nan("");
}

float get_nanf() {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code */
int test_unordered(double a, double b) {
    /* __builtin_isunordered should generate "unord" */
    int res = __builtin_isunordered(a, b);
    /* Also test with !(a == a) pattern */
    int res2 = !(a == a) || !(b == b);
    return res + res2;
}

/* Test ORDERED condition code */
int test_ordered(float a, float b) {
    /* Ordered check - both are not NaN */
    int res = !__builtin_isunordered(a, b);
    /* Alternative: (a == a) && (b == b) */
    int res2 = (a == a) && (b == b);
    return res + res2;
}

/* Test UNEQ (unordered or equal) */
int test_uneq(double a, double b) {
    /* a == b or unordered */
    int res = (a == b) || __builtin_isunordered(a, b);
    return res;
}

/* Test UNGE (not less than) - generates "nlt" */
int test_unge(float a, float b) {
    /* !(a < b) includes unordered case */
    int res = !(a < b);
    return res;
}

/* Test UNGT (not less than or equal) - generates "nle" */
int test_ungt(double a, double b) {
    /* !(a <= b) */
    int res = !(a <= b);
    return res;
}

/* Test UNLE (unordered or less than or equal) - generates "ule" */
int test_unle(float a, float b) {
    /* (a <= b) || unordered */
    int res = (a <= b) || __builtin_isunordered(a, b);
    return res;
}

/* Test UNLT (unordered or less than) - generates "ult" */
int test_unlt(double a, double b) {
    /* (a < b) || unordered */
    int res = (a < b) || __builtin_isunordered(a, b);
    return res;
}

/* Test LTGT (less than or greater than) - generates "une" */
int test_ltgt(float a, float b) {
    /* __builtin_islessgreater generates "une" */
    int res = __builtin_islessgreater(a, b);
    /* Alternative: (a < b) || (a > b) with ordered check */
    int res2 = (!__builtin_isunordered(a, b) && (a < b || a > b));
    return res + res2;
}

/* Mixed precision tests */
int test_mixed_precision() {
    float f = vf1;
    double d = vd1;
    int res = 0;
    
    /* Mixed float/double comparisons */
    res += (f < d) || __builtin_isunordered(f, d);  /* May generate ult */
    res += !(f >= d);  /* May generate nlt */
    res += __builtin_islessgreater(f, d);  /* May generate une */
    
    return res;
}

/* Test with function returns (may produce NaN) */
int test_with_functions() {
    double nan_val = get_nan();
    float nanf_val = get_nanf();
    double normal = sqrt(4.0);
    double maybe_nan = sqrt(-1.0);  /* Returns NaN */
    
    int res = 0;
    
    /* These should trigger various condition codes */
    res += test_unordered(nan_val, normal);
    res += test_ordered(nanf_val, 1.0f);
    res += test_uneq(maybe_nan, 2.0);
    res += test_unge((float)maybe_nan, vf1);
    res += test_ungt(3.0, nan_val);
    res += test_unle(vf2, nanf_val);
    res += test_unlt(nan_val, 5.0);
    res += test_ltgt(1.0f, 2.0f);
    
    return res;
}

/* Test with constants including INFINITY and NAN */
int test_constants() {
    int res = 0;
    const double inf = INFINITY;
    const double neg_inf = -INFINITY;
    const double nan_const = NAN;
    
    /* Comparisons with infinity */
    res += !(1.0 < inf);      /* Should be true, may use nlt */
    res += (neg_inf <= 0.0) || __builtin_isunordered(neg_inf, 0.0);
    
    /* Comparisons with NaN constant */
    res += __builtin_isunordered(nan_const, 3.14);
    res += !__builtin_isunordered(2.71, 3.14);  /* Both ordered */
    
    /* Inverse comparisons */
    res += !(nan_const > 0.0);  /* nan > 0 is false, ! is true */
    res += !(0.0 <= nan_const); /* 0 <= nan is false, ! is true */
    
    return res;
}

/* Main driver that uses all tests */
int main() {
    int checksum = 0;
    
    /* Initialize NaN values */
    vd_nan = get_nan();
    vf_nan = get_nanf();
    
    printf("Starting floating-point condition code tests...\n");
    
    /* Test 1: Basic comparisons with volatile variables */
    checksum += test_unordered(vd_nan, vd1);
    checksum += test_ordered(vf1, vf2);
    checksum += test_uneq(vd_nan, vd_nan);  /* Both NaN -> unordered -> true */
    checksum += test_unge(vf2, vf1);        /* 2.0 < 1.0 is false, ! is true */
    checksum += test_ungt(vd1, vd2);        /* 1.0 <= 2.0 is true, ! is false */
    checksum += test_unle(vf_nan, vf1);     /* NaN <= 1.0 or unordered -> true */
    checksum += test_unlt(vd1, vd_nan);     /* 1.0 < NaN or unordered -> true */
    checksum += test_ltgt(vf1, vf2);        /* 1.0 < 2.0 -> true */
    
    /* Test 2: Mixed precision */
    checksum += test_mixed_precision();
    
    /* Test 3: Function returns (some produce NaN) */
    checksum += test_with_functions();
    
    /* Test 4: Constants including INFINITY and NAN */
    checksum += test_constants();
    
    /* Additional direct comparisons in control flow */
    double a = 1.5;
    double b = get_nan();
    
    /* if statements that should generate conditional jumps */
    if (__builtin_isunordered(a, b)) {
        checksum += 1;
    }
    
    if (!(a >= 2.0)) {  /* Should use "nlt" or similar */
        checksum += 2;
    }
    
    if (__builtin_islessgreater(a, 1.0)) {
        checksum += 4;
    }
    
    /* Ternary operator usage */
    float x = vf1;
    float y = vf_nan;
    int temp = (__builtin_isunordered(x, y)) ? 8 : 16;
    checksum += temp;
    
    /* While loop with comparison */
    int count = 0;
    while (count < 3 && !(x > y)) {  /* Inverse comparison */
        checksum += 32;
        count++;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    /* Return checksum mod 256 to avoid large exit codes */
    return checksum & 0xFF;
}
