/* test_conditions.c - Program to trigger x86 floating-point condition code generation */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = 0.0/0.0;
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = 0.0f/0.0f;

/* Function prototypes for different condition code patterns */
void test_unordered(void);
void test_ordered(void);
void test_uneq(void);
void test_unge(void);
void test_ungt(void);
void test_unle(void);
void test_unlt(void);
void test_ltgt(void);

/* Helper to generate NaN */
double make_nan(void) {
    return __builtin_nan("");
}

float make_nanf(void) {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code */
void test_unordered(void) {
    double d1 = vd1;
    double d2 = vd_nan;
    float f1 = vf1;
    float f2 = vf_nan;
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d2)) result |= 1;
    if (__builtin_isunordered(f1, f2)) result |= 2;
    
    /* Using NaN property */
    if (d1 != d1 || d2 != d2) result |= 4;
    
    /* Mixed types */
    if (__builtin_isunordered(d1, f2)) result |= 8;
    
    printf("UNORDERED test result: %d\n", result);
}

/* Test ORDERED condition code */
void test_ordered(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = make_nan();
    float f1 = vf1;
    float f2 = vf2;
    int result = 0;
    
    /* Direct ordered checks */
    if (__builtin_isordered(d1, d2)) result |= 1;
    if (__builtin_isordered(f1, f2)) result |= 2;
    
    /* Inverse of unordered */
    if (!__builtin_isunordered(d1, d2)) result |= 4;
    
    /* With NaN */
    if (!__builtin_isunordered(d1, d_nan)) result |= 8;
    
    printf("ORDERED test result: %d\n", result);
}

/* Test UNEQ condition code */
void test_uneq(void) {
    double d1 = vd1;
    double d2 = vd1;  /* Equal values */
    double d_nan = make_nan();
    float f1 = vf1;
    float f2 = vf1;
    int result = 0;
    
    /* Using builtin */
    if (!__builtin_islessgreater(d1, d2)) result |= 1;
    
    /* With NaN - should be unordered, not equal */
    if (!__builtin_islessgreater(d1, d_nan)) result |= 2;
    
    /* Float version */
    if (!__builtin_islessgreater(f1, f2)) result |= 4;
    
    printf("UNEQ test result: %d\n", result);
}

/* Test UNGE condition code (nlt) */
void test_unge(void) {
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d_nan = make_nan();
    float f1 = vf2;
    float f2 = vf1;
    int result = 0;
    
    /* Inverse of less than */
    if (!(d1 < d2)) result |= 1;      /* Should generate nlt */
    if (!(f1 < f2)) result |= 2;
    
    /* With function calls */
    if (!(sin(d1) < cos(d2))) result |= 4;
    
    /* With NaN */
    if (!(d_nan < d1)) result |= 8;
    
    printf("UNGE test result: %d\n", result);
}

/* Test UNGT condition code (nle) */
void test_ungt(void) {
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d_same = vd1;
    float f1 = vf2;
    float f2 = vf1;
    int result = 0;
    
    /* Inverse of less or equal */
    if (!(d1 <= d2)) result |= 1;     /* Should generate nle */
    if (!(f1 <= f2)) result |= 2;
    
    /* Equal case */
    if (!(d1 <= d_same)) result |= 4;
    
    printf("UNGT test result: %d\n", result);
}

/* Test UNLE condition code */
void test_unle(void) {
    double d1 = vd1;  /* 1.0 */
    double d2 = vd2;  /* 2.0 */
    double d_nan = make_nan();
    float f1 = vf1;
    float f2 = vf2;
    int result = 0;
    
    /* Using builtin with unordered */
    if (__builtin_islessequal(d1, d2)) result |= 1;
    if (__builtin_islessequal(f1, f2)) result |= 2;
    
    /* With NaN */
    if (__builtin_islessequal(d_nan, d1)) result |= 4;
    
    printf("UNLE test result: %d\n", result);
}

/* Test UNLT condition code */
void test_unlt(void) {
    double d1 = vd1;  /* 1.0 */
    double d2 = vd2;  /* 2.0 */
    double d_nan = make_nan();
    float f1 = vf1;
    float f2 = vf2;
    int result = 0;
    
    /* Using builtin with unordered */
    if (__builtin_isless(d1, d2)) result |= 1;
    if (__builtin_isless(f1, f2)) result |= 2;
    
    /* With NaN */
    if (__builtin_isless(d_nan, d1)) result |= 4;
    
    printf("UNLT test result: %d\n", result);
}

/* Test LTGT condition code (une) */
void test_ltgt(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_same = vd1;
    double d_nan = make_nan();
    float f1 = vf1;
    float f2 = vf2;
    int result = 0;
    
    /* Direct builtin */
    if (__builtin_islessgreater(d1, d2)) result |= 1;
    if (__builtin_islessgreater(f1, f2)) result |= 2;
    
    /* Equal values */
    if (__builtin_islessgreater(d1, d_same)) result |= 4;
    
    /* With NaN */
    if (__builtin_islessgreater(d1, d_nan)) result |= 8;
    
    /* Manual ordered comparison */
    if ((d1 < d2) || (d1 > d2)) result |= 16;
    
    printf("LTGT test result: %d\n", result);
}

/* Advanced: Direct assembly templates for specific control */
#ifdef USE_ASM
void test_asm_conditions(void) {
    double a = vd1;
    double b = vd_nan;
    int res_unord, res_ord, res_une;
    
    /* UNORDERED test with asm */
    __asm__ volatile (
        "fucomi %2, %1\n\t"
        "setp %0"
        : "=r"(res_unord)
        : "t"(a), "u"(b)
        : "cc"
    );
    
    /* ORDERED test with asm */
    __asm__ volatile (
        "fucomi %2, %1\n\t"
        "setnp %0"
        : "=r"(res_ord)
        : "t"(a), "u"(vd2)
        : "cc"
    );
    
    /* LTGT (une) test with asm */
    __asm__ volatile (
        "fucomi %2, %1\n\t"
        "setne %0"
        : "=r"(res_une)
        : "t"(a), "u"(vd2)
        : "cc"
    );
    
    printf("ASM results: unord=%d, ord=%d, une=%d\n", 
           res_unord, res_ord, res_une);
}
#endif

/* Main driver that uses all comparisons in control flow */
int main(void) {
    unsigned int checksum = 0;
    
    printf("=== Testing x86 Floating-Point Condition Codes ===\n\n");
    
    /* Call all test functions */
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    
#ifdef USE_ASM
    test_asm_conditions();
#endif
    
    /* Additional runtime tests with varied values */
    double values[] = {0.0, -0.0, 1.0, -1.0, INFINITY, -INFINITY, NAN};
    int n = sizeof(values)/sizeof(values[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Use all condition types in control flow */
            if (__builtin_isunordered(a, b)) checksum += 1;
            if (__builtin_isordered(a, b)) checksum += 2;
            if (!__builtin_islessgreater(a, b)) checksum += 4;  /* UNEQ */
            if (!(a < b)) checksum += 8;                        /* UNGE */
            if (!(a <= b)) checksum += 16;                      /* UNGT */
            if (__builtin_islessequal(a, b)) checksum += 32;    /* UNLE */
            if (__builtin_isless(a, b)) checksum += 64;         /* UNLT */
            if (__builtin_islessgreater(a, b)) checksum += 128; /* LTGT */
        }
    }
    
    printf("\nFinal checksum: %u\n", checksum);
    return (int)checksum;
}
