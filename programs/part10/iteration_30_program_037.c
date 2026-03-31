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

/* Function to generate checksum */
static inline int update_checksum(int checksum, int cond) {
    return (checksum * 31) + (cond ? 1 : 0);
}

/* Test UNORDERED condition code */
int test_unordered(int checksum) {
    double d1 = vd1;
    double d2 = vd_nan;
    float f1 = vf1;
    float f2 = vf_nan;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d2)) checksum = update_checksum(checksum, 1);
    if (__builtin_isunordered(f1, f2)) checksum = update_checksum(checksum, 1);
    
    /* Using NaN property */
    if (d1 != d1) checksum = update_checksum(checksum, 0);  /* false */
    if (d2 != d2) checksum = update_checksum(checksum, 1);  /* true */
    
    /* Mixed types */
    if (__builtin_isunordered(d1, f2)) checksum = update_checksum(checksum, 1);
    
    return checksum;
}

/* Test ORDERED condition code */
int test_ordered(int checksum) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = vd_nan;
    
    /* Ordered checks */
    if (__builtin_isordered(d1, d2)) checksum = update_checksum(checksum, 1);
    if (__builtin_isordered(d1, d_nan)) checksum = update_checksum(checksum, 0);
    
    /* Using !unordered */
    if (!__builtin_isunordered(d1, d2)) checksum = update_checksum(checksum, 1);
    
    /* With function return values */
    double sqrt_result = sqrt(-1.0);  /* Returns NaN */
    if (__builtin_isordered(d1, sqrt_result)) checksum = update_checksum(checksum, 0);
    
    return checksum;
}

/* Test UNEQ (unordered or equal) condition code */
int test_uneq(int checksum) {
    double d1 = vd1;
    double d2 = vd1;  /* Same value */
    double d_nan = vd_nan;
    
    /* This should generate UNEQ: !(a < b) && !(a > b) */
    if (!(d1 < d2) && !(d1 > d2)) checksum = update_checksum(checksum, 1);
    
    /* With NaN - should be true (unordered) */
    if (!(d1 < d_nan) && !(d1 > d_nan)) checksum = update_checksum(checksum, 1);
    
    /* Using == with NaN (always false for NaN) */
    if (d1 == d2) checksum = update_checksum(checksum, 1);
    if (d1 == d_nan) checksum = update_checksum(checksum, 0);
    
    return checksum;
}

/* Test UNGE (not less than) condition code */
int test_unge(int checksum) {
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d_nan = vd_nan;
    
    /* Inverse condition: !(a < b) */
    if (!(d2 < d1)) checksum = update_checksum(checksum, 1);  /* true: 1.0 < 2.0 */
    if (!(d1 < d2)) checksum = update_checksum(checksum, 0);  /* false: 2.0 < 1.0 */
    
    /* With NaN */
    if (!(d1 < d_nan)) checksum = update_checksum(checksum, 1);  /* true (unordered) */
    
    /* Using >= directly (might generate different code) */
    if (d1 >= d2) checksum = update_checksum(checksum, 1);
    
    return checksum;
}

/* Test UNGT (not less than or equal) condition code */
int test_ungt(int checksum) {
    double d1 = vd2;  /* 2.0 */
    double d2 = vd1;  /* 1.0 */
    double d_same = vd1; /* 1.0 */
    
    /* Inverse condition: !(a <= b) */
    if (!(d2 <= d1)) checksum = update_checksum(checksum, 0);  /* false: 1.0 <= 2.0 */
    if (!(d1 <= d2)) checksum = update_checksum(checksum, 1);  /* true: 2.0 <= 1.0 */
    if (!(d2 <= d_same)) checksum = update_checksum(checksum, 0);  /* false: 1.0 <= 1.0 */
    
    /* Using > directly */
    if (d1 > d2) checksum = update_checksum(checksum, 1);
    
    return checksum;
}

/* Test UNLE (unordered or less than or equal) condition code */
int test_unle(int checksum) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = vd_nan;
    
    /* This pattern might generate UNLE */
    if (d1 <= d2) checksum = update_checksum(checksum, 1);
    if (d2 <= d1) checksum = update_checksum(checksum, 0);
    
    /* With NaN - using <= with NaN always false, but unordered case... */
    int res = (d1 <= d_nan) || __builtin_isunordered(d1, d_nan);
    checksum = update_checksum(checksum, res);
    
    return checksum;
}

/* Test UNLT (unordered or less than) condition code */
int test_unlt(int checksum) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = vd_nan;
    
    /* Direct less than comparison */
    if (d1 < d2) checksum = update_checksum(checksum, 1);
    if (d2 < d1) checksum = update_checksum(checksum, 0);
    
    /* Pattern that might generate UNLT */
    int res = (d1 < d_nan) || __builtin_isunordered(d1, d_nan);
    checksum = update_checksum(checksum, res);
    
    return checksum;
}

/* Test LTGT (less than or greater than - unordered not equal) condition code */
int test_ltgt(int checksum) {
    double d1 = vd1;
    double d2 = vd2;
    double d_same = vd1;
    double d_nan = vd_nan;
    
    /* Using __builtin_islessgreater */
    if (__builtin_islessgreater(d1, d2)) checksum = update_checksum(checksum, 1);
    if (__builtin_islessgreater(d1, d_same)) checksum = update_checksum(checksum, 0);
    if (__builtin_islessgreater(d1, d_nan)) checksum = update_checksum(checksum, 0);
    
    /* Manual: (a < b) || (a > b) */
    if ((d1 < d2) || (d1 > d2)) checksum = update_checksum(checksum, 1);
    if ((d1 < d_same) || (d1 > d_same)) checksum = update_checksum(checksum, 0);
    
    return checksum;
}

/* Test with inline assembly for direct control */
int test_asm(int checksum) {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    int res;
    
    /* Using inline asm to potentially trigger specific condition codes */
    /* Note: These are examples and may need adjustment for specific compilers */
    
    /* Test for unordered */
    __asm__ volatile (
        "fucomi %1, %0\n\t"
        "setp %2"
        : "=t"(res)
        : "u"(b), "m"(res)
        : "cc"
    );
    checksum = update_checksum(checksum, res);
    
    return checksum;
}

/* Main test driver */
int main() {
    int checksum = 0;
    
    printf("Starting floating-point condition code tests...\n");
    
    /* Run all tests */
    checksum = test_unordered(checksum);
    checksum = test_ordered(checksum);
    checksum = test_uneq(checksum);
    checksum = test_unge(checksum);
    checksum = test_ungt(checksum);
    checksum = test_unle(checksum);
    checksum = test_unlt(checksum);
    checksum = test_ltgt(checksum);
    
    /* Test with various optimization barriers */
    {
        volatile double a = 0.0;
        volatile double b = -0.0;
        volatile double inf = vd_inf;
        
        /* Test with ±0.0 (equal but with different signs) */
        if (a == b) checksum = update_checksum(checksum, 1);
        
        /* Test with infinity */
        if (inf > vd1) checksum = update_checksum(checksum, 1);
        if (vd1 < inf) checksum = update_checksum(checksum, 1);
        if (__builtin_isunordered(inf, vd_nan)) checksum = update_checksum(checksum, 1);
    }
    
    /* Additional complex patterns */
    {
        double arr[4] = {1.0, 2.0, __builtin_nan(""), 4.0};
        for (int i = 0; i < 3; i++) {
            for (int j = i + 1; j < 4; j++) {
                /* Multiple comparison types in loop */
                if (__builtin_isunordered(arr[i], arr[j])) {
                    checksum = update_checksum(checksum, 1);
                }
                if (arr[i] < arr[j]) {
                    checksum = update_checksum(checksum, 1);
                }
                /* Ternary operator usage */
                double result = (arr[i] > arr[j]) ? arr[i] : arr[j];
                checksum = update_checksum(checksum, (int)(result * 10) % 2);
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
