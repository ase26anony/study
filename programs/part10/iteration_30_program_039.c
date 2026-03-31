/* test_conditions.c - Program to trigger x86 floating-point condition code generation */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = __builtin_nanf("");
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = __builtin_nan("");

/* Function to generate UNORDERED condition code */
int test_unordered(float a, float b) {
    /* Direct unordered check - should generate "unord" */
    if (__builtin_isunordered(a, b)) {
        return 1;
    }
    return 0;
}

/* Function to generate ORDERED condition code */
int test_ordered(double a, double b) {
    /* Ordered check - should generate "ord" */
    if (!__builtin_isunordered(a, b)) {
        return 1;
    }
    return 0;
}

/* Function to generate UNEQ condition code */
int test_uneq(float a, float b) {
    /* Unordered or equal - should generate "ueq" */
    if (__builtin_isunordered(a, b) || a == b) {
        return 1;
    }
    return 0;
}

/* Function to generate UNGE condition code (nlt) */
int test_unge(double a, double b) {
    /* Not less than (unordered or greater or equal) - should generate "nlt" */
    if (!(a < b)) {
        return 1;
    }
    return 0;
}

/* Function to generate UNGT condition code (nle) */
int test_ungt(float a, float b) {
    /* Not less or equal (unordered or greater) - should generate "nle" */
    if (!(a <= b)) {
        return 1;
    }
    return 0;
}

/* Function to generate UNLE condition code */
int test_unle(double a, double b) {
    /* Unordered or less or equal - should generate "ule" */
    if (__builtin_isunordered(a, b) || a <= b) {
        return 1;
    }
    return 0;
}

/* Function to generate UNLT condition code */
int test_unlt(float a, float b) {
    /* Unordered or less than - should generate "ult" */
    if (__builtin_isunordered(a, b) || a < b) {
        return 1;
    }
    return 0;
}

/* Function to generate LTGT condition code (une) */
int test_ltgt(double a, double b) {
    /* Less or greater (unordered excluded) - should generate "une" */
    if (__builtin_islessgreater(a, b)) {
        return 1;
    }
    return 0;
}

/* Mixed precision tests */
int test_mixed_precision(float f, double d) {
    int result = 0;
    
    /* Mixed precision unordered check */
    if (__builtin_isunordered(f, d)) {
        result |= 1;
    }
    
    /* Mixed precision ordered comparison */
    if (!__builtin_isunordered(f, d) && f != d) {
        result |= 2;
    }
    
    return result;
}

/* Test with function returns that might produce NaN */
double maybe_nan(int flag) {
    if (flag) {
        return sqrt(-1.0);  /* Returns NaN */
    }
    return 3.14159;
}

float maybe_nanf(int flag) {
    if (flag) {
        return sqrtf(-1.0f);  /* Returns NaN */
    }
    return 2.71828f;
}

/* Test using ternary operator with comparisons */
int test_ternary(float a, float b) {
    /* Using ternary operator with floating comparison */
    return (__builtin_isunordered(a, b) ? 1 : 
           (a < b ? 2 : 
           (a > b ? 3 : 4)));
}

/* Test with array indexing based on comparisons */
int test_array_index(double a, double b) {
    static const int results[4] = {10, 20, 30, 40};
    
    if (__builtin_isunordered(a, b)) {
        return results[0];
    } else if (a < b) {
        return results[1];
    } else if (a > b) {
        return results[2];
    } else {
        return results[3];
    }
}

/* Main test driver */
int main() {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Test 1: Basic unordered comparisons with NaN */
    checksum += test_unordered(vf1, vf_nan);
    checksum += test_unordered(vf_nan, vf2);
    checksum += test_unordered(vf_nan, vf_nan);
    
    /* Test 2: Ordered comparisons */
    checksum += test_ordered(vd1, vd2);
    checksum += test_ordered(vd_nan, vd1);
    checksum += test_ordered(vd1, vd_nan);
    
    /* Test 3: UNEQ (unordered or equal) */
    checksum += test_uneq(0.0f, -0.0f);  /* Should be equal despite sign */
    checksum += test_uneq(vf1, vf_nan);  /* Unordered case */
    checksum += test_uneq(vf1, vf1);     /* Equal case */
    
    /* Test 4: UNGE (not less than) */
    checksum += test_unge(5.0, 3.0);
    checksum += test_unge(vd_nan, vd1);  /* Unordered case */
    checksum += test_unge(3.0, 3.0);     /* Equal case */
    
    /* Test 5: UNGT (not less or equal) */
    checksum += test_ungt(7.0f, 5.0f);
    checksum += test_ungt(vf_nan, vf2);  /* Unordered case */
    
    /* Test 6: UNLE (unordered or less or equal) */
    checksum += test_unle(2.0, 5.0);
    checksum += test_unle(vd_nan, vd2);  /* Unordered case */
    checksum += test_unle(4.0, 4.0);     /* Equal case */
    
    /* Test 7: UNLT (unordered or less than) */
    checksum += test_unlt(3.0f, 8.0f);
    checksum += test_unlt(vf_nan, vf1);  /* Unordered case */
    
    /* Test 8: LTGT (less or greater, not equal, not unordered) */
    checksum += test_ltgt(2.0, 3.0);
    checksum += test_ltgt(5.0, 1.0);
    
    /* Test 9: Mixed precision */
    checksum += test_mixed_precision(vf1, vd2);
    checksum += test_mixed_precision(vf_nan, vd1);
    
    /* Test 10: Function returns that might be NaN */
    checksum += test_unordered(maybe_nanf(1), 1.0f);
    checksum += test_ordered(maybe_nan(0), 2.0);
    
    /* Test 11: Ternary operator */
    checksum += test_ternary(vf1, vf2);
    checksum += test_ternary(vf_nan, vf1);
    
    /* Test 12: Array indexing */
    checksum += test_array_index(vd1, vd2);
    checksum += test_array_index(vd_nan, vd1);
    
    /* Test with constants */
    checksum += test_unordered(1.0f, __builtin_nanf(""));
    checksum += test_ordered(__builtin_nan(""), 2.0);
    
    /* Test with INFINITY */
    checksum += test_unge(INFINITY, 1000.0);
    checksum += test_unlt(-INFINITY, 0.0f);
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum is non-zero, all tests contributed to the result.\n");
    
    return 0;
}
