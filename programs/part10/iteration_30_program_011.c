/* test_conditions.c - Program to exercise x86 floating-point condition codes */
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

/* Function prototypes for different condition code patterns */
int test_unordered(void);
int test_ordered(void);
int test_uneq(void);
int test_unge(void);
int test_ungt(void);
int test_unle(void);
int test_unlt(void);
int test_ltgt(void);
int test_mixed_precision(void);
int test_with_constants(void);
int test_function_results(void);

/* Helper to generate NaN */
double make_nan(void) {
    return __builtin_nan("");
}

/* Helper to generate infinity */
double make_inf(void) {
    return __builtin_inf();
}

/* Test UNORDERED condition code */
int test_unordered(void) {
    int result = 0;
    double a = vd1;
    double b = vd_nan;
    
    /* Direct unordered check - should generate "unord" */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* Alternative unordered check */
    float f1 = vf1;
    float f2 = vf_nan;
    if (__builtin_isunordered(f1, f2)) {
        result |= 2;
    }
    
    /* Using !(a == a) pattern for NaN check */
    double nan_val = make_nan();
    if (__builtin_isunordered(nan_val, 0.0)) {
        result |= 4;
    }
    
    return result;
}

/* Test ORDERED condition code */
int test_ordered(void) {
    int result = 0;
    double a = vd1;
    double b = vd2;
    
    /* Ordered check - should generate "ord" */
    if (!__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* Check with NaN */
    if (!__builtin_isunordered(a, vd_nan)) {
        /* This won't execute, but forces code generation */
        result |= 2;
    }
    
    /* Mixed types */
    float f1 = vf1;
    double d1 = vd1;
    if (!__builtin_isunordered(f1, d1)) {
        result |= 4;
    }
    
    return result;
}

/* Test UNEQ condition code */
int test_uneq(void) {
    int result = 0;
    double a = vd1;
    double b = vd1;  /* Equal values */
    
    /* Unordered or equal - should generate "ueq" */
    /* Using !(a < b) && !(a > b) which includes NaN case */
    if (!(a < b) && !(a > b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(vd_nan < b) && !(vd_nan > b)) {
        result |= 2;
    }
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge(void) {
    int result = 0;
    double a = vd2;
    double b = vd1;
    
    /* Unordered or greater than or equal - should generate "nlt" */
    /* Using !(a < b) which is UNGE */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* With NaN - NaN is unordered, so !(NaN < b) is true */
    if (!(vd_nan < b)) {
        result |= 2;
    }
    
    /* Float version */
    float f1 = vf2;
    float f2 = vf1;
    if (!(f1 < f2)) {
        result |= 4;
    }
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt(void) {
    int result = 0;
    double a = vd2;
    double b = vd1;
    
    /* Unordered or greater than - should generate "nle" */
    /* Using !(a <= b) which is UNGT */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(vd_nan <= b)) {
        result |= 2;
    }
    
    return result;
}

/* Test UNLE condition code */
int test_unle(void) {
    int result = 0;
    double a = vd1;
    double b = vd2;
    
    /* Unordered or less than or equal - should generate "ule" */
    /* Using (a <= b) || __builtin_isunordered(a, b) */
    if ((a <= b) || __builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* With actual NaN */
    if ((vd_nan <= b) || __builtin_isunordered(vd_nan, b)) {
        result |= 2;
    }
    
    return result;
}

/* Test UNLT condition code */
int test_unlt(void) {
    int result = 0;
    double a = vd1;
    double b = vd2;
    
    /* Unordered or less than - should generate "ult" */
    /* Using (a < b) || __builtin_isunordered(a, b) */
    if ((a < b) || __builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* With NaN */
    if ((vd_nan < b) || __builtin_isunordered(vd_nan, b)) {
        result |= 2;
    }
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt(void) {
    int result = 0;
    double a = vd1;
    double b = vd2;
    
    /* Less than or greater than (ordered and not equal) - should generate "une" */
    /* Using __builtin_islessgreater(a, b) */
    if (__builtin_islessgreater(a, b)) {
        result |= 1;
    }
    
    /* Alternative: (a < b) || (a > b) with ordered check */
    if ((a < b) || (a > b)) {
        result |= 2;
    }
    
    /* With function results */
    double sqrt_neg = sqrt(-1.0);  /* Returns NaN */
    if (__builtin_islessgreater(sqrt_neg, 0.0)) {
        result |= 4;  /* Won't execute but forces code gen */
    }
    
    return result;
}

/* Test mixed precision comparisons */
int test_mixed_precision(void) {
    int result = 0;
    
    /* float vs double */
    float f = vf1;
    double d = vd2;
    
    /* Various comparisons with mixed types */
    if (__builtin_isunordered(f, d)) {
        result |= 1;
    }
    
    if (!(f < d)) {  /* UNGE */
        result |= 2;
    }
    
    if ((f <= d) || __builtin_isunordered(f, d)) {  /* UNLE */
        result |= 4;
    }
    
    return result;
}

/* Test with constants */
int test_with_constants(void) {
    int result = 0;
    double d = vd1;
    
    /* Compare with various constants */
    if (__builtin_isunordered(d, NAN)) {
        result |= 1;
    }
    
    if (!(d < 0.0)) {  /* UNGE */
        result |= 2;
    }
    
    if (!(d <= -0.0)) {  /* UNGT */
        result |= 4;
    }
    
    if ((INFINITY <= d) || __builtin_isunordered(INFINITY, d)) {  /* UNLE */
        result |= 8;
    }
    
    return result;
}

/* Test with function return values */
int test_function_results(void) {
    int result = 0;
    
    /* Use function results in comparisons */
    double nan1 = make_nan();
    double inf1 = make_inf();
    
    if (__builtin_isunordered(nan1, inf1)) {
        result |= 1;
    }
    
    if (!__builtin_isunordered(sqrt(4.0), 2.0)) {  /* ORDERED */
        result |= 2;
    }
    
    if (__builtin_islessgreater(sin(0.0), cos(0.0))) {  /* LTGT */
        result |= 4;
    }
    
    return result;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Call all test functions */
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    checksum ^= test_mixed_precision();
    checksum ^= test_with_constants();
    checksum ^= test_function_results();
    
    /* Additional complex expressions to force code generation */
    volatile double x = 1.5;
    volatile double y = 2.5;
    volatile double z = __builtin_nan("");
    
    /* Use ternary operator with floating-point conditions */
    double t = (__builtin_isunordered(x, z)) ? x : y;
    checksum ^= (int)t;
    
    /* Array indexing based on FP comparison */
    int arr[4] = {0, 0, 0, 0};
    int idx = (__builtin_islessgreater(x, y)) ? 0 : 
              ((!(x <= y)) ? 1 : 2);  /* UNGT */
    arr[idx] = 1;
    checksum ^= arr[0] ^ arr[1] ^ arr[2];
    
    /* Loop with FP condition */
    int count = 0;
    while (count < 3 && !(__builtin_isunordered(vd1 + count, vd_nan))) {
        count++;
    }
    checksum ^= count;
    
    printf("Final checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates all tests contributed to result)\n");
    
    return checksum != 0 ? 0 : 1;
}
