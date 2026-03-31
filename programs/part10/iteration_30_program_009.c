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

/* Function to generate UNORDERED condition code (unord) */
int test_unordered(double a, double b) {
    /* Direct unordered check - should generate "unord" */
    if (__builtin_isunordered(a, b)) {
        return 1;
    }
    /* Alternative unordered check */
    if (a != a || b != b) {  /* NaN check */
        return 2;
    }
    return 0;
}

/* Function to generate ORDERED condition code (ord) */
int test_ordered(double a, double b) {
    /* Ordered check - should generate "ord" */
    if (__builtin_isordered(a, b)) {
        return 1;
    }
    return 0;
}

/* Function to generate UNEQ condition code (ueq) */
int test_uneq(double a, double b) {
    /* Unordered or equal - should generate "ueq" */
    if (!__builtin_islessgreater(a, b)) {  /* !(a < b || a > b) */
        return 1;
    }
    return 0;
}

/* Function to generate UNGE condition code (nlt) */
int test_unge(double a, double b) {
    /* Not less than - should generate "nlt" */
    if (!(a < b)) {
        return 1;
    }
    return 0;
}

/* Function to generate UNGT condition code (nle) */
int test_ungt(double a, double b) {
    /* Not less than or equal - should generate "nle" */
    if (!(a <= b)) {
        return 1;
    }
    return 0;
}

/* Function to generate UNLE condition code (ule) */
int test_unle(double a, double b) {
    /* Unordered or less than or equal - should generate "ule" */
    if (__builtin_isunordered(a, b) || a <= b) {
        return 1;
    }
    return 0;
}

/* Function to generate UNLT condition code (ult) */
int test_unlt(double a, double b) {
    /* Unordered or less than - should generate "ult" */
    if (__builtin_isunordered(a, b) || a < b) {
        return 1;
    }
    return 0;
}

/* Function to generate LTGT condition code (une) */
int test_ltgt(double a, double b) {
    /* Less than or greater than (ordered) - should generate "une" */
    if (__builtin_islessgreater(a, b)) {
        return 1;
    }
    return 0;
}

/* Mixed precision tests */
int test_mixed_precision(float a, double b) {
    int result = 0;
    
    /* Mixed precision unordered check */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* Mixed precision not less than */
    if (!(a < b)) {
        result |= 2;
    }
    
    return result;
}

/* Function calls in comparisons */
double get_nan() {
    return __builtin_nan("");
}

double get_inf() {
    return __builtin_inf();
}

float get_nanf() {
    return __builtin_nanf("");
}

/* Test with function return values */
int test_function_calls() {
    int result = 0;
    
    /* Compare function results */
    if (__builtin_isunordered(get_nan(), get_inf())) {
        result |= 1;
    }
    
    if (!(get_nan() < get_inf())) {
        result |= 2;
    }
    
    if (__builtin_islessgreater(vd1, vd2)) {
        result |= 4;
    }
    
    return result;
}

/* Complex control flow to force conditional jumps */
int test_complex_control_flow(double a, double b, double c) {
    int count = 0;
    
    /* Nested if-else with floating point conditions */
    if (__builtin_isunordered(a, b)) {
        count++;
    } else if (!(a < b)) {
        count += 2;
    } else if (__builtin_islessgreater(b, c)) {
        count += 3;
    } else if (__builtin_isordered(a, c)) {
        count += 4;
    }
    
    /* Ternary operator with FP condition */
    count += (__builtin_isunordered(b, c) ? 5 : 6);
    
    /* While loop with FP condition */
    double temp = a;
    while (!__builtin_isunordered(temp, b) && temp < 10.0) {
        temp += 1.0;
        count++;
    }
    
    return count;
}

/* Array indexing based on FP comparisons */
int test_array_indexing(double a, double b) {
    static const int lookup[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int index = 0;
    
    /* Build index from multiple FP comparisons */
    if (__builtin_isunordered(a, b)) index |= 1;
    if (!(a < b)) index |= 2;
    if (__builtin_islessgreater(a, b)) index |= 4;
    
    return lookup[index & 7];
}

/* Main driver function */
int main() {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Test with NaN values */
    checksum += test_unordered(vd_nan, vd1);
    checksum += test_unordered(vd1, vd_nan);
    checksum += test_unordered(vd_nan, vd_nan);
    
    /* Test with normal values */
    checksum += test_ordered(vd1, vd2);
    checksum += test_ordered(vd2, vd1);
    
    /* Test UNEQ */
    checksum += test_uneq(vd1, vd1);      /* Equal */
    checksum += test_uneq(vd_nan, vd1);   /* Unordered */
    
    /* Test UNGE (nlt) */
    checksum += test_unge(vd2, vd1);      /* Greater than */
    checksum += test_unge(vd1, vd1);      /* Equal */
    checksum += test_unge(vd_nan, vd1);   /* Unordered */
    
    /* Test UNGT (nle) */
    checksum += test_ungt(vd2, vd1);      /* Greater than */
    checksum += test_ungt(vd_nan, vd1);   /* Unordered */
    
    /* Test UNLE (ule) */
    checksum += test_unle(vd1, vd2);      /* Less than */
    checksum += test_unle(vd1, vd1);      /* Equal */
    checksum += test_unle(vd_nan, vd1);   /* Unordered */
    
    /* Test UNLT (ult) */
    checksum += test_unlt(vd1, vd2);      /* Less than */
    checksum += test_unlt(vd_nan, vd1);   /* Unordered */
    
    /* Test LTGT (une) */
    checksum += test_ltgt(vd1, vd2);      /* Less than */
    checksum += test_ltgt(vd2, vd1);      /* Greater than */
    
    /* Test mixed precision */
    checksum += test_mixed_precision(vf1, vd2);
    checksum += test_mixed_precision(vf_nan, vd1);
    
    /* Test with function calls */
    checksum += test_function_calls();
    
    /* Test complex control flow */
    checksum += test_complex_control_flow(vd1, vd2, vd_nan);
    checksum += test_complex_control_flow(vd_nan, vd_inf, vd1);
    
    /* Test array indexing */
    checksum += test_array_indexing(vd1, vd2);
    checksum += test_array_indexing(vd_nan, vd1);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Additional volatile operations to ensure they're not optimized away */
    volatile int final_check = checksum;
    
    return final_check != 0 ? 0 : 1;
}
