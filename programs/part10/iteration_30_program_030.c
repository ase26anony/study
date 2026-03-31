/* test_conditions.c - Program to trigger x86 floating-point condition code generation */
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
double get_nan() {
    return __builtin_nan("");
}

float get_nanf() {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code */
int test_unordered(double a, double b) {
    int result = 0;
    /* Direct unordered check */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    /* Alternative unordered check */
    if (a != a || b != b) {
        result |= 2;
    }
    return result;
}

/* Test ORDERED condition code */
int test_ordered(double a, double b) {
    int result = 0;
    /* Ordered check */
    if (!__builtin_isunordered(a, b)) {
        result |= 1;
    }
    /* Ordered comparison */
    if (a == a && b == b) {
        result |= 2;
    }
    return result;
}

/* Test UNEQ (unordered or equal) */
int test_uneq(double a, double b) {
    int result = 0;
    /* This should generate ueq */
    if (!(a < b) && !(a > b)) {
        result |= 1;
    }
    /* Alternative using builtin */
    if (__builtin_isunordered(a, b) || a == b) {
        result |= 2;
    }
    return result;
}

/* Test UNGE (not less than) - should generate nlt */
int test_unge(double a, double b) {
    int result = 0;
    /* Inverse of less than */
    if (!(a < b)) {
        result |= 1;
    }
    /* Direct greater or equal with NaN handling */
    if (a >= b || __builtin_isunordered(a, b)) {
        result |= 2;
    }
    return result;
}

/* Test UNGT (not less or equal) - should generate nle */
int test_ungt(double a, double b) {
    int result = 0;
    /* Inverse of less or equal */
    if (!(a <= b)) {
        result |= 1;
    }
    /* Direct greater than with NaN handling */
    if (a > b || __builtin_isunordered(a, b)) {
        result |= 2;
    }
    return result;
}

/* Test UNLE (unordered or less or equal) - should generate ule */
int test_unle(double a, double b) {
    int result = 0;
    /* This should generate ule */
    if (a <= b || __builtin_isunordered(a, b)) {
        result |= 1;
    }
    return result;
}

/* Test UNLT (unordered or less than) - should generate ult */
int test_unlt(double a, double b) {
    int result = 0;
    /* This should generate ult */
    if (a < b || __builtin_isunordered(a, b)) {
        result |= 1;
    }
    return result;
}

/* Test LTGT (less or greater, ordered) - should generate une */
int test_ltgt(double a, double b) {
    int result = 0;
    /* Direct less or greater check */
    if (__builtin_islessgreater(a, b)) {
        result |= 1;
    }
    /* Alternative: ordered and not equal */
    if (!__builtin_isunordered(a, b) && a != b) {
        result |= 2;
    }
    /* Manual check */
    if ((a < b) || (a > b)) {
        result |= 4;
    }
    return result;
}

/* Mixed precision tests */
int test_mixed_precision(float f, double d) {
    int result = 0;
    
    /* Mixed unordered */
    if (__builtin_isunordered(f, d)) {
        result |= 1;
    }
    
    /* Mixed ordered comparison */
    if (!__builtin_isunordered(f, d) && f != d) {
        result |= 2;
    }
    
    /* Inverse mixed comparison */
    if (!(f > d)) {
        result |= 4;
    }
    
    return result;
}

/* Test with function returns */
int test_function_calls() {
    int result = 0;
    double d1 = sqrt(-1.0);  /* Returns NaN */
    double d2 = 1.0 / 0.0;   /* Infinity */
    double d3 = 0.0 / 0.0;   /* NaN */
    
    /* Various comparisons with function results */
    if (__builtin_isunordered(d1, d2)) result |= 1;
    if (!__builtin_isunordered(d2, 1.0)) result |= 2;
    if (d1 <= d3 || __builtin_isunordered(d1, d3)) result |= 4;
    if (!(d2 < 0.0)) result |= 8;
    
    return result;
}

/* Test with constants */
int test_constants() {
    int result = 0;
    volatile double x = vd1;
    
    /* Compare with NaN constant */
    if (__builtin_isunordered(x, NAN)) result |= 1;
    
    /* Compare with infinity */
    if (!(x > INFINITY)) result |= 2;
    
    /* Compare with zero */
    if (x <= 0.0 || __builtin_isunordered(x, 0.0)) result |= 4;
    
    /* Inverse comparison with constant */
    if (!(x < 1.5)) result |= 8;
    
    return result;
}

/* Main test driver */
int main() {
    int checksum = 0;
    
    printf("Starting floating-point condition code tests...\n");
    
    /* Test with normal values */
    checksum ^= test_unordered(1.0, 2.0);
    checksum ^= test_ordered(1.0, 2.0);
    checksum ^= test_uneq(1.0, 2.0);
    checksum ^= test_unge(1.0, 2.0);
    checksum ^= test_ungt(1.0, 2.0);
    checksum ^= test_unle(1.0, 2.0);
    checksum ^= test_unlt(1.0, 2.0);
    checksum ^= test_ltgt(1.0, 2.0);
    
    /* Test with NaN */
    checksum ^= test_unordered(1.0, get_nan());
    checksum ^= test_ordered(get_nan(), 2.0);
    checksum ^= test_uneq(get_nan(), get_nan());
    checksum ^= test_unge(get_nan(), 1.0);
    checksum ^= test_ungt(1.0, get_nan());
    checksum ^= test_unle(get_nan(), 2.0);
    checksum ^= test_unlt(2.0, get_nan());
    checksum ^= test_ltgt(get_nan(), 1.0);
    
    /* Test with volatile variables */
    checksum ^= test_unordered(vd1, vd_nan);
    checksum ^= test_ordered(vd_nan, vd2);
    checksum ^= test_uneq(vd1, vd2);
    checksum ^= test_unge(vd_nan, vd1);
    
    /* Mixed precision tests */
    checksum ^= test_mixed_precision(vf1, vd2);
    checksum ^= test_mixed_precision(vf_nan, vd1);
    checksum ^= test_mixed_precision(1.5f, get_nan());
    
    /* Function call tests */
    checksum ^= test_function_calls();
    
    /* Constant tests */
    checksum ^= test_constants();
    
    /* Additional complex expressions */
    {
        double a = vd1;
        double b = vd2;
        double c = vd_nan;
        
        /* Complex conditional expression */
        int res1 = (__builtin_isunordered(a, b) ? 1 : 0);
        int res2 = (!(a >= c) ? 2 : 0);
        int res3 = ((b <= a || __builtin_isunordered(b, a)) ? 4 : 0);
        int res4 = (__builtin_islessgreater(a, b) ? 8 : 0);
        
        checksum ^= (res1 | res2 | res3 | res4);
        
        /* Use in loop condition */
        int count = 0;
        while (!__builtin_isunordered(a, b) && count < 3) {
            count++;
            a += 0.5;
        }
        checksum ^= count;
        
        /* Use in array indexing (simulated) */
        int array[4] = {0, 0, 0, 0};
        int idx = (__builtin_isunordered(c, 0.0) ? 0 : 1) +
                 (!(b < a) ? 0 : 2);
        if (idx < 4) {
            array[idx] = 1;
            checksum ^= array[idx];
        }
    }
    
    printf("Test checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
