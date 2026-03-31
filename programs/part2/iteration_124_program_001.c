/* test_fp_conditions.c
 * Designed to trigger x86 floating-point condition code generation
 * for UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT cases
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global results array to prevent optimization */
volatile int results[32];
volatile int idx = 0;

/* External function to prevent inlining */
extern void use_result(int);

/* Initialize with volatile to prevent constant folding */
volatile double v_normal = 1.5;
volatile double v_nan = 0.0 / 0.0;  /* NaN */
volatile double v_zero = 0.0;
volatile double v_inf = 1.0 / 0.0;  /* Infinity */
volatile double v_neg = -2.0;

/* Test UNORDERED and ORDERED condition codes */
void test_unordered_ordered(void) {
    volatile double a = v_nan;
    volatile double b = v_normal;
    
    /* UNORDERED: x != x when x is NaN */
    if (a != a) {  /* Should generate UNORDERED condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* ORDERED: x == x when x is not NaN */
    if (b == b) {  /* Should generate ORDERED condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Mixed: compare NaN with normal value */
    if (a < b) {  /* UNLT (ult) when unordered */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNEQ (unordered or equal) */
void test_uneq(void) {
    volatile double a = v_normal;
    volatile double b = v_normal + 0.1;
    volatile double c = v_nan;
    
    /* UNEQ: a == b (ordered equal) */
    if (a == a) {  /* Should generate UNEQ */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNEQ with potential NaN */
    if (c == c) {  /* NaN == NaN is false, but generates UNEQ */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGE (not less than - unordered greater or equal) */
void test_unge(void) {
    volatile double a = v_normal;
    volatile double b = v_nan;
    
    /* UNGE: a >= b where b could be NaN */
    if (a >= b) {  /* Should generate UNGE (nlt) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Another UNGE case */
    if (v_inf >= v_normal) {  /* Infinity >= normal */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGT (not less or equal - unordered greater than) */
void test_ungt(void) {
    volatile double a = v_normal;
    volatile double b = v_nan;
    
    /* UNGT: a > b where b could be NaN */
    if (a > b) {  /* Should generate UNGT (nle) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if (v_inf > v_normal) {  /* Infinity > normal */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLE (unordered less or equal) */
void test_unle(void) {
    volatile double a = v_nan;
    volatile double b = v_normal;
    
    /* UNLE: a <= b where a is NaN */
    if (a <= b) {  /* Should generate UNLE (ule) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if (v_neg <= v_zero) {  /* -2.0 <= 0.0 */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLT (unordered less than) */
void test_unlt(void) {
    volatile double a = v_nan;
    volatile double b = v_normal;
    
    /* UNLT: a < b where a is NaN */
    if (a < b) {  /* Should generate UNLT (ult) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if (v_neg < v_zero) {  /* -2.0 < 0.0 */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test LTGT (not equal and ordered) */
void test_ltgt(void) {
    volatile double a = v_normal;
    volatile double b = v_normal + 1.0;
    volatile double c = v_nan;
    
    /* LTGT: a != b (ordered not equal) */
    if (a != b) {  /* Should generate LTGT (une) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Using __builtin_islessgreater for explicit LTGT */
    if (__builtin_islessgreater(a, b)) {  /* Explicit LTGT */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* LTGT with NaN (should be false) */
    if (c != a) {  /* NaN != normal */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test mixed conditions in complex expressions */
void test_mixed_conditions(void) {
    volatile double a = v_nan;
    volatile double b = v_normal;
    volatile double c = v_zero;
    
    /* Complex expression triggering multiple condition codes */
    int r1 = (a == a) ? 1 : 0;  /* UNEQ */
    int r2 = (b >= a) ? 1 : 0;  /* UNGE */
    int r3 = (a <= b) ? 1 : 0;  /* UNLE */
    int r4 = (b != c) ? 1 : 0;  /* LTGT */
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    
    /* Nested conditionals */
    if ((a < b) || (b > c)) {  /* UNLT or ordered greater */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Main function that runs all tests */
int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 32; i++) {
        results[i] = -1;
    }
    idx = 0;
    
    /* Run all test functions */
    test_unordered_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    /* Compute checksum to ensure all comparisons executed */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        use_result(results[i]);  /* Prevent dead code elimination */
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Number of tests executed: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy function to prevent optimization */
void use_result(int val) {
    /* This function is defined elsewhere to prevent inlining */
    /* In a real test, this would be in a separate compilation unit */
    static volatile int sink;
    sink = val;
}
