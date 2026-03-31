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
double get_nan() {
    return __builtin_nan("");
}

/* Function to generate infinity */
double get_inf() {
    return __builtin_inf();
}

/* Test UNORDERED condition code (unord) */
int test_unordered() {
    double d1 = vd1;
    double d2 = vd_nan;
    float f1 = vf1;
    float f2 = vf_nan;
    
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d2)) result |= 1;
    if (__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Using != self to detect NaN */
    if (d2 != d2) result |= 4;  /* NaN check */
    
    /* Mixed types */
    if (__builtin_isunordered(d1, (double)f2)) result |= 8;
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered() {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = vd_nan;
    
    int result = 0;
    
    /* Ordered checks */
    if (!__builtin_isunordered(d1, d2)) result |= 1;
    if (d1 == d1 && d2 == d2) result |= 2;  /* Both are numbers */
    
    /* Compare ordered value with NaN */
    if (!__builtin_isunordered(d1, d_nan)) result |= 4;
    else result |= 8;
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq() {
    double a = vd1;
    double b = vd1;  /* Equal values */
    double c = vd_nan;
    
    int result = 0;
    
    /* UNEQ: unordered or equal */
    if (__builtin_isunordered(a, c) || a == b) result |= 1;
    
    /* Using builtin for unordered-or-equal logic */
    if (!(a < b) && !(a > b)) result |= 2;  /* Includes NaN case */
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge() {
    double a = vd2;
    double b = vd1;
    double nan = vd_nan;
    
    int result = 0;
    
    /* UNGE: unordered or not less than (nlt) */
    if (__builtin_isunordered(a, nan) || !(a < b)) result |= 1;
    
    /* Inverse of less than */
    if (!(b < a)) result |= 2;  /* b >= a or unordered */
    
    /* With NaN operand */
    if (!(nan < a)) result |= 4;
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt() {
    double a = vd2;
    double b = vd1;
    double nan = vd_nan;
    
    int result = 0;
    
    /* UNGT: unordered or not less or equal (nle) */
    if (__builtin_isunordered(a, nan) || !(a <= b)) result |= 1;
    
    /* Inverse of less or equal */
    if (!(b <= a)) result |= 2;
    
    /* Direct with NaN */
    if (!(nan <= a)) result |= 4;
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle() {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    int result = 0;
    
    /* UNLE: unordered or less or equal */
    if (__builtin_isunordered(a, nan) || a <= b) result |= 1;
    
    /* Standard comparison that should generate ule */
    if (a <= b) result |= 2;
    
    /* With NaN */
    if (nan <= b) result |= 4;
    else result |= 8;
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt() {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    int result = 0;
    
    /* UNLT: unordered or less than */
    if (__builtin_isunordered(a, nan) || a < b) result |= 1;
    
    /* Standard less than */
    if (a < b) result |= 2;
    
    /* NaN comparison */
    if (nan < b) result |= 4;
    else result |= 8;
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt() {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    int result = 0;
    
    /* LTGT: less or greater (ordered and not equal) */
    if (__builtin_islessgreater(a, b)) result |= 1;
    
    /* Equivalent: ordered and (less than or greater than) */
    if (!__builtin_isunordered(a, b) && (a < b || a > b)) result |= 2;
    
    /* With NaN - should be false */
    if (__builtin_islessgreater(a, nan)) result |= 4;
    
    return result;
}

/* Test mixed precision comparisons */
int test_mixed_precision() {
    float f = vf1;
    double d = vd2;
    float f_nan = vf_nan;
    double d_nan = vd_nan;
    
    int result = 0;
    
    /* Mixed float/double comparisons */
    if (__builtin_isunordered(f, d_nan)) result |= 1;
    if (!__builtin_isunordered((double)f, d)) result |= 2;
    if (f < d) result |= 4;
    if (!(d <= (double)f_nan)) result |= 8;
    
    return result;
}

/* Test with function return values */
int test_function_calls() {
    int result = 0;
    
    /* Comparisons with function results */
    double nan = get_nan();
    double inf = get_inf();
    
    if (__builtin_isunordered(nan, inf)) result |= 1;
    if (!__builtin_isunordered(vd1, sqrt(vd2))) result |= 2;
    if (sin(vd1) < cos(vd2)) result |= 4;
    if (!(log(vd1) <= nan)) result |= 8;
    
    return result;
}

/* Test complex expressions with condition codes */
int test_complex_expressions() {
    volatile double x = 0.0;
    volatile double y = -0.0;
    volatile double z = vd_nan;
    
    int result = 0;
    int i;
    
    /* Loop with floating-point condition */
    for (i = 0; i < 10; i++) {
        x += 0.1;
        /* Use different condition codes in loop control */
        if (__builtin_isunordered(x, z)) {
            result += i;
        }
        if (!(y < x)) {  /* nlt */
            result += i * 2;
        }
    }
    
    /* Array indexing based on FP comparison */
    int arr[4] = {0};
    arr[(__builtin_islessgreater(x, y) ? 1 : 0)] = 1;
    arr[(!(x <= z) ? 2 : 0)] = 2;  /* nle */
    
    result += arr[0] + arr[1] + arr[2];
    
    return result;
}

/* Main driver function */
int main() {
    unsigned int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Call all test functions */
    checksum = (checksum * 31) + test_unordered();
    printf("test_unordered: %d\n", test_unordered());
    
    checksum = (checksum * 31) + test_ordered();
    printf("test_ordered: %d\n", test_ordered());
    
    checksum = (checksum * 31) + test_uneq();
    printf("test_uneq: %d\n", test_uneq());
    
    checksum = (checksum * 31) + test_unge();
    printf("test_unge: %d\n", test_unge());
    
    checksum = (checksum * 31) + test_ungt();
    printf("test_ungt: %d\n", test_ungt());
    
    checksum = (checksum * 31) + test_unle();
    printf("test_unle: %d\n", test_unle());
    
    checksum = (checksum * 31) + test_unlt();
    printf("test_unlt: %d\n", test_unlt());
    
    checksum = (checksum * 31) + test_ltgt();
    printf("test_ltgt: %d\n", test_ltgt());
    
    checksum = (checksum * 31) + test_mixed_precision();
    printf("test_mixed_precision: %d\n", test_mixed_precision());
    
    checksum = (checksum * 31) + test_function_calls();
    printf("test_function_calls: %d\n", test_function_calls());
    
    checksum = (checksum * 31) + test_complex_expressions();
    printf("test_complex_expressions: %d\n", test_complex_expressions());
    
    printf("Final checksum: %u\n", checksum);
    
    /* Return checksum to ensure all code is live */
    return checksum & 0xFF;
}
