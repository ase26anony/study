/* test_conditions.c - Program to exercise x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = 0.0/0.0;  /* Will become NAN */
volatile float vf1 = 3.0f;
volatile float vf2 = 4.0f;
volatile float vf_nan = 0.0f/0.0f;

/* Function to generate NaN */
double get_nan(void) {
    return __builtin_nan("");
}

float get_nanf(void) {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code */
int test_unordered(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = get_nan();
    float f_nan = get_nanf();
    
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d_nan)) result |= 1;
    if (__builtin_isunordered(d_nan, d2)) result |= 2;
    if (__builtin_isunordered(d_nan, d_nan)) result |= 4;
    
    /* Using !(a == a) pattern */
    if (!(d_nan == d_nan)) result |= 8;
    
    /* Mixed float/double */
    if (__builtin_isunordered(vf1, f_nan)) result |= 16;
    
    return result;
}

/* Test ORDERED condition code */
int test_ordered(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = get_nan();
    
    int result = 0;
    
    /* Ordered checks */
    if (__builtin_isordered(d1, d2)) result |= 1;
    if (__builtin_isordered(d1, d1)) result |= 2;
    
    /* Using !__builtin_isunordered */
    if (!__builtin_isunordered(d1, d2)) result |= 4;
    
    /* With constants */
    if (__builtin_isordered(3.14, 2.71)) result |= 8;
    
    return result;
}

/* Test UNEQ (unordered or equal) */
int test_uneq(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = get_nan();
    
    int result = 0;
    
    /* Using ordered equal check */
    if (!__builtin_islessgreater(d1, d1)) result |= 1;  /* a == a */
    if (!__builtin_islessgreater(d1, d2)) result |= 2;  /* a != b, but not less/greater */
    
    /* With NaN - should be true (unordered) */
    if (!__builtin_islessgreater(d1, d_nan)) result |= 4;
    if (!__builtin_islessgreater(d_nan, d_nan)) result |= 8;
    
    return result;
}

/* Test UNGE (not less than) - generates "nlt" */
int test_unge(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = get_nan();
    
    int result = 0;
    
    /* Inverse conditions */
    if (!(d1 < d2)) result |= 1;    /* d1 >= d2 -> nlt */
    if (!(d2 < d1)) result |= 2;    /* d2 >= d1 -> nlt */
    
    /* With NaN - unordered, so !(NaN < x) is true */
    if (!(d_nan < d1)) result |= 4;
    if (!(d_nan < d_nan)) result |= 8;
    
    /* Using __builtin_isless */
    if (!__builtin_isless(d1, d2)) result |= 16;
    
    return result;
}

/* Test UNGT (not less than or equal) - generates "nle" */
int test_ungt(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = get_nan();
    
    int result = 0;
    
    /* Inverse conditions */
    if (!(d1 <= d2)) result |= 1;   /* d1 > d2 -> nle */
    if (!(d2 <= d1)) result |= 2;   /* d2 > d1 -> nle */
    
    /* With NaN */
    if (!(d_nan <= d1)) result |= 4;
    if (!(d_nan <= d_nan)) result |= 8;
    
    /* Using __builtin_islessequal */
    if (!__builtin_islessequal(d1, d2)) result |= 16;
    
    return result;
}

/* Test UNLE (unordered or less than or equal) - generates "ule" */
int test_unle(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = get_nan();
    
    int result = 0;
    
    /* Direct comparisons that might generate ule */
    if (d1 <= d2) result |= 1;
    if (d2 <= d1) result |= 2;
    
    /* With NaN - should be true (unordered) */
    if (d_nan <= d1) result |= 4;
    if (d_nan <= d_nan) result |= 8;
    
    /* Using __builtin_islessequal with NaN */
    if (__builtin_islessequal(d_nan, d1)) result |= 16;
    
    return result;
}

/* Test UNLT (unordered or less than) - generates "ult" */
int test_unlt(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = get_nan();
    
    int result = 0;
    
    /* Direct less-than comparisons */
    if (d1 < d2) result |= 1;
    if (d2 < d1) result |= 2;
    
    /* With NaN - should be true (unordered) */
    if (d_nan < d1) result |= 4;
    if (d_nan < d_nan) result |= 8;
    
    /* Using __builtin_isless with NaN */
    if (__builtin_isless(d_nan, d1)) result |= 16;
    
    return result;
}

/* Test LTGT (less than or greater than, ordered) - generates "une" */
int test_ltgt(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = get_nan();
    
    int result = 0;
    
    /* Direct __builtin_islessgreater */
    if (__builtin_islessgreater(d1, d2)) result |= 1;
    if (__builtin_islessgreater(d2, d1)) result |= 2;
    
    /* Equivalent: (a < b) || (a > b) */
    if ((d1 < d2) || (d1 > d2)) result |= 4;
    
    /* With NaN - should be false */
    if (__builtin_islessgreater(d1, d_nan)) result |= 8;
    if (__builtin_islessgreater(d_nan, d_nan)) result |= 16;
    
    return result;
}

/* Test mixed precision comparisons */
int test_mixed(void) {
    float f1 = vf1;
    float f2 = vf2;
    double d1 = vd1;
    double d_nan = get_nan();
    
    int result = 0;
    
    /* float vs double */
    if (__builtin_isunordered(f1, d_nan)) result |= 1;
    if (!__builtin_islessgreater(f1, (double)f2)) result |= 2;
    if (!(f1 < d1)) result |= 4;  /* nlt */
    if (!(d1 <= f2)) result |= 8; /* nle */
    
    return result;
}

/* Test with function returns */
int test_function_calls(void) {
    int result = 0;
    
    /* sqrt(-1) returns NaN */
    double nan_val = sqrt(-1.0);
    double inf_val = 1.0 / 0.0;  /* INFINITY */
    
    if (__builtin_isunordered(nan_val, 0.0)) result |= 1;
    if (!__builtin_islessgreater(inf_val, 1.0)) result |= 2;
    if (!(nan_val < inf_val)) result |= 4;  /* nlt */
    if (nan_val <= nan_val) result |= 8;    /* ule */
    
    return result;
}

/* Test using ternary operator */
int test_ternary(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = get_nan();
    
    int result = 0;
    
    /* Ternary with unordered check */
    result += __builtin_isunordered(d1, d_nan) ? 1 : 0;
    result += __builtin_isunordered(d_nan, d2) ? 2 : 0;
    
    /* Ternary with inverse conditions */
    result += !(d1 < d2) ? 4 : 0;   /* nlt */
    result += !(d1 <= d2) ? 8 : 0;  /* nle */
    
    /* Ternary with ordered comparisons */
    result += __builtin_islessgreater(d1, d2) ? 16 : 0;  /* une */
    
    return result;
}

/* Test in loop for multiple executions */
int test_loop(void) {
    double values[] = {1.0, 2.0, get_nan(), 3.0, 4.0};
    int result = 0;
    
    for (int i = 0; i < 4; i++) {
        if (__builtin_isunordered(values[i], values[i+1])) {
            result += (1 << i);
        }
        if (!__builtin_islessgreater(values[i], values[i+1])) {
            result += (1 << (i + 4));
        }
    }
    
    return result;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Call all test functions */
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    checksum ^= test_mixed();
    checksum ^= test_function_calls();
    checksum ^= test_ternary();
    checksum ^= test_loop();
    
    /* Print result to ensure code isn't optimized away */
    printf("Checksum: %d\n", checksum);
    
    /* Additional control flow based on comparisons */
    double d1 = vd1;
    double d_nan = get_nan();
    
    if (__builtin_isunordered(d1, d_nan)) {
        printf("Unordered comparison triggered\n");
    }
    
    if (!__builtin_islessgreater(d1, d_nan)) {
        printf("Ordered equal/NaN comparison triggered\n");
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
