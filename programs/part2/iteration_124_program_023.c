/* test_fp_conditions.c
 * Designed to generate x86 floating-point condition codes:
 * UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global results array to prevent optimization */
volatile int results[32];
volatile int idx = 0;

/* External function to prevent inlining */
extern void use_result(int);

/* Generate NaN values */
static double get_nan(void) {
    return 0.0 / 0.0;
}

static double get_nan2(void) {
    return nan("");
}

/* Test UNORDERED and ORDERED conditions */
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = get_nan();
    volatile double v3 = 3.0;
    
    /* UNORDERED: x != x when x is NaN */
    if (v2 != v2) {  /* Should generate UNORDERED condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* ORDERED: x == x when x is not NaN */
    if (v1 == v1) {  /* Should generate ORDERED condition */
        results[idx++] = 2;
    } else {
        results[idx++] = -2;
    }
    
    /* Mixed: ORDERED with potential NaN */
    if (v1 == v3) {  /* Both non-NaN, but compiler doesn't know */
        results[idx++] = 3;
    } else {
        results[idx++] = -3;
    }
}

/* Test UNEQ (unordered or equal) */
void test_uneq(void) {
    volatile double a = 5.0;
    volatile double b = get_nan2();
    volatile double c = 5.0;
    
    /* UNEQ: a == b where b could be NaN */
    if (a == b) {  /* Should generate UNEQ condition */
        results[idx++] = 10;
    } else {
        results[idx++] = -10;
    }
    
    /* Another UNEQ with both non-NaN */
    if (a == c) {
        results[idx++] = 11;
    } else {
        results[idx++] = -11;
    }
}

/* Test UNGE (unordered or greater or equal) */
void test_unge(void) {
    volatile double x = 7.0;
    volatile double y = get_nan();
    volatile double z = 5.0;
    
    /* UNGE: x >= y where y is NaN */
    if (x >= y) {  /* Should generate UNGE (nlt) */
        results[idx++] = 20;
    } else {
        results[idx++] = -20;
    }
    
    /* UNGE with normal values */
    if (x >= z) {
        results[idx++] = 21;
    } else {
        results[idx++] = -21;
    }
}

/* Test UNGT (unordered or greater than) */
void test_ungt(void) {
    volatile double p = 8.0;
    volatile double q = get_nan();
    
    /* UNGT: p > q where q is NaN */
    if (p > q) {  /* Should generate UNGT (nle) */
        results[idx++] = 30;
    } else {
        results[idx++] = -30;
    }
}

/* Test UNLE (unordered or less or equal) */
void test_unle(void) {
    volatile double m = get_nan();
    volatile double n = 6.0;
    
    /* UNLE: m <= n where m is NaN */
    if (m <= n) {  /* Should generate UNLE */
        results[idx++] = 40;
    } else {
        results[idx++] = -40;
    }
}

/* Test UNLT (unordered or less than) */
void test_unlt(void) {
    volatile double r = get_nan();
    volatile double s = 9.0;
    
    /* UNLT: r < s where r is NaN */
    if (r < s) {  /* Should generate UNLT (ult) */
        results[idx++] = 50;
    } else {
        results[idx++] = -50;
    }
}

/* Test LTGT (less than or greater than - ordered and not equal) */
void test_ltgt(void) {
    volatile double u = 10.0;
    volatile double v = 11.0;
    volatile double w = get_nan();
    
    /* LTGT: u != v (both ordered) */
    if (u != v) {  /* Should generate LTGT (une) */
        results[idx++] = 60;
    } else {
        results[idx++] = -60;
    }
    
    /* Using __builtin_islessgreater for explicit LTGT */
    if (__builtin_islessgreater(u, v)) {
        results[idx++] = 61;
    } else {
        results[idx++] = -61;
    }
    
    /* LTGT with NaN - should be false */
    if (__builtin_islessgreater(u, w)) {
        results[idx++] = 62;
    } else {
        results[idx++] = -62;
    }
}

/* Complex test mixing multiple conditions */
void test_mixed(void) {
    volatile double a = 1.5;
    volatile double b = get_nan();
    volatile double c = 2.5;
    volatile double d = 1.5;
    
    /* Chain of comparisons to force different condition codes */
    if (a == d) {           /* UNEQ */
        if (b >= a) {       /* UNGE */
            if (c > b) {    /* UNGT */
                if (b <= c) { /* UNLE */
                    if (a < b) { /* UNLT */
                        results[idx++] = 100;
                    }
                }
            }
        }
    }
    
    /* Ternary operator with FP comparison */
    int res = (a != c) ? 1 : 0;  /* LTGT */
    results[idx++] = res;
    
    /* Nested comparisons */
    if ((a == a) && (b != b)) {  /* ORDERED and UNORDERED */
        results[idx++] = 200;
    }
}

/* Prevent dead code elimination */
void use_result(int val) {
    /* Empty in source, but prevents optimization */
    (void)val;
}

int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 32; i++) {
        results[i] = 0;
    }
    idx = 0;
    
    /* Execute all test functions */
    test_unordered_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_mixed();
    
    /* Compute checksum to ensure all code executed */
    int checksum = 0;
    for (int i = 0; i < idx && i < 32; i++) {
        checksum += results[i];
        use_result(results[i]);
    }
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    printf("Tests executed: %d\n", idx);
    
    return 0;
}
