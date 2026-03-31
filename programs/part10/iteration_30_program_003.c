/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = 0.0/0.0;  /* Will become NaN */
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;

/* Function to generate NaN */
double make_nan() {
    return __builtin_nan("");
}

/* Function to generate infinity */
double make_inf() {
    return __builtin_inf();
}

/* Test UNORDERED condition (unord) */
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
    return result;
}

/* Test ORDERED condition (ord) */
int test_ordered(double a, double b) {
    int result = 0;
    /* Ordered check - both are not NaN */
    if (!__builtin_isunordered(a, b)) {
        result |= 1;
    }
    /* Using builtin */
    if (__builtin_islessgreater(a, b) || a == b) {
        result |= 2;
    }
    return result;
}

/* Test UNEQ condition (ueq) */
int test_uneq(double a, double b) {
    int result = 0;
    /* Unordered or equal */
    if (__builtin_isunordered(a, b) || a == b) {
        result |= 1;
    }
    /* Alternative using !(a != b) with NaN consideration */
    if (!(a != b)) {  /* This includes NaN != NaN case */
        result |= 2;
    }
    return result;
}

/* Test UNGE condition (nlt) */
int test_unge(double a, double b) {
    int result = 0;
    /* Not less than (includes unordered) */
    if (!(a < b)) {
        result |= 1;
    }
    /* Greater than or equal or unordered */
    if (a >= b || __builtin_isunordered(a, b)) {
        result |= 2;
    }
    return result;
}

/* Test UNGT condition (nle) */
int test_ungt(double a, double b) {
    int result = 0;
    /* Not less than or equal (includes unordered) */
    if (!(a <= b)) {
        result |= 1;
    }
    /* Greater than or unordered */
    if (a > b || __builtin_isunordered(a, b)) {
        result |= 2;
    }
    return result;
}

/* Test UNLE condition (ule) */
int test_unle(double a, double b) {
    int result = 0;
    /* Unordered or less than or equal */
    if (__builtin_isunordered(a, b) || a <= b) {
        result |= 1;
    }
    return result;
}

/* Test UNLT condition (ult) */
int test_unlt(double a, double b) {
    int result = 0;
    /* Unordered or less than */
    if (__builtin_isunordered(a, b) || a < b) {
        result |= 1;
    }
    return result;
}

/* Test LTGT condition (une) */
int test_ltgt(double a, double b) {
    int result = 0;
    /* Less than or greater than (ordered, not equal) */
    if (__builtin_islessgreater(a, b)) {
        result |= 1;
    }
    /* Alternative: ordered and not equal */
    if (!__builtin_isunordered(a, b) && a != b) {
        result |= 2;
    }
    return result;
}

/* Mixed precision tests */
int test_mixed_precision(float f, double d) {
    int result = 0;
    
    /* Promote float to double and compare */
    if (__builtin_isunordered(f, d)) {
        result |= 1;
    }
    
    if (!(f < d)) {  /* UNGE (nlt) */
        result |= 2;
    }
    
    if (__builtin_islessgreater(f, d)) {  /* LTGT (une) */
        result |= 4;
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
    
    if (!__builtin_isunordered(sqrt(-1.0), 0.0)) {
        result |= 2;
    }
    
    return result;
}

/* Test with constants */
int test_constants() {
    int result = 0;
    volatile double x = vd1;
    
    /* Compare with various constants */
    if (__builtin_isunordered(x, NAN)) {
        result |= 1;
    }
    
    if (!(x < INFINITY)) {  /* UNGE (nlt) */
        result |= 2;
    }
    
    if (x != 0.0 || __builtin_isunordered(x, 0.0)) {  /* UNEQ variant */
        result |= 4;
    }
    
    return result;
}

/* Complex control flow to force conditional jumps */
void complex_control_flow(double a, double b, int *arr) {
    int idx = 0;
    
    /* Use comparisons to compute array index */
    if (__builtin_isunordered(a, b)) {
        idx = 0;
    } else if (!(a < b)) {  /* UNGE */
        idx = 1;
    } else if (__builtin_islessgreater(a, b)) {  /* LTGT */
        idx = 2;
    } else if (__builtin_isunordered(a, b) || a <= b) {  /* UNLE */
        idx = 3;
    }
    
    arr[idx] += 1;
}

/* Main test driver */
int main() {
    int checksum = 0;
    int arr[4] = {0, 0, 0, 0};
    
    /* Test with NaN values */
    double nan1 = make_nan();
    double nan2 = make_nan();
    double normal = 3.14;
    double inf = make_inf();
    
    printf("Starting condition code tests...\n");
    
    /* Run all tests with various inputs */
    checksum ^= test_unordered(nan1, normal);
    checksum ^= test_unordered(normal, nan2);
    checksum ^= test_unordered(nan1, nan2);
    
    checksum ^= test_ordered(normal, inf);
    checksum ^= test_ordered(normal, normal);
    
    checksum ^= test_uneq(nan1, normal);
    checksum ^= test_uneq(normal, normal);
    checksum ^= test_uneq(nan1, nan2);
    
    checksum ^= test_unge(normal, inf);
    checksum ^= test_unge(inf, normal);
    checksum ^= test_unge(nan1, normal);
    
    checksum ^= test_ungt(normal, normal);
    checksum ^= test_ungt(inf, normal);
    checksum ^= test_ungt(nan1, normal);
    
    checksum ^= test_unle(normal, inf);
    checksum ^= test_unle(normal, normal);
    checksum ^= test_unle(nan1, normal);
    
    checksum ^= test_unlt(normal, inf);
    checksum ^= test_unlt(normal, normal);
    checksum ^= test_unlt(nan1, normal);
    
    checksum ^= test_ltgt(normal, inf);
    checksum ^= test_ltgt(inf, normal);
    checksum ^= test_ltgt(normal, normal);
    
    checksum ^= test_mixed_precision(vf1, vd2);
    checksum ^= test_mixed_precision(vf2, vd_nan);
    
    checksum ^= test_function_calls();
    checksum ^= test_constants();
    
    /* Complex control flow tests */
    complex_control_flow(nan1, normal, arr);
    complex_control_flow(normal, inf, arr);
    complex_control_flow(inf, normal, arr);
    complex_control_flow(normal, normal, arr);
    
    /* Add array values to checksum */
    for (int i = 0; i < 4; i++) {
        checksum ^= arr[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum == 0 ? 0 : 1;
}
