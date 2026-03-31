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
    int result = 0;
    double d1 = vd1;
    double d2 = vd_nan;
    float f1 = vf1;
    float f2 = vf_nan;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d2)) result |= 1;
    if (__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Using != self to detect NaN */
    if (d2 != d2) result |= 4;  /* Should be true for NaN */
    
    /* Mixed types */
    if (__builtin_isunordered(d1, (double)f2)) result |= 8;
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = get_nan();
    
    /* Direct ordered checks */
    if (__builtin_isordered(d1, d2)) result |= 1;
    if (!__builtin_isunordered(d1, d2)) result |= 2;  /* Alternative form */
    
    /* With constants */
    if (__builtin_isordered(3.14, 2.71)) result |= 4;
    
    /* Function return values */
    if (__builtin_isordered(sin(d1), cos(d2))) result |= 8;
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd1;  /* Same value */
    double d_nan = vd_nan;
    
    /* Using ordered equality */
    if (!__builtin_islessgreater(d1, d2)) result |= 1;
    
    /* With NaN - should be false for ordered comparison */
    if (!__builtin_islessgreater(d1, d_nan)) result |= 2;
    
    /* Using explicit check */
    if (__builtin_isunordered(d1, d2) || d1 == d2) result |= 4;
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = vd_nan;
    
    /* Inverse of less than */
    if (!(d1 < d2)) result |= 1;
    if (!(d2 < d1)) result |= 2;
    
    /* With NaN */
    if (!(d_nan < d1)) result |= 4;
    
    /* Using builtin */
    if (!__builtin_isless(d1, d2)) result |= 8;
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = vd_nan;
    
    /* Inverse of less or equal */
    if (!(d1 <= d2)) result |= 1;
    if (!(d2 <= d1)) result |= 2;
    
    /* With NaN */
    if (!(d_nan <= d1)) result |= 4;
    
    /* Using builtin */
    if (!__builtin_islessequal(d1, d2)) result |= 8;
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = vd_nan;
    
    /* Unordered or less or equal */
    if (__builtin_isunordered(d1, d2) || d1 <= d2) result |= 1;
    
    /* Alternative formulation */
    if (!(d1 > d2)) result |= 2;
    
    /* With NaN */
    if (__builtin_isunordered(d_nan, d1) || d_nan <= d1) result |= 4;
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = vd_nan;
    
    /* Unordered or less than */
    if (__builtin_isunordered(d1, d2) || d1 < d2) result |= 1;
    
    /* Alternative formulation */
    if (!(d1 >= d2)) result |= 2;
    
    /* With NaN */
    if (__builtin_isunordered(d_nan, d1) || d_nan < d1) result |= 4;
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt(void) {
    int result = 0;
    double d1 = vd1;
    double d2 = vd2;
    double d_same = vd1;
    
    /* Direct lessgreater check */
    if (__builtin_islessgreater(d1, d2)) result |= 1;
    if (__builtin_islessgreater(d2, d1)) result |= 2;
    
    /* Equal values should return false */
    if (!__builtin_islessgreater(d1, d_same)) result |= 4;
    
    /* Using explicit ordered comparison */
    if ((d1 < d2) || (d1 > d2)) result |= 8;
    
    return result;
}

/* Test mixed conditions in control flow */
int test_control_flow(void) {
    int result = 0;
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    float fa = vf1;
    float fb = vf2;
    
    /* Complex if-else chain with various conditions */
    if (__builtin_isunordered(a, nan)) {
        result |= 1;
    } else if (!__builtin_isless(a, b)) {  /* nlt */
        result |= 2;
    }
    
    /* Ternary operator with unordered check */
    int x = __builtin_isunordered(fa, fb) ? 4 : 8;
    result |= x;
    
    /* While loop with ordered check */
    int count = 0;
    while (count < 3 && __builtin_isordered(a + count, b)) {
        result += 16;
        count++;
    }
    
    /* Switch-like behavior using comparisons */
    if (!(a <= b)) {  /* nle */
        result |= 32;
    } else if (__builtin_islessgreater(a, b)) {  /* une */
        result |= 64;
    }
    
    return result;
}

/* Test with inline assembly for direct control */
int test_asm_direct(void) {
    double a = vd1;
    double b = vd2;
    int result = 0;
    
    /* Using inline assembly to force specific condition codes */
    #ifdef __GNUC__
    asm volatile (
        "fcomip %2, %1\n\t"
        "seta %0"
        : "=r"(result)
        : "t"(a), "u"(b)
        : "cc"
    );
    #endif
    
    return result;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests */
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    checksum ^= test_control_flow();
    
    #ifdef __GNUC__
    checksum ^= test_asm_direct();
    #endif
    
    /* Use results in array indexing to prevent dead code elimination */
    static const int multipliers[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int final_result = checksum * multipliers[checksum & 7];
    
    printf("Final checksum: %d\n", final_result);
    
    /* Return non-zero if any test failed (simplified check) */
    return (final_result == 0) ? 1 : 0;
}
