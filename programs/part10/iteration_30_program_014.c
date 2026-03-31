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
    return sqrt(-1.0);
}

/* Function to generate infinity */
double make_inf() {
    return 1.0 / 0.0;
}

/* Test UNORDERED condition code (unord) */
int test_unordered() {
    double d1 = vd_nan;
    double d2 = vd1;
    float f1 = vf_nan;
    float f2 = vf1;
    
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d2)) result |= 1;
    if (__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Using !(a == a) to detect NaN */
    double d3 = make_nan();
    if (!(d3 == d3)) result |= 4;
    
    /* Mixed types */
    if (__builtin_isunordered(d1, f2)) result |= 8;
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered() {
    double d1 = vd1;
    double d2 = vd2;
    float f1 = vf1;
    float f2 = vf2;
    
    int result = 0;
    
    /* Ordered comparisons */
    if (!__builtin_isunordered(d1, d2)) result |= 1;
    if (!__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Compare with constant */
    if (!__builtin_isunordered(d1, 0.0)) result |= 4;
    
    /* Function return */
    double d3 = sin(1.0);
    if (!__builtin_isunordered(d3, d1)) result |= 8;
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq() {
    double d1 = vd1;
    double d2 = vd1;  /* Equal values */
    double d_nan = vd_nan;
    
    int result = 0;
    
    /* Unordered or equal */
    if (__builtin_isunordered(d1, d2) || d1 == d2) result |= 1;
    
    /* With NaN */
    if (__builtin_isunordered(d_nan, d1) || d_nan == d1) result |= 2;
    
    /* Using float */
    float f1 = vf1;
    float f2 = vf1;
    if (__builtin_isunordered(f1, f2) || f1 == f2) result |= 4;
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge() {
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d_nan = vd_nan;
    
    int result = 0;
    
    /* Not less than (>= or unordered) */
    if (!(d1 < d2)) result |= 1;  /* Should generate nlt */
    
    /* With NaN */
    if (!(d_nan < d1)) result |= 2;
    
    /* Mixed comparison */
    float f1 = vf2;
    double d3 = 1.5;
    if (!(f1 < d3)) result |= 4;
    
    /* Using volatile */
    if (!(vd1 < vd2)) result |= 8;
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt() {
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d_nan = vd_nan;
    
    int result = 0;
    
    /* Not less or equal (> or unordered) */
    if (!(d1 <= d2)) result |= 1;  /* Should generate nle */
    
    /* With NaN */
    if (!(d_nan <= d1)) result |= 2;
    
    /* Compare with zero */
    if (!(d1 <= 0.0)) result |= 4;
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle() {
    double d1 = vd1;  /* 1.0 */
    double d2 = vd2;  /* 2.0 */
    double d_nan = vd_nan;
    
    int result = 0;
    
    /* Unordered or less or equal */
    if (__builtin_isunordered(d1, d2) || d1 <= d2) result |= 1;
    
    /* With NaN */
    if (__builtin_isunordered(d_nan, d1) || d_nan <= d1) result |= 2;
    
    /* Using float */
    float f1 = 1.0f;
    float f2 = 2.0f;
    if (__builtin_isunordered(f1, f2) || f1 <= f2) result |= 4;
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt() {
    double d1 = vd1;  /* 1.0 */
    double d2 = vd2;  /* 2.0 */
    double d_nan = vd_nan;
    
    int result = 0;
    
    /* Unordered or less than */
    if (__builtin_isunordered(d1, d2) || d1 < d2) result |= 1;
    
    /* With NaN */
    if (__builtin_isunordered(d_nan, d1) || d_nan < d1) result |= 2;
    
    /* Compare with constant */
    if (__builtin_isunordered(d1, 3.0) || d1 < 3.0) result |= 4;
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt() {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = vd_nan;
    
    int result = 0;
    
    /* Less or greater (ordered and not equal) */
    if (__builtin_islessgreater(d1, d2)) result |= 1;
    
    /* With NaN (should be false) */
    if (__builtin_islessgreater(d_nan, d1)) result |= 2;
    
    /* Manual ordered comparison */
    if ((d1 < d2) || (d1 > d2)) result |= 4;
    
    /* Using float */
    float f1 = vf1;
    float f2 = vf2;
    if (__builtin_islessgreater(f1, f2)) result |= 8;
    
    return result;
}

/* Test mixed condition codes in control flow */
int test_mixed_control_flow() {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    int result = 0;
    
    /* Complex if-else chain with different conditions */
    if (__builtin_isunordered(a, nan)) {
        result += 1;
    } else if (!__builtin_isunordered(a, b)) {
        result += 2;
    }
    
    /* Ternary operator with unordered check */
    result += __builtin_isunordered(b, nan) ? 4 : 8;
    
    /* While loop with ordered check */
    int count = 0;
    while (count < 3 && !__builtin_isunordered(a + count, b)) {
        result += 16;
        count++;
    }
    
    return result;
}

/* Test with inline assembly for direct control */
int test_asm_direct() {
    double a = vd1;
    double b = vd2;
    int res1 = 0, res2 = 0, res3 = 0;
    
    /* Using inline assembly to potentially trigger condition codes */
    #ifdef __x86_64__
    __asm__ volatile (
        "comisd %1, %0\n\t"
        "seta %2"
        : "=r" (res1)
        : "x" (a), "x" (b), "0" (res1)
        : "cc"
    );
    #endif
    
    return res1 + res2 + res3;
}

/* Main driver function */
int main() {
    int checksum = 0;
    
    printf("Testing floating-point condition codes...\n");
    
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
    
    checksum ^= test_mixed_control_flow();
    printf("test_mixed_control_flow: %d\n", test_mixed_control_flow());
    
    checksum ^= test_asm_direct();
    printf("test_asm_direct: %d\n", test_asm_direct());
    
    printf("Final checksum: %d\n", checksum);
    
    /* Use checksum in a way that can't be optimized away */
    volatile int* dummy = (volatile int*)&checksum;
    return *dummy;
}
