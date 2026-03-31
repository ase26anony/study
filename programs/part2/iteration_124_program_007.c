/* test_fp_conditions.c
 * Designed to trigger x86 floating-point condition code generation
 * for UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, and LTGT cases.
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
volatile double v_normal1 = 1.0;
volatile double v_normal2 = 3.14159;
volatile double v_normal3 = -2.5;
volatile double v_nan = 0.0 / 0.0;  /* NaN */
volatile double v_inf = 1.0 / 0.0;  /* Infinity */

/* Test UNORDERED and ORDERED condition codes */
void test_unordered_ordered(void) {
    volatile double a = v_nan;
    volatile double b = v_normal1;
    
    /* UNORDERED: a != a when a is NaN */
    if (a != a) {  /* Should generate UNORDERED condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* ORDERED: b == b when b is normal */
    if (b == b) {  /* Should generate ORDERED condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Mixed: UNORDERED comparison */
    if (a < b) {  /* Should generate UNLT (ult) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNEQ (unordered or equal) */
void test_uneq(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal1;
    volatile double c = v_nan;
    
    /* UNEQ: a == b (both normal, equal) */
    if (a == b) {  /* Should generate UNEQ */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNEQ with NaN possibility */
    if (a == c) {  /* Should generate UNEQ */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGE (not less than) and UNGT (not less than or equal) */
void test_unge_ungt(void) {
    volatile double a = v_normal1;
    volatile double b = v_nan;
    volatile double c = v_normal2;
    
    /* UNGE: a >= b where b is NaN */
    if (a >= b) {  /* Should generate UNGE (nlt) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGT: a > b where b is NaN */
    if (a > b) {  /* Should generate UNGT (nle) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE with normal values */
    if (c >= a) {  /* Should generate UNGE */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLE (unordered or less than or equal) and UNLT (unordered or less than) */
void test_unle_unlt(void) {
    volatile double a = v_nan;
    volatile double b = v_normal1;
    volatile double c = v_normal3;
    
    /* UNLE: a <= b where a is NaN */
    if (a <= b) {  /* Should generate UNLE (ule) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLT: a < b where a is NaN */
    if (a < b) {  /* Should generate UNLT (ult) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLE with normal values */
    if (c <= b) {  /* Should generate UNLE */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test LTGT (less than or greater than - ordered not equal) */
void test_ltgt(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal2;
    volatile double c = v_nan;
    
    /* LTGT: a != b (both normal, not equal) */
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
    if (a != c) {  /* Should generate LTGT */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test mixed conditions in complex expressions */
void test_mixed_conditions(void) {
    volatile double a = v_normal1;
    volatile double b = v_nan;
    volatile double c = v_normal2;
    
    /* Ternary operator with floating comparisons */
    int r1 = (a >= b) ? 1 : 0;  /* UNGE */
    int r2 = (b <= c) ? 2 : 0;  /* UNLE */
    int r3 = (a != c) ? 3 : 0;  /* LTGT */
    
    results[idx++] = r1 + r2 + r3;
    
    /* Nested comparisons */
    if ((a < b) || (c > a)) {  /* UNLT or UNGT */
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
    test_unordered_ordered();   /* Covers UNORDERED, ORDERED, UNLT */
    test_uneq();                /* Covers UNEQ */
    test_unge_ungt();           /* Covers UNGE, UNGT */
    test_unle_unlt();           /* Covers UNLE, UNLT */
    test_ltgt();                /* Covers LTGT */
    test_mixed_conditions();    /* Covers mixed cases */
    
    /* Compute checksum to ensure all code executed */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        use_result(results[i]);  /* Prevent optimization */
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Number of tests executed: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy function to prevent optimization */
void use_result(int val) {
    /* Empty but prevents dead code elimination */
    volatile int dummy = val;
    (void)dummy;
}
