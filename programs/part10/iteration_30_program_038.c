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

static float make_nanf(void) {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code (unord) */
int test_unordered(void) {
    float f1 = make_nanf();
    float f2 = 1.5f;
    double d1 = make_nan();
    double d2 = 2.5;
    
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(f1, f2)) result |= 1;
    if (__builtin_isunordered(d1, d2)) result |= 2;
    
    /* Using volatile variables */
    if (__builtin_isunordered(vf_nan, vf1)) result |= 4;
    if (__builtin_isunordered(vd_nan, vd1)) result |= 8;
    
    /* Manual NaN check */
    if (!(f1 == f1)) result |= 16;
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered(void) {
    float f1 = 1.0f;
    float f2 = 2.0f;
    double d1 = 3.0;
    double d2 = 4.0;
    
    int result = 0;
    
    /* Direct ordered checks */
    if (__builtin_isordered(f1, f2)) result |= 1;
    if (__builtin_isordered(d1, d2)) result |= 2;
    
    /* Ordered check with constants */
    if (__builtin_isordered(1.0f, 2.0f)) result |= 4;
    if (__builtin_isordered(3.0, 4.0)) result |= 8;
    
    /* Using function returns */
    float f_sqrt = sqrtf(4.0f);
    double d_sqrt = sqrt(9.0);
    if (__builtin_isordered(f_sqrt, 2.0f)) result |= 16;
    if (__builtin_isordered(d_sqrt, 3.0)) result |= 32;
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq(void) {
    float f1 = 1.0f;
    float f2 = 1.0f;
    double d1 = 2.0;
    double d2 = 2.0;
    
    int result = 0;
    
    /* Compare equal values */
    if (f1 == f2) result |= 1;
    if (d1 == d2) result |= 2;
    
    /* Compare with NaN (should be false with -fno-fast-math) */
    float f_nan = make_nanf();
    if (!(f_nan == f1)) result |= 4;
    
    /* Mixed precision */
    if ((double)f1 == d1) result |= 8;
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge(void) {
    float f1 = 2.0f;
    float f2 = 1.0f;
    double d1 = 4.0;
    double d2 = 3.0;
    
    int result = 0;
    
    /* Inverse conditions: !(a < b) generates nlt */
    if (!(f1 < f2)) result |= 1;
    if (!(d1 < d2)) result |= 2;
    
    /* Using volatile */
    if (!(vf1 < vf2)) result |= 4;
    if (!(vd1 < vd2)) result |= 8;
    
    /* With constants */
    if (!(3.0f < 2.0f)) result |= 16;
    if (!(5.0 < 4.0)) result |= 32;
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt(void) {
    float f1 = 2.0f;
    float f2 = 1.0f;
    double d1 = 4.0;
    double d2 = 3.0;
    
    int result = 0;
    
    /* Inverse conditions: !(a <= b) generates nle */
    if (!(f1 <= f2)) result |= 1;
    if (!(d1 <= d2)) result |= 2;
    
    /* With NaN handling */
    float f_nan = make_nanf();
    if (!(f_nan <= f1)) result |= 4;
    
    /* Mixed comparisons */
    if (!((double)f1 <= d2)) result |= 8;
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle(void) {
    float f1 = 1.0f;
    float f2 = 2.0f;
    double d1 = 3.0;
    double d2 = 4.0;
    
    int result = 0;
    
    /* Using <= with unordered possibility */
    if (f1 <= f2) result |= 1;
    if (d1 <= d2) result |= 2;
    
    /* With volatile */
    if (vf1 <= vf2) result |= 4;
    if (vd1 <= vd2) result |= 8;
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt(void) {
    float f1 = 1.0f;
    float f2 = 2.0f;
    double d1 = 3.0;
    double d2 = 4.0;
    
    int result = 0;
    
    /* Using < with unordered possibility */
    if (f1 < f2) result |= 1;
    if (d1 < d2) result |= 2;
    
    /* With function calls */
    float f_sqrt = sqrtf(2.0f);
    if (f_sqrt < 2.0f) result |= 4;
    
    /* Mixed precision */
    if ((float)d1 < f2) result |= 8;
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt(void) {
    float f1 = 1.0f;
    float f2 = 2.0f;
    double d1 = 3.0;
    double d2 = 4.0;
    
    int result = 0;
    
    /* Direct builtin for lessgreater */
    if (__builtin_islessgreater(f1, f2)) result |= 1;
    if (__builtin_islessgreater(d1, d2)) result |= 2;
    
    /* Manual ordered inequality: (a < b) || (a > b) */
    if ((f1 < f2) || (f1 > f2)) result |= 4;
    if ((d1 < d2) || (d1 > d2)) result |= 8;
    
    /* With NaN (should be false) */
    float f_nan = make_nanf();
    if (!__builtin_islessgreater(f_nan, f1)) result |= 16;
    
    return result;
}

/* Advanced: Using inline assembly for direct control */
#ifdef USE_ASM
int test_asm_conditions(void) {
    double a = 1.0;
    double b = 2.0;
    int result_u, result_o, result_ue;
    
    /* Unordered check */
    __asm__ volatile (
        "fucomip %2, %1\n\t"
        "setp %0"
        : "=r"(result_u)
        : "t"(a), "u"(b)
        : "cc"
    );
    
    /* Ordered check */
    __asm__ volatile (
        "fucomip %2, %1\n\t"
        "setnp %0"
        : "=r"(result_o)
        : "t"(a), "u"(b)
        : "cc"
    );
    
    /* Not equal (unordered or not equal) */
    __asm__ volatile (
        "fucomip %2, %1\n\t"
        "setne %0"
        : "=r"(result_ue)
        : "t"(a), "u"(b)
        : "cc"
    );
    
    return result_u | (result_o << 8) | (result_ue << 16);
}
#endif

/* Main driver that uses all condition codes */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Call all test functions */
    checksum ^= test_unordered();
    printf("test_unordered: %d\n", checksum);
    
    checksum ^= test_ordered();
    printf("test_ordered: %d\n", checksum);
    
    checksum ^= test_uneq();
    printf("test_uneq: %d\n", checksum);
    
    checksum ^= test_unge();
    printf("test_unge: %d\n", checksum);
    
    checksum ^= test_ungt();
    printf("test_ungt: %d\n", checksum);
    
    checksum ^= test_unle();
    printf("test_unle: %d\n", checksum);
    
    checksum ^= test_unlt();
    printf("test_unlt: %d\n", checksum);
    
    checksum ^= test_ltgt();
    printf("test_ltgt: %d\n", checksum);
    
#ifdef USE_ASM
    checksum ^= test_asm_conditions();
    printf("test_asm_conditions: %d\n", checksum);
#endif
    
    /* Additional complex patterns */
    {
        volatile float a = 0.0f;
        volatile float b = -0.0f;
        volatile double c = INFINITY;
        volatile double d = -INFINITY;
        
        /* These should generate various condition codes */
        if (__builtin_isunordered(a, b)) checksum ^= 0x100;
        if (__builtin_isordered(c, d)) checksum ^= 0x200;
        if (!(a >= b)) checksum ^= 0x400;  /* nlt */
        if (!(c <= d)) checksum ^= 0x800;  /* nle */
        
        /* Array indexing based on FP comparisons */
        int array[4] = {0};
        int idx = 0;
        if (a < b) idx = 1;
        if (a <= b) idx = 2;
        if (a > b) idx = 3;
        checksum ^= array[idx];
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
