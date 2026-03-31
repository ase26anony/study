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

/* Function to generate NaN */
double make_nan() {
    return __builtin_nan("");
}

float make_nanf() {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code */
int test_unordered(double a, double b) {
    int result = 0;
    /* Direct unordered check - should generate "unord" */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    /* Alternative unordered check */
    if (a != a || b != b) {  /* NaN check */
        result |= 2;
    }
    return result;
}

/* Test ORDERED condition code */
int test_ordered(double a, double b) {
    int result = 0;
    /* Ordered check - should generate "ord" */
    if (!__builtin_isunordered(a, b)) {
        result |= 1;
    }
    /* Ordered comparison */
    if (a == a && b == b) {  /* Both are not NaN */
        result |= 2;
    }
    return result;
}

/* Test UNEQ (unordered or equal) condition code */
int test_uneq(double a, double b) {
    int result = 0;
    /* Unordered or equal */
    if (__builtin_isunordered(a, b) || a == b) {
        result |= 1;
    }
    return result;
}

/* Test UNGE (not less than) condition code */
int test_unge(double a, double b) {
    int result = 0;
    /* Not less than - should generate "nlt" */
    if (!(a < b)) {
        result |= 1;
    }
    /* Alternative: greater than or equal */
    if (a >= b) {
        result |= 2;  /* May also generate "nlt" */
    }
    return result;
}

/* Test UNGT (not less than or equal) condition code */
int test_ungt(double a, double b) {
    int result = 0;
    /* Not less than or equal - should generate "nle" */
    if (!(a <= b)) {
        result |= 1;
    }
    /* Alternative: greater than */
    if (a > b) {
        result |= 2;  /* May also generate "nle" */
    }
    return result;
}

/* Test UNLE (unordered or less than or equal) condition code */
int test_unle(double a, double b) {
    int result = 0;
    /* Unordered or less than or equal - should generate "ule" */
    if (__builtin_isunordered(a, b) || a <= b) {
        result |= 1;
    }
    return result;
}

/* Test UNLT (unordered or less than) condition code */
int test_unlt(double a, double b) {
    int result = 0;
    /* Unordered or less than - should generate "ult" */
    if (__builtin_isunordered(a, b) || a < b) {
        result |= 1;
    }
    return result;
}

/* Test LTGT (less than or greater than, ordered) condition code */
int test_ltgt(double a, double b) {
    int result = 0;
    /* Less than or greater than (ordered) - should generate "une" */
    if (__builtin_islessgreater(a, b)) {
        result |= 1;
    }
    /* Alternative: (a < b) || (a > b) with ordered check */
    if ((a < b) || (a > b)) {
        result |= 2;
    }
    return result;
}

/* Mixed precision tests */
int test_mixed_precision(float a, double b) {
    int result = 0;
    /* Mixed float/double comparisons */
    if (__builtin_isunordered(a, b)) result |= 1;
    if (!(a < b)) result |= 2;      /* UNGE/nlt */
    if (!(a <= b)) result |= 4;     /* UNGT/nle */
    if (__builtin_islessgreater(a, b)) result |= 8;  /* LTGT/une */
    return result;
}

/* Test with function returns (may produce NaN) */
int test_function_calls() {
    int result = 0;
    double d1 = sqrt(-1.0);  /* Returns NaN */
    double d2 = 1.0 / 0.0;   /* Infinity */
    float f1 = sqrtf(-1.0f);
    
    /* Various comparisons with NaN results */
    if (__builtin_isunordered(d1, d2)) result |= 1;
    if (!__builtin_isunordered(f1, 0.0f)) result |= 2;
    if (__builtin_islessgreater(d1, d2)) result |= 4;
    if (!(d1 < d2)) result |= 8;  /* UNGE/nlt with NaN */
    
    return result;
}

/* Test with constants */
int test_constants() {
    int result = 0;
    const double inf = INFINITY;
    const double neg_inf = -INFINITY;
    const double nan = NAN;
    const double zero = 0.0;
    const double neg_zero = -0.0;
    
    /* Compare various constants */
    if (__builtin_isunordered(nan, inf)) result |= 1;
    if (!(zero < nan)) result |= 2;           /* UNGE/nlt */
    if (!(inf <= neg_inf)) result |= 4;       /* UNGT/nle */
    if (__builtin_islessgreater(zero, neg_zero)) result |= 8;  /* LTGT/une (they're equal) */
    if (__builtin_isunordered(zero, neg_zero)) result |= 16;   /* UNORDERED (they're ordered) */
    
    return result;
}

/* Main test driver */
int main() {
    int checksum = 0;
    
    printf("Testing floating-point condition codes...\n");
    
    /* Test with volatile variables */
    checksum ^= test_unordered(vd1, vd_nan);
    checksum ^= test_ordered(vd1, vd2);
    checksum ^= test_uneq(vd_nan, vd2);
    checksum ^= test_unge(vd1, vd2);
    checksum ^= test_ungt(vd2, vd1);
    checksum ^= test_unle(vd_nan, vd1);
    checksum ^= test_unlt(vd1, vd_nan);
    checksum ^= test_ltgt(vd1, vd2);
    
    /* Test with float volatile variables */
    checksum ^= test_unordered(vf1, vf_nan);
    checksum ^= test_ordered(vf1, vf2);
    
    /* Mixed precision */
    checksum ^= test_mixed_precision(vf1, vd2);
    checksum ^= test_mixed_precision(vf_nan, vd1);
    
    /* Function calls */
    checksum ^= test_function_calls();
    
    /* Constants */
    checksum ^= test_constants();
    
    /* Additional tests with computed values */
    double d1 = 3.14159;
    double d2 = 2.71828;
    float f1 = 1.41421f;
    
    checksum ^= test_unordered(d1, make_nan());
    checksum ^= test_ordered(d1, d2);
    checksum ^= test_uneq(make_nan(), d2);
    checksum ^= test_unge(d1, d2);
    checksum ^= test_ungt(d2, d1);
    checksum ^= test_unle(make_nanf(), f1);
    checksum ^= test_unlt(f1, make_nanf());
    checksum ^= test_ltgt(d1, d2);
    
    /* Use results in control flow */
    if (test_unordered(d1, make_nan()) & 1) {
        checksum += 1000;
    }
    
    /* Ternary operator usage */
    int res = (__builtin_isunordered(vd1, vd_nan)) ? 42 : 24;
    checksum ^= res;
    
    /* While loop with floating-point condition */
    int counter = 0;
    double x = 0.0;
    while (x < 10.0 && !__builtin_isunordered(x, make_nan())) {
        x += 1.0;
        counter++;
    }
    checksum ^= counter;
    
    printf("Final checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates tests executed)\n");
    
    return checksum != 0 ? 0 : 1;
}
