/* test_conditions.c - Program to trigger x86 floating-point condition code generation */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;

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
    double nan = make_nan();
    float fnan = (float)nan;
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(vd1, nan)) result |= 1;
    if (__builtin_isunordered(nan, vd2)) result |= 2;
    if (__builtin_isunordered(vf1, fnan)) result |= 4;
    
    /* Using !(a == a) to detect NaN */
    volatile double d = nan;
    if (!(d == d)) result |= 8;
    
    /* Mixed precision unordered check */
    if (__builtin_isunordered(vd1, (double)vf2)) result |= 16;
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered(void) {
    double nan = make_nan();
    float fnan = (float)nan;
    int result = 0;
    
    /* Ordered checks */
    if (__builtin_isordered(vd1, vd2)) result |= 1;
    if (__builtin_isordered(vf1, vf2)) result |= 2;
    
    /* Ordered check with constant */
    if (__builtin_isordered(3.14159, 2.71828)) result |= 4;
    
    /* Ordered check after function call */
    double sqrt_val = sqrt(4.0);
    if (__builtin_isordered(sqrt_val, 2.0)) result |= 8;
    
    /* Check that NaN is not ordered */
    if (!__builtin_isordered(nan, vd1)) result |= 16;
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Using !(a < b || a > b) which for unordered gives ueq */
    volatile double a = vd1;
    volatile double b = vd1;  /* Equal values */
    
    if (!(a < b || a > b)) result |= 1;  /* Should be true for equal or unordered */
    
    /* Test with NaN */
    a = nan;
    b = vd1;
    if (!(a < b || a > b)) result |= 2;  /* Should be true for unordered */
    
    /* Mixed types */
    volatile float fa = vf1;
    volatile float fb = vf1;
    if (!(fa < fb || fa > fb)) result |= 4;
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge(void) {
    double nan = make_nan();
    int result = 0;
    
    /* !(a < b) generates nlt for unordered aware comparisons */
    if (!(vd1 < vd2)) result |= 1;
    if (!(vf1 < vf2)) result |= 2;
    
    /* With NaN */
    if (!(nan < vd1)) result |= 4;
    if (!(vd1 < nan)) result |= 8;
    
    /* Mixed precision */
    if (!((double)vf1 < vd2)) result |= 16;
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt(void) {
    double nan = make_nan();
    int result = 0;
    
    /* !(a <= b) generates nle */
    if (!(vd1 <= vd2)) result |= 1;
    if (!(vf1 <= vf2)) result |= 2;
    
    /* With NaN */
    if (!(nan <= vd1)) result |= 4;
    if (!(vd1 <= nan)) result |= 8;
    
    /* Using function return value */
    double val = sin(1.0);
    if (!(val <= 1.0)) result |= 16;
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Using <= with unordered awareness */
    volatile double a = vd1;
    volatile double b = vd2;
    
    /* These should generate ule when compiled with -fno-fast-math */
    if (a <= b) result |= 1;
    if (vf1 <= vf2) result |= 2;
    
    /* With NaN - result depends on unordered semantics */
    a = nan;
    if (a <= vd1) result |= 4;
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt(void) {
    double nan = make_nan();
    int result = 0;
    
    /* Using < with unordered awareness */
    if (vd1 < vd2) result |= 1;
    if (vf1 < vf2) result |= 2;
    
    /* With NaN */
    if (nan < vd1) result |= 4;
    if (vd1 < nan) result |= 8;
    
    /* Comparison with zero */
    if (vd1 < 0.0) result |= 16;
    if (-vd2 < 0.0) result |= 32;
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt(void) {
    double nan = make_nan();
    int result = 0;
    
    /* __builtin_islessgreater generates une */
    if (__builtin_islessgreater(vd1, vd2)) result |= 1;
    if (__builtin_islessgreater(vf1, vf2)) result |= 2;
    
    /* With NaN */
    if (__builtin_islessgreater(nan, vd1)) result |= 4;
    if (__builtin_islessgreater(vd1, nan)) result |= 8;
    
    /* Using (a < b) || (a > b) which may generate une */
    volatile double a = vd1;
    volatile double b = vd2;
    if (a < b || a > b) result |= 16;
    
    return result;
}

/* Test with inline assembly for direct control */
int test_asm_direct(void) {
    double a = vd1;
    double b = vd2;
    int result;
    
    /* Direct assembly with condition codes */
    __asm__ volatile (
        "fucomip %2, %1\n\t"
        "seta %0"
        : "=r"(result)
        : "t"(a), "u"(b)
        : "cc"
    );
    
    return result;
}

/* Main driver function */
int main(void) {
    unsigned int checksum = 0;
    
    printf("Testing floating-point condition codes...\n");
    
    /* Call all test functions and accumulate results */
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
    
    checksum = (checksum * 31) + test_asm_direct();
    printf("test_asm_direct: %d\n", test_asm_direct());
    
    /* Additional complex patterns */
    {
        volatile double x = make_nan();
        volatile double y = 0.0;
        volatile float fx = (float)x;
        volatile float fy = 0.0f;
        
        /* Mixed unordered comparisons */
        int r1 = __builtin_isunordered(x, y) ? 1 : 0;
        int r2 = __builtin_isunordered(fx, fy) ? 2 : 0;
        int r3 = !(x >= y) ? 4 : 0;  /* Should generate nlt (UNGE) */
        int r4 = !(fx >= fy) ? 8 : 0;
        
        checksum = (checksum * 31) + (r1 + r2 + r3 + r4);
        printf("mixed tests: %d\n", r1 + r2 + r3 + r4);
    }
    
    printf("Final checksum: %u\n", checksum);
    
    /* Use checksum in a way that can't be optimized away */
    volatile int* dummy = (volatile int*)&checksum;
    return *dummy & 0xFF;
}
