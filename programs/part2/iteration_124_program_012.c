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
volatile double v_normal1 = 1.0;
volatile double v_normal2 = 3.14159;
volatile double v_normal3 = 2.71828;
volatile double v_nan = 0.0 / 0.0;  /* NaN */
volatile double v_inf = 1.0 / 0.0;  /* Infinity */

/* Test UNORDERED and ORDERED conditions */
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
    if (a < b) {  /* UNLT when a is NaN */
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
    
    /* UNEQ: a == b (ordered equal) */
    if (a == b) {  /* Should generate UNEQ condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNEQ with NaN possibility */
    if (a == c) {  /* UNEQ when c is NaN */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGE (not less than) */
void test_unge(void) {
    volatile double a = v_normal1;
    volatile double b = v_nan;
    volatile double c = v_normal2;
    
    /* UNGE: a >= b where b is NaN */
    if (a >= b) {  /* Should generate UNGE condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE: c >= a (normal comparison) */
    if (c >= a) {  /* Should generate UNGE condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGT (not less than or equal) */
void test_ungt(void) {
    volatile double a = v_normal2;
    volatile double b = v_nan;
    volatile double c = v_normal1;
    
    /* UNGT: a > b where b is NaN */
    if (a > b) {  /* Should generate UNGT condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGT: a > c (normal comparison) */
    if (a > c) {  /* Should generate UNGT condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLE (unordered or less than or equal) */
void test_unle(void) {
    volatile double a = v_nan;
    volatile double b = v_normal1;
    volatile double c = v_normal3;
    
    /* UNLE: a <= b where a is NaN */
    if (a <= b) {  /* Should generate UNLE condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLE: c <= v_normal2 */
    if (c <= v_normal2) {  /* Should generate UNLE condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLT (unordered or less than) */
void test_unlt(void) {
    volatile double a = v_nan;
    volatile double b = v_normal1;
    volatile double c = v_normal1;
    volatile double d = v_normal2;
    
    /* UNLT: a < b where a is NaN */
    if (a < b) {  /* Should generate UNLT condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLT: c < d (normal comparison) */
    if (c < d) {  /* Should generate UNLT condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test LTGT (less than or greater than - ordered and not equal) */
void test_ltgt(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal2;
    volatile double c = v_normal1;
    
    /* LTGT: a != b (ordered not equal) */
    if (a != b) {  /* Should generate LTGT condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* LTGT using builtin for explicit ordered comparison */
    if (__builtin_islessgreater(a, b)) {  /* Explicit LTGT */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* LTGT: a != c (equal case) */
    if (a != c) {  /* Should generate LTGT condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test mixed conditions in complex expressions */
void test_mixed_conditions(void) {
    volatile double a = v_nan;
    volatile double b = v_normal1;
    volatile double c = v_normal2;
    
    /* Use ternary operator to force condition code generation */
    int r1 = (a >= b) ? 1 : 0;  /* UNGE */
    int r2 = (b <= c) ? 2 : 0;  /* UNLE */
    int r3 = (a < c) ? 3 : 0;   /* UNLT */
    int r4 = (b > a) ? 4 : 0;   /* UNGT */
    
    results[idx++] = r1 + r2 + r3 + r4;
    
    /* Complex if-else chain */
    if (a == a) {  /* ORDERED for NaN - false */
        results[idx++] = 0;
    } else if (b != b) {  /* UNORDERED for normal - false */
        results[idx++] = 0;
    } else if (c >= a) {  /* UNGE */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Main function that runs all tests */
int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 32; i++) {
        results[i] = 0;
    }
    idx = 0;
    
    /* Run all test functions */
    test_unordered_ordered();    /* Covers UNORDERED, ORDERED, UNLT */
    test_uneq();                 /* Covers UNEQ */
    test_unge();                 /* Covers UNGE */
    test_ungt();                 /* Covers UNGT */
    test_unle();                 /* Covers UNLE */
    test_unlt();                 /* Covers UNLT */
    test_ltgt();                 /* Covers LTGT */
    test_mixed_conditions();     /* Covers multiple conditions */
    
    /* Compute checksum to ensure all comparisons executed */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        checksum ^= (results[i] << (i % 16));
    }
    
    /* Print checksum to prevent optimization */
    printf("Result checksum: %d\n", checksum);
    printf("Number of comparisons: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}
