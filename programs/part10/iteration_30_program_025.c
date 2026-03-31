/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile int results[32] = {0};
volatile int result_index = 0;

/* Helper to store comparison results */
static void store_result(int res) {
    results[result_index++] = res;
}

/* Generate NaN values */
static double get_nan(void) {
    return __builtin_nan("");
}

static float get_nanf(void) {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code */
void test_unordered(void) {
    double nan = get_nan();
    float nanf = get_nanf();
    
    /* Direct unordered checks */
    int res1 = __builtin_isunordered(vd1, nan);
    int res2 = __builtin_isunordered(nan, vd2);
    int res3 = __builtin_isunordered(nan, nan);
    int res4 = __builtin_isunordered(vf1, nanf);
    
    /* Using !(a == a) pattern */
    double d = sqrt(-1.0);  /* Produces NaN */
    int res5 = !(d == d);
    
    /* Mixed types */
    int res6 = __builtin_isunordered(vd1, (double)nanf);
    
    store_result(res1);
    store_result(res2);
    store_result(res3);
    store_result(res4);
    store_result(res5);
    store_result(res6);
}

/* Test ORDERED condition code */
void test_ordered(void) {
    double nan = get_nan();
    
    /* Ordered checks */
    int res1 = !__builtin_isunordered(vd1, vd2);
    int res2 = !__builtin_isunordered(vd1, 0.0);
    int res3 = !__builtin_isunordered(3.14, 2.71);
    
    /* Using __builtin_isfinite */
    int res4 = __builtin_isfinite(vd1) && __builtin_isfinite(vd2);
    
    /* With function returns */
    double d1 = sin(1.0);
    double d2 = cos(1.0);
    int res5 = !__builtin_isunordered(d1, d2);
    
    store_result(res1);
    store_result(res2);
    store_result(res3);
    store_result(res4);
    store_result(res5);
}

/* Test UNEQ (unordered or equal) */
void test_uneq(void) {
    double nan = get_nan();
    
    /* Using builtin */
    int res1 = !__builtin_islessgreater(vd1, vd2);
    
    /* Manual implementation */
    int res2 = __builtin_isunordered(vd1, vd2) || (vd1 == vd2);
    
    /* With NaN */
    int res3 = !__builtin_islessgreater(vd1, nan);
    
    /* Float version */
    float nanf = get_nanf();
    int res4 = !__builtin_islessgreaterf(vf1, nanf);
    
    /* Constants */
    int res5 = !__builtin_islessgreater(0.0, -0.0);  /* Should be true */
    
    store_result(res1);
    store_result(res2);
    store_result(res3);
    store_result(res4);
    store_result(res5);
}

/* Test UNGE (not less than) - generates "nlt" */
void test_unge(void) {
    /* Inverse of < */
    int res1 = !(vd1 < vd2);
    int res2 = !(vf1 < vf2);
    
    /* With constants */
    int res3 = !(3.0 < 2.0);
    int res4 = !(0.0 < -0.0);
    
    /* Mixed precision */
    int res5 = !((double)vf1 < vd2);
    
    /* Using >= directly (should also work) */
    int res6 = (vd1 >= vd2);
    
    store_result(res1);
    store_result(res2);
    store_result(res3);
    store_result(res4);
    store_result(res5);
    store_result(res6);
}

/* Test UNGT (not less than or equal) - generates "nle" */
void test_ungt(void) {
    /* Inverse of <= */
    int res1 = !(vd1 <= vd2);
    int res2 = !(vf1 <= vf2);
    
    /* With NaN */
    double nan = get_nan();
    int res3 = !(vd1 <= nan);
    
    /* Using > directly */
    int res4 = (vd2 > vd1);
    
    /* Complex expression */
    int res5 = !((vd1 * 2.0) <= (vd2 + 1.0));
    
    store_result(res1);
    store_result(res2);
    store_result(res3);
    store_result(res4);
    store_result(res5);
}

/* Test UNLE (unordered or less than or equal) - generates "ule" */
void test_unle(void) {
    double nan = get_nan();
    
    /* Using builtin */
    int res1 = !__builtin_isgreater(vd1, vd2);
    
    /* Manual with unordered check */
    int res2 = __builtin_isunordered(vd1, vd2) || (vd1 <= vd2);
    
    /* With NaN */
    int res3 = !__builtin_isgreater(vd1, nan);
    
    /* Float version */
    int res4 = !__builtin_isgreaterf(vf1, vf2);
    
    store_result(res1);
    store_result(res2);
    store_result(res3);
    store_result(res4);
}

/* Test UNLT (unordered or less than) - generates "ult" */
void test_unlt(void) {
    double nan = get_nan();
    
    /* Using builtin */
    int res1 = !__builtin_isgreaterequal(vd1, vd2);
    
    /* Manual with unordered check */
    int res2 = __builtin_isunordered(vd1, vd2) || (vd1 < vd2);
    
    /* With NaN */
    int res3 = !__builtin_isgreaterequal(nan, vd2);
    
    store_result(res1);
    store_result(res2);
    store_result(res3);
}

/* Test LTGT (less than or greater than) - generates "une" */
void test_ltgt(void) {
    /* Direct builtin */
    int res1 = __builtin_islessgreater(vd1, vd2);
    int res2 = __builtin_islessgreaterf(vf1, vf2);
    
    /* Manual: ordered and not equal */
    int res3 = !__builtin_isunordered(vd1, vd2) && (vd1 != vd2);
    
    /* With constants */
    int res4 = __builtin_islessgreater(1.0, 2.0);
    int res5 = __builtin_islessgreater(0.0, -0.0);  /* Should be false */
    
    /* Using || of < and > */
    int res6 = (vd1 < vd2) || (vd1 > vd2);
    
    store_result(res1);
    store_result(res2);
    store_result(res3);
    store_result(res4);
    store_result(res5);
    store_result(res6);
}

/* Test condition codes in control flow */
void test_control_flow(void) {
    double nan = get_nan();
    float nanf = get_nanf();
    int count = 0;
    
    /* if with unordered */
    if (__builtin_isunordered(vd1, nan)) {
        count++;
    }
    
    /* while with ordered */
    volatile double d = 0.0;
    while (!__builtin_isunordered(d, d)) {
        d += 1.0;
        if (d > 5.0) break;
        count++;
    }
    
    /* Ternary operator with UNGE */
    int res1 = !(vd1 < vd2) ? 1 : 0;
    
    /* Switch-like with comparisons */
    for (int i = 0; i < 3; i++) {
        double val = (double)i;
        if (!__builtin_islessgreater(val, 1.0)) {  /* UNEQ */
            count++;
        }
        if (!__builtin_isgreater(val, 1.0)) {      /* UNLE */
            count++;
        }
    }
    
    store_result(count);
    store_result(res1);
}

/* Test with inline assembly for direct control */
#ifdef USE_ASM
void test_asm(void) {
    double a = 1.0;
    double b = 2.0;
    int res;
    
    /* UNORDERED check */
    __asm__ volatile (
        "fucomip %1, %0\n\t"
        "setp %2"
        : "=t"(a), "+u"(b), "=r"(res)
        : "0"(a), "1"(b)
        : "cc"
    );
    store_result(res);
    
    /* ORDERED check */
    __asm__ volatile (
        "fucomip %1, %0\n\t"
        "setnp %2"
        : "=t"(a), "+u"(b), "=r"(res)
        : "0"(a), "1"(b)
        : "cc"
    );
    store_result(res);
}
#endif

int main(void) {
    /* Initialize with various values */
    vd1 = 1.5;
    vd2 = 2.5;
    vf1 = 1.5f;
    vf2 = 2.5f;
    
    /* Run all tests */
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_control_flow();
    
#ifdef USE_ASM
    test_asm();
#endif
    
    /* Compute checksum to ensure all comparisons affect output */
    int checksum = 0;
    for (int i = 0; i < result_index; i++) {
        checksum = checksum * 31 + results[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    printf("Total comparisons: %d\n", result_index);
    
    return 0;
}
