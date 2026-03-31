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
double get_nan(void) {
    return __builtin_nan("");
}

/* Function to generate infinity */
double get_inf(void) {
    return __builtin_inf();
}

/* Test UNORDERED condition code (unord) */
int test_unordered(void) {
    double a = vd_nan;
    double b = vd1;
    float c = vf_nan;
    float d = vf1;
    
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(a, b)) result |= 1;
    if (__builtin_isunordered(c, d)) result |= 2;
    
    /* Compare NaN with normal value */
    if (a != a) result |= 4;  /* NaN check */
    if (c != c) result |= 8;  /* NaN check */
    
    /* Mixed types */
    if (__builtin_isunordered(a, vf1)) result |= 16;
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered(void) {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    int result = 0;
    
    /* Direct ordered checks */
    if (__builtin_isordered(a, b)) result |= 1;
    if (__builtin_isordered(b, a)) result |= 2;
    
    /* Ordered check with NaN */
    if (!__builtin_isunordered(a, c)) result |= 4;
    
    /* Float ordered checks */
    if (__builtin_isordered(vf1, vf2)) result |= 8;
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq(void) {
    double a = vd1;
    double b = vd1;  /* Equal values */
    double c = vd_nan;
    
    int result = 0;
    
    /* Unordered or equal */
    if (!__builtin_islessgreater(a, b)) result |= 1;
    
    /* Compare with NaN (unordered case) */
    if (!__builtin_islessgreater(a, c)) result |= 2;
    
    /* Using !(a < b || a > b) which is UNEQ for unordered or equal */
    if (!(a < b || a > b)) result |= 4;
    
    /* Float version */
    if (!__builtin_islessgreater(vf1, vf1)) result |= 8;
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge(void) {
    double a = vd2;
    double b = vd1;
    double c = vd_nan;
    
    int result = 0;
    
    /* Not less than (greater or equal OR unordered) */
    if (!(a < b)) result |= 1;      /* nlt - UNGE */
    
    /* With NaN (unordered case should be true) */
    if (!(c < a)) result |= 2;      /* nlt - UNGE */
    
    /* Float version */
    if (!(vf2 < vf1)) result |= 4;
    
    /* Mixed: double !< float */
    if (!(vd2 < vf1)) result |= 8;
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt(void) {
    double a = vd2;
    double b = vd1;
    double c = vd_nan;
    
    int result = 0;
    
    /* Not less or equal (greater OR unordered) */
    if (!(a <= b)) result |= 1;     /* nle - UNGT */
    
    /* With NaN */
    if (!(c <= a)) result |= 2;     /* nle - UNGT */
    
    /* Float version */
    if (!(vf2 <= vf1)) result |= 4;
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle(void) {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    int result = 0;
    
    /* Unordered or less or equal */
    if (!(a > b)) result |= 1;      /* !gt - UNLE */
    
    /* With NaN */
    if (!(c > a)) result |= 2;      /* !gt - UNLE */
    
    /* Float version */
    if (!(vf1 > vf2)) result |= 4;
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt(void) {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    int result = 0;
    
    /* Unordered or less than */
    if (!(a >= b)) result |= 1;     /* !ge - UNLT */
    
    /* With NaN */
    if (!(c >= a)) result |= 2;     /* !ge - UNLT */
    
    /* Float version */
    if (!(vf1 >= vf2)) result |= 4;
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt(void) {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    int result = 0;
    
    /* Less or greater (ordered and not equal) */
    if (__builtin_islessgreater(a, b)) result |= 1;
    
    /* With NaN (should be false for ordered lessgreater) */
    if (__builtin_islessgreater(c, a)) result |= 2;
    
    /* Using ordered comparison: (a < b) || (a > b) */
    if ((a < b) || (a > b)) result |= 4;
    
    /* Float version */
    if (__builtin_islessgreater(vf1, vf2)) result |= 8;
    
    return result;
}

/* Test mixed conditions with function calls */
int test_mixed(void) {
    int result = 0;
    
    /* Compare function results that may produce NaN */
    double nan_val = sqrt(-1.0);  /* Should produce NaN */
    double inf_val = 1.0 / 0.0;   /* Should produce Inf */
    
    /* Various comparisons that should generate different condition codes */
    if (__builtin_isunordered(nan_val, 0.0)) result |= 1;
    if (__builtin_isordered(inf_val, 1.0)) result |= 2;
    if (!__builtin_islessgreater(nan_val, inf_val)) result |= 4;
    if (!(nan_val < 0.0)) result |= 8;          /* UNGE */
    if (!(nan_val <= 0.0)) result |= 16;        /* UNGT */
    if (!(nan_val > 0.0)) result |= 32;         /* UNLE */
    if (!(nan_val >= 0.0)) result |= 64;        /* UNLT */
    if (__builtin_islessgreater(inf_val, 1.0)) result |= 128; /* LTGT */
    
    return result;
}

/* Test with conditional moves */
int test_cmov(void) {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    int x = 0, y = 0, z = 0;
    
    /* These might generate cmov instructions with condition codes */
    x = (__builtin_isunordered(a, c)) ? 100 : 200;
    y = (!__builtin_islessgreater(a, b)) ? 300 : 400;
    z = (!(a < b)) ? 500 : 600;  /* UNGE */
    
    return x + y + z;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Call all test functions */
    checksum += test_unordered();
    printf("test_unordered completed\n");
    
    checksum += test_ordered();
    printf("test_ordered completed\n");
    
    checksum += test_uneq();
    printf("test_uneq completed\n");
    
    checksum += test_unge();
    printf("test_unge completed\n");
    
    checksum += test_ungt();
    printf("test_ungt completed\n");
    
    checksum += test_unle();
    printf("test_unle completed\n");
    
    checksum += test_unlt();
    printf("test_unlt completed\n");
    
    checksum += test_ltgt();
    printf("test_ltgt completed\n");
    
    checksum += test_mixed();
    printf("test_mixed completed\n");
    
    checksum += test_cmov();
    printf("test_cmov completed\n");
    
    /* Additional runtime tests with loops */
    volatile double arr[4] = {1.0, 2.0, __builtin_nan(""), 3.0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (__builtin_isunordered(arr[i], arr[j])) checksum++;
            if (!__builtin_islessgreater(arr[i], arr[j])) checksum += 2;
            if (!(arr[i] < arr[j])) checksum += 3;  /* UNGE */
            if (!(arr[i] <= arr[j])) checksum += 4; /* UNGT */
            if (!(arr[i] > arr[j])) checksum += 5;  /* UNLE */
            if (!(arr[i] >= arr[j])) checksum += 6; /* UNLT */
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Return non-zero if any test failed (simplistic check) */
    return (checksum == 0) ? 1 : 0;
}
