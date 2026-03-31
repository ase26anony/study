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

/* Function to generate NAN through computation */
double make_nan() {
    return sqrt(-1.0);
}

float make_nanf() {
    return sqrtf(-1.0f);
}

/* Test UNORDERED condition code */
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
    /* Mixed types */
    float fa = (float)a;
    float fb = (float)b;
    if (__builtin_isunordered(fa, fb)) {
        result |= 4;
    }
    return result;
}

/* Test ORDERED condition code */
int test_ordered(double a, double b) {
    int result = 0;
    /* Direct ordered check */
    if (__builtin_isordered(a, b)) {
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
    /* This should generate ueq */
    if (!(a < b) && !(a > b)) {  /* Not less and not greater */
        result |= 1;
    }
    /* Alternative using builtin */
    if (!__builtin_islessgreater(a, b)) {
        result |= 2;
    }
    return result;
}

/* Test UNGE (not less than) condition code */
int test_unge(double a, double b) {
    int result = 0;
    /* Inverse of < should generate nlt */
    if (!(a < b)) {
        result |= 1;
    }
    /* Using >= with NaN possibility */
    if (a >= b) {
        result |= 2;
    }
    return result;
}

/* Test UNGT (not less than or equal) condition code */
int test_ungt(double a, double b) {
    int result = 0;
    /* Inverse of <= should generate nle */
    if (!(a <= b)) {
        result |= 1;
    }
    /* Using > with NaN possibility */
    if (a > b) {
        result |= 2;
    }
    return result;
}

/* Test UNLE (unordered or less than or equal) condition code */
int test_unle(double a, double b) {
    int result = 0;
    /* This pattern should generate ule */
    if (a <= b) {
        result |= 1;
    }
    /* With volatile variables */
    volatile double va = a;
    volatile double vb = b;
    if (va <= vb) {
        result |= 2;
    }
    return result;
}

/* Test UNLT (unordered or less than) condition code */
int test_unlt(double a, double b) {
    int result = 0;
    /* This pattern should generate ult */
    if (a < b) {
        result |= 1;
    }
    /* Mixed precision */
    float fa = (float)a;
    float fb = (float)b;
    if (fa < fb) {
        result |= 4;
    }
    return result;
}

/* Test LTGT (less than or greater than) condition code */
int test_ltgt(double a, double b) {
    int result = 0;
    /* Direct lessgreater check - should generate une */
    if (__builtin_islessgreater(a, b)) {
        result |= 1;
    }
    /* Alternative: (a < b) || (a > b) */
    if (a < b || a > b) {
        result |= 2;
    }
    return result;
}

/* Test with function return values */
int test_function_calls() {
    int result = 0;
    double nan1 = make_nan();
    double nan2 = make_nan();
    float nanf1 = make_nanf();
    
    /* Compare function results */
    if (__builtin_isunordered(nan1, 1.0)) {
        result |= 1;
    }
    if (!__builtin_islessgreater(nanf1, 2.0f)) {
        result |= 2;
    }
    if (nan2 <= 3.0) {
        result |= 4;
    }
    
    return result;
}

/* Test with constants including INFINITY */
int test_constants() {
    int result = 0;
    const double inf = INFINITY;
    const double neg_inf = -INFINITY;
    const double zero = 0.0;
    const double neg_zero = -0.0;
    
    /* Compare with infinity */
    if (__builtin_isunordered(vd1, inf)) {
        result |= 1;
    }
    if (!(vd2 < inf)) {  /* Should generate nlt */
        result |= 2;
    }
    if (neg_inf <= vd1) {  /* Should generate ule */
        result |= 4;
    }
    
    /* Compare with zero */
    if (zero >= neg_zero) {  /* -0.0 == 0.0 */
        result |= 8;
    }
    
    return result;
}

/* Main driver function */
int main() {
    int checksum = 0;
    
    printf("Testing floating-point condition codes...\n");
    
    /* Test with normal values */
    checksum ^= test_unordered(1.0, 2.0);
    checksum ^= test_ordered(1.0, 2.0);
    checksum ^= test_uneq(1.0, 1.0);
    checksum ^= test_unge(3.0, 2.0);
    checksum ^= test_ungt(3.0, 2.0);
    checksum ^= test_unle(1.0, 2.0);
    checksum ^= test_unlt(1.0, 2.0);
    checksum ^= test_ltgt(1.0, 2.0);
    
    /* Test with NaN values */
    checksum ^= test_unordered(vd_nan, 1.0);
    checksum ^= test_ordered(vd_nan, 1.0);
    checksum ^= test_uneq(vd_nan, vd_nan);
    checksum ^= test_unge(vd_nan, 2.0);
    checksum ^= test_ungt(1.0, vd_nan);
    checksum ^= test_unle(vd_nan, 3.0);
    checksum ^= test_unlt(1.0, vd_nan);
    checksum ^= test_ltgt(vd_nan, 2.0);
    
    /* Test with volatile variables */
    checksum ^= test_unordered(vd1, vd2);
    checksum ^= test_ordered(vf1, vf2);
    checksum ^= test_uneq(vd1, vd1);
    checksum ^= test_unge(vd2, vd1);
    checksum ^= test_ungt(vd2, vd1);
    checksum ^= test_unle(vd1, vd2);
    checksum ^= test_unlt(vf1, vf2);
    checksum ^= test_ltgt(vd2, vd1);
    
    /* Additional tests */
    checksum ^= test_function_calls();
    checksum ^= test_constants();
    
    /* Force use of results in control flow */
    if (checksum & 1) {
        printf("Checksum has LSB set\n");
    }
    
    /* Array indexing based on comparison results */
    int array[8] = {0};
    int idx = 0;
    idx += test_unordered(1.0, 2.0) ? 1 : 0;
    idx += test_ltgt(2.0, 1.0) ? 2 : 0;
    array[idx % 8] = checksum;
    
    printf("Final checksum: %d\n", checksum);
    printf("Array[%d] = %d\n", idx % 8, array[idx % 8]);
    
    return 0;
}
