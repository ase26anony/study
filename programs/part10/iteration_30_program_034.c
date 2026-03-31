/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = __builtin_nan("");
volatile double vd_inf = __builtin_inf();
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = __builtin_nanf("");

/* Function to generate NaN */
double make_nan() {
    return __builtin_nan("");
}

/* Function to generate infinity */
double make_inf() {
    return __builtin_inf();
}

/* Test UNORDERED condition code (unord) */
int test_unordered(double a, double b) {
    int result = 0;
    /* Direct unordered check */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    /* Alternative unordered check */
    if (a != a || b != b) {  /* NaN check */
        result |= 2;
    }
    /* Compare with volatile NaN */
    if (__builtin_isunordered(a, vd_nan)) {
        result |= 4;
    }
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered(double a, double b) {
    int result = 0;
    /* Direct ordered check */
    if (!__builtin_isunordered(a, b)) {
        result |= 1;
    }
    /* Ordered comparison after check */
    if (a == a && b == b) {  /* Both are not NaN */
        result |= 2;
    }
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq(double a, double b) {
    int result = 0;
    /* Unordered or equal */
    if (!(a < b) && !(a > b)) {  /* Includes NaN case */
        result |= 1;
    }
    /* Using builtin */
    if (__builtin_isunordered(a, b) || a == b) {
        result |= 2;
    }
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge(double a, double b) {
    int result = 0;
    /* Not less than (includes unordered) */
    if (!(a < b)) {
        result |= 1;
    }
    /* Inverse of less than */
    if (a >= b || a != a || b != b) {
        result |= 2;
    }
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt(double a, double b) {
    int result = 0;
    /* Not less than or equal (includes unordered) */
    if (!(a <= b)) {
        result |= 1;
    }
    /* Greater than or unordered */
    if (a > b || a != a || b != b) {
        result |= 2;
    }
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle(double a, double b) {
    int result = 0;
    /* Unordered or less than or equal */
    if (a <= b || a != a || b != b) {
        result |= 1;
    }
    /* Using builtin */
    if (__builtin_isunordered(a, b) || a <= b) {
        result |= 2;
    }
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt(double a, double b) {
    int result = 0;
    /* Unordered or less than */
    if (a < b || a != a || b != b) {
        result |= 1;
    }
    /* Using builtin */
    if (__builtin_isunordered(a, b) || a < b) {
        result |= 2;
    }
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt(double a, double b) {
    int result = 0;
    /* Less than or greater than (ordered, not equal) */
    if (__builtin_islessgreater(a, b)) {
        result |= 1;
    }
    /* Manual implementation */
    if ((a < b) || (a > b)) {
        result |= 2;
    }
    return result;
}

/* Mixed precision tests */
int test_mixed_precision(float a, double b) {
    int result = 0;
    
    /* Mixed precision unordered */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* Mixed precision ordered comparison */
    if (!__builtin_isunordered(a, b) && a < b) {
        result |= 2;
    }
    
    return result;
}

/* Test with function returns */
int test_function_calls() {
    int result = 0;
    double nan_val = make_nan();
    double inf_val = make_inf();
    
    /* Compare function results */
    if (__builtin_isunordered(nan_val, inf_val)) {
        result |= 1;
    }
    
    if (!__builtin_isunordered(inf_val, 1.0)) {
        result |= 2;
    }
    
    /* sqrt(-1) should produce NaN */
    if (__builtin_isunordered(sqrt(-1.0), 0.0)) {
        result |= 4;
    }
    
    return result;
}

/* Complex control flow to force conditional jumps */
int test_control_flow(double a, double b) {
    int result = 0;
    int i;
    
    /* Loop with floating-point condition */
    for (i = 0; i < 10; i++) {
        if (__builtin_isunordered(a + i, b)) {
            result += i * 2;
        } else if (!__builtin_isunordered(a, b) && a < b) {
            result += i * 3;
        }
    }
    
    /* Switch-like behavior using ternary */
    result += (__builtin_islessgreater(a, b)) ? 100 : 200;
    result += (!(a >= b)) ? 300 : 400;  /* Should generate nlt */
    
    return result;
}

/* Main test driver */
int main() {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Test with various inputs */
    double test_values[] = {0.0, -0.0, 1.0, -1.0, 2.0, NAN, INFINITY};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    for (int i = 0; i < num_tests; i++) {
        for (int j = 0; j < num_tests; j++) {
            double a = test_values[i];
            double b = test_values[j];
            
            checksum += test_unordered(a, b);
            checksum += test_ordered(a, b);
            checksum += test_uneq(a, b);
            checksum += test_unge(a, b);
            checksum += test_ungt(a, b);
            checksum += test_unle(a, b);
            checksum += test_unlt(a, b);
            checksum += test_ltgt(a, b);
            
            /* Test with volatile variables */
            checksum += test_unordered(vd1, vd_nan);
            checksum += test_ordered(vd1, vd2);
            checksum += test_uneq(vd_nan, vd_nan);
            checksum += test_unge(vd_inf, vd1);
            checksum += test_ungt(vd1, vd_inf);
        }
    }
    
    /* Test mixed precision */
    checksum += test_mixed_precision(vf1, vd2);
    checksum += test_mixed_precision(vf_nan, vd1);
    
    /* Test function calls */
    checksum += test_function_calls();
    
    /* Test control flow */
    checksum += test_control_flow(1.0, 2.0);
    checksum += test_control_flow(NAN, 1.0);
    checksum += test_control_flow(1.0, NAN);
    
    /* Array indexing based on FP comparisons */
    int array[4] = {0};
    for (int i = 0; i < 4; i++) {
        double val = test_values[i % num_tests];
        int idx = (__builtin_isunordered(val, 0.0)) ? 0 : 
                  (!__builtin_isunordered(val, 0.0) && val < 0.0) ? 1 : 2;
        array[idx]++;
        checksum += array[idx];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
