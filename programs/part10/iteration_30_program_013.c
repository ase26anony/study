/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = 0.0f/0.0f;
volatile double vd1 = 3.0;
volatile double vd2 = 4.0;
volatile double vd_nan = 0.0/0.0;

/* Function to generate NaN */
static double make_nan(void) {
    return __builtin_nan("");
}

/* Function to generate infinity */
static double make_inf(void) {
    return __builtin_inf();
}

/* Test UNORDERED condition code (unord) */
int test_unordered(void) {
    float f1 = vf1;
    float f2 = vf_nan;
    double d1 = vd1;
    double d2 = vd_nan;
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(f1, f2)) result |= 1;
    if (__builtin_isunordered(d1, d2)) result |= 2;
    
    /* Using != self to detect NaN */
    if (f2 != f2) result |= 4;
    if (d2 != d2) result |= 8;
    
    /* Mixed types */
    if (__builtin_isunordered(f1, d2)) result |= 16;
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered(void) {
    float f1 = vf1;
    float f2 = vf2;
    double d1 = vd1;
    double d2 = make_nan();
    int result = 0;
    
    /* Ordered checks - true when neither is NaN */
    if (__builtin_isordered(f1, f2)) result |= 1;
    if (!__builtin_isunordered(f1, f2)) result |= 2;  /* Alternative form */
    
    /* With NaN */
    if (!__builtin_isordered(d1, d2)) result |= 4;
    
    /* Compare with constants */
    if (__builtin_isordered(1.0f, 2.0f)) result |= 8;
    if (!__builtin_isordered(1.0, make_nan())) result |= 16;
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq(void) {
    float f1 = vf1;
    float f2 = vf1;  /* Equal values */
    double d1 = make_nan();
    double d2 = make_nan();
    int result = 0;
    
    /* UNEQ: unordered or equal */
    /* Using builtin that might generate ueq */
    if (!__builtin_islessgreater(f1, f2)) result |= 1;
    
    /* With NaN - both unordered */
    if (!__builtin_islessgreater(d1, d2)) result |= 2;
    
    /* Compare equal values */
    if (!__builtin_islessgreater(3.14, 3.14)) result |= 4;
    
    /* Using volatile to force reload */
    volatile float v1 = 5.0f;
    volatile float v2 = 5.0f;
    if (!__builtin_islessgreater(v1, v2)) result |= 8;
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge(void) {
    float f1 = vf1;
    float f2 = vf2;
    double d1 = vd1;
    double d2 = vd_nan;
    int result = 0;
    
    /* UNGE: unordered or not less than (nlt) */
    /* Using !(a < b) which can generate nlt */
    if (!(f1 < f2)) result |= 1;
    if (!(2.0f < 1.0f)) result |= 2;
    
    /* With NaN - should be true */
    if (!(d1 < d2)) result |= 4;
    
    /* Mixed precision */
    if (!(1.0f < 2.0)) result |= 8;
    
    /* In conditional expression */
    result |= (!(vf1 < vf2)) ? 16 : 0;
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt(void) {
    float f1 = vf2;
    float f2 = vf1;
    double d1 = vd_nan;
    double d2 = vd1;
    int result = 0;
    
    /* UNGT: unordered or not less than or equal (nle) */
    /* Using !(a <= b) which can generate nle */
    if (!(f2 <= f1)) result |= 1;
    if (!(1.0 <= 1.0)) result |= 2;  /* Equal case */
    
    /* With NaN */
    if (!(d2 <= d1)) result |= 4;
    
    /* Compare with zero */
    if (!(0.0f <= -0.0f)) result |= 8;  /* Negative zero */
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle(void) {
    float f1 = vf1;
    float f2 = vf2;
    double d1 = make_nan();
    double d2 = vd1;
    int result = 0;
    
    /* UNLE: unordered or less than or equal */
    /* This might generate ule directly */
    if (f1 <= f2) result |= 1;
    
    /* With NaN */
    if (d1 <= d2) result |= 2;
    
    /* Equal values */
    if (3.14 <= 3.14) result |= 4;
    
    /* In while loop condition */
    volatile float counter = 0.0f;
    while (counter <= 5.0f) {
        result++;
        counter += 1.0f;
    }
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt(void) {
    float f1 = vf1;
    float f2 = vf2;
    double d1 = vd_nan;
    double d2 = vd2;
    int result = 0;
    
    /* UNLT: unordered or less than */
    if (f1 < f2) result |= 1;
    
    /* With NaN */
    if (d1 < d2) result |= 2;
    
    /* Compare negative values */
    if (-5.0 < -3.0) result |= 4;
    
    /* Using function return values */
    if (sin(0.0) < sin(1.0)) result |= 8;
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt(void) {
    float f1 = vf1;
    float f2 = vf2;
    double d1 = make_nan();
    double d2 = make_nan();
    int result = 0;
    
    /* LTGT: less than or greater than (ordered and not equal) */
    /* Using __builtin_islessgreater */
    if (__builtin_islessgreater(f1, f2)) result |= 1;
    
    /* Equal values - should be false */
    if (!__builtin_islessgreater(1.0, 1.0)) result |= 2;
    
    /* With NaN - should be false */
    if (!__builtin_islessgreater(d1, d2)) result |= 4;
    
    /* Mixed comparison */
    if (__builtin_islessgreater(2.0f, 1.0f)) result |= 8;
    
    return result;
}

/* Test with inline assembly for direct control */
int test_asm_direct(void) {
    double a = vd1;
    double b = vd_nan;
    int res1 = 0, res2 = 0, res3 = 0;
    
    /* Using inline assembly to potentially trigger condition codes */
    /* Note: These are examples and may need adjustment for specific compilers */
    
    /* Test for unordered */
    __asm__ volatile (
        "fucomi %2, %1\n\t"
        "setp %0"
        : "=r"(res1)
        : "t"(a), "u"(b)
        : "cc"
    );
    
    /* Test for ordered */
    __asm__ volatile (
        "fucomi %2, %1\n\t"
        "setnp %0"
        : "=r"(res2)
        : "t"(a), "u"(3.14)
        : "cc"
    );
    
    return res1 + res2 + res3;
}

/* Main test driver */
int main(void) {
    unsigned int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests and accumulate checksum */
    checksum += test_unordered();
    printf("test_unordered: %d\n", test_unordered());
    
    checksum += test_ordered();
    printf("test_ordered: %d\n", test_ordered());
    
    checksum += test_uneq();
    printf("test_uneq: %d\n", test_uneq());
    
    checksum += test_unge();
    printf("test_unge: %d\n", test_unge());
    
    checksum += test_ungt();
    printf("test_ungt: %d\n", test_ungt());
    
    checksum += test_unle();
    printf("test_unle: %d\n", test_unle());
    
    checksum += test_unlt();
    printf("test_unlt: %d\n", test_unlt());
    
    checksum += test_ltgt();
    printf("test_ltgt: %d\n", test_ltgt());
    
    checksum += test_asm_direct();
    printf("test_asm_direct: %d\n", test_asm_direct());
    
    printf("\nFinal checksum: %u\n", checksum);
    
    /* Use checksum in a way that can't be optimized away */
    volatile unsigned int* dummy = (volatile unsigned int*)&checksum;
    return *dummy & 0xFF;
}
