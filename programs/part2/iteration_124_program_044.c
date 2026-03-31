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
    
    /* UNEQ: a == b (ordered equal) */
    if (a == b) {  /* Should generate UNEQ */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNEQ with NaN possibility */
    if (a == c) {  /* Should generate UNEQ (unordered equal) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGE (not less than) */
void test_unge(void) {
    volatile double a = v_normal1;
    volatile double b = v_nan;
    
    /* UNGE: a >= b where b is NaN */
    if (a >= b) {  /* Should generate UNGE (nlt) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE with normal values */
    if (v_normal3 >= v_normal1) {  /* Should generate UNGE */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGT (not less than or equal) */
void test_ungt(void) {
    volatile double a = v_normal2;
    volatile double b = v_nan;
    
    /* UNGT: a > b where b is NaN */
    if (a > b) {  /* Should generate UNGT (nle) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLE (unordered or less than or equal) */
void test_unle(void) {
    volatile double a = v_nan;
    volatile double b = v_normal1;
    
    /* UNLE: a <= b where a is NaN */
    if (a <= b) {  /* Should generate UNLE (ule) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLT (unordered or less than) */
void test_unlt(void) {
    volatile double a = v_nan;
    volatile double b = v_normal2;
    
    /* UNLT: a < b where a is NaN */
    if (a < b) {  /* Should generate UNLT (ult) */
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
    if (a != c) {  /* This is UNEQ, not LTGT */
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
    
    /* Complex expression triggering multiple condition codes */
    int r1 = (a < b) ? 1 : 0;      /* UNLT */
    int r2 = (b <= c) ? 1 : 0;     /* UNLE */
    int r3 = (a > b) ? 1 : 0;      /* UNGT */
    int r4 = (b >= a) ? 1 : 0;     /* UNGE */
    int r5 = (a == a) ? 1 : 0;     /* ORDERED */
    int r6 = (b != b) ? 1 : 0;     /* UNORDERED */
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    results[idx++] = r5;
    results[idx++] = r6;
}

/* Function to use results, preventing dead code elimination */
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
    test_mixed_conditions();
    
    /* Compute checksum to ensure all comparisons executed */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        use_result(results[i]);  /* Prevent optimization */
    }
    
    /* Print checksum to prevent entire program elimination */
    printf("Checksum: %d\n", checksum);
    printf("Total comparisons: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}
