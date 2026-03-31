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
int test_unordered() {
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(vd_nan, vd1)) result |= 1;
    if (__builtin_isunordered(vd1, vd_nan)) result |= 2;
    
    /* Using NaN constant */
    double local_nan = make_nan();
    if (__builtin_isunordered(local_nan, 3.14)) result |= 4;
    
    /* Mixed float/double */
    if (__builtin_isunordered(vf_nan, (float)vd1)) result |= 8;
    
    /* Compare two NaNs */
    if (__builtin_isunordered(vd_nan, vd_nan)) result |= 16;
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered() {
    int result = 0;
    
    /* Ordered checks */
    if (!__builtin_isunordered(vd1, vd2)) result |= 1;
    if (!__builtin_isunordered(vd2, vd1)) result |= 2;
    
    /* Using function returns */
    double x = sin(1.0);
    double y = cos(1.0);
    if (!__builtin_isunordered(x, y)) result |= 4;
    
    /* Float ordered check */
    if (!__builtin_isunordered(vf1, vf2)) result |= 8;
    
    /* Ordered with infinity */
    if (!__builtin_isunordered(vd_inf, vd1)) result |= 16;
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq() {
    int result = 0;
    
    /* Unordered or equal */
    double nan1 = make_nan();
    double a = 5.0;
    
    /* This should generate UNEQ: (a == b) || unordered(a, b) */
    if (!(a < nan1) && !(a > nan1)) result |= 1;  /* a UNEQ nan */
    
    /* Equal values */
    if (!(vd1 < vd1) && !(vd1 > vd1)) result |= 2;  /* vd1 UNEQ vd1 */
    
    /* Using builtin */
    if (!__builtin_islessgreater(3.14, 3.14)) result |= 4;
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge() {
    int result = 0;
    
    /* Unordered or greater-or-equal: !(a < b) */
    double nan_val = make_nan();
    
    if (!(vd1 < nan_val)) result |= 1;      /* vd1 UNGE nan */
    if (!(2.0 < 1.0)) result |= 2;          /* 2.0 UNGE 1.0 */
    if (!(vd1 < vd2)) result |= 4;          /* vd1 UNGE vd2 */
    
    /* Float version */
    if (!(vf1 < vf_nan)) result |= 8;       /* vf1 UNGE nan */
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt() {
    int result = 0;
    
    /* Unordered or greater: !(a <= b) */
    if (!(vd1 <= vd_nan)) result |= 1;      /* vd1 UNGT nan */
    if (!(1.0 <= 0.5)) result |= 2;         /* 1.0 UNGT 0.5 */
    
    /* Using function result */
    double x = exp(1.0);
    if (!(x <= 2.0)) result |= 4;           /* e UNGT 2.0 */
    
    /* Mixed precision */
    if (!((double)vf1 <= vd_nan)) result |= 8;
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle() {
    int result = 0;
    
    /* Unordered or less-or-equal */
    if (!(vd_nan > vd1)) result |= 1;       /* nan UNLE vd1 */
    if (!(3.0 > 5.0)) result |= 2;          /* 3.0 UNLE 5.0 */
    
    /* Using volatile */
    if (!(vd2 > vd1)) result |= 4;          /* vd2 UNLE vd1 (false) */
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt() {
    int result = 0;
    
    /* Unordered or less-than */
    if (!(vd_nan >= vd1)) result |= 1;      /* nan UNLT vd1 */
    if (!(1.0 >= 2.0)) result |= 2;         /* 1.0 UNLT 2.0 */
    
    /* Float version */
    if (!(vf_nan >= vf1)) result |= 4;      /* nan UNLT vf1 */
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt() {
    int result = 0;
    
    /* Less or greater (ordered comparison, not equal) */
    if (__builtin_islessgreater(vd1, vd2)) result |= 1;   /* vd1 LTGT vd2 */
    if (__builtin_islessgreater(vd2, vd1)) result |= 2;   /* vd2 LTGT vd1 */
    
    /* Using explicit comparison */
    double a = 10.0, b = 20.0;
    if ((a < b) || (a > b)) result |= 4;    /* a LTGT b */
    
    /* With function calls */
    if (__builtin_islessgreater(sin(0.5), cos(0.5))) result |= 8;
    
    return result;
}

/* Test complex patterns that might generate specific condition codes */
int test_complex_patterns() {
    int result = 0;
    volatile double x = 1.5;
    volatile double y = 2.5;
    volatile double z = make_nan();
    
    /* Complex expression that might use UNORDERED */
    result += __builtin_isunordered(x, z) ? 1 : 0;
    
    /* Ternary operator with unordered check */
    result += (__builtin_isunordered(y, z) ? 2 : 0);
    
    /* While loop with ordered check */
    int count = 0;
    while (!__builtin_isunordered(x, y) && count < 3) {
        result += 4;
        count++;
    }
    
    /* Array indexing based on comparison */
    int arr[4] = {0, 1, 2, 3};
    result += arr[__builtin_islessgreater(x, y) ? 1 : 0];
    
    return result;
}

/* Main driver function */
int main() {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Call all test functions */
    checksum ^= test_unordered();
    printf("test_unordered: %d\n", test_unordered());
    
    checksum ^= test_ordered();
    printf("test_ordered: %d\n", test_ordered());
    
    checksum ^= test_uneq();
    printf("test_uneq: %d\n", test_uneq());
    
    checksum ^= test_unge();
    printf("test_unge: %d\n", test_unge());
    
    checksum ^= test_ungt();
    printf("test_ungt: %d\n", test_ungt());
    
    checksum ^= test_unle();
    printf("test_unle: %d\n", test_unle());
    
    checksum ^= test_unlt();
    printf("test_unlt: %d\n", test_unlt());
    
    checksum ^= test_ltgt();
    printf("test_ltgt: %d\n", test_ltgt());
    
    checksum ^= test_complex_patterns();
    printf("test_complex_patterns: %d\n", test_complex_patterns());
    
    printf("Final checksum: %d\n", checksum);
    
    /* Return checksum as exit code */
    return checksum & 0xFF;
}
