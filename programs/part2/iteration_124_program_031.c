/* test_fp_conditions.c
 * Designed to trigger x86 floating-point condition code generation
 * for UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT cases
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global results array to prevent dead code elimination */
volatile int results[32];
volatile int idx = 0;

/* External function to prevent optimization */
extern void use_result(int);

/* Helper to produce NaN */
static double make_nan(void) {
    return 0.0 / 0.0;
}

/* Test UNORDERED and ORDERED conditions */
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = make_nan();  /* NaN */
    volatile double v3 = 3.0;
    
    /* UNORDERED: v2 != v2 when v2 is NaN */
    if (v2 != v2) {  /* Should generate UNORDERED condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* ORDERED: v1 == v1 when v1 is normal */
    if (v1 == v1) {  /* Should generate ORDERED condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Mixed: ORDERED check with potential NaN */
    if (v1 == v3) {  /* Should generate UNEQ (unordered or equal) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNEQ, UNGE conditions */
void test_uneq_unge(void) {
    volatile double a = 2.0;
    volatile double b = make_nan();  /* NaN */
    volatile double c = 2.0;
    
    /* UNEQ: a == c (but could be unordered) */
    if (a == c) {  /* Should generate UNEQ */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE: a >= b where b is NaN */
    if (a >= b) {  /* Should generate UNGE (not less than) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Another UNGE variant */
    if (b >= a) {  /* NaN >= normal */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGT, UNLE, UNLT conditions */
void test_ungt_unle_unlt(void) {
    volatile double x = 5.0;
    volatile double y = make_nan();  /* NaN */
    volatile double z = 7.0;
    
    /* UNGT: x > y where y is NaN */
    if (x > y) {  /* Should generate UNGT (not less or equal) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLE: y <= z where y is NaN */
    if (y <= z) {  /* Should generate UNLE */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLT: y < x where y is NaN */
    if (y < x) {  /* Should generate UNLT */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Additional UNGT with both normal values */
    if (z > x) {  /* Should generate UNGT (ordered case) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test LTGT (not equal and ordered) condition */
void test_ltgt(void) {
    volatile double p = 10.0;
    volatile double q = 20.0;
    volatile double r = make_nan();  /* NaN */
    
    /* LTGT: p != q (both ordered) */
    if (p != q) {  /* Should generate LTGT (unordered or not equal) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* LTGT with builtin for clarity */
    if (__builtin_islessgreater(p, q)) {  /* Explicit LTGT */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* LTGT with NaN - should be false */
    if (p != r) {  /* NaN comparison */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test mixed conditions in complex expressions */
void test_mixed_conditions(void) {
    volatile double m = 15.0;
    volatile double n = make_nan();
    volatile double o = 15.0;
    
    /* Complex expression triggering multiple conditions */
    int res = (m == o) ? 1 : ((m >= n) ? 2 : 3);
    results[idx++] = res;
    
    /* Nested comparisons */
    if ((m != n) && (o > m)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Use ternary with floating comparison */
    double val = (m < n) ? 100.0 : 200.0;
    results[idx++] = (int)val;
}

/* Main function that runs all tests */
int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 32; i++) {
        results[i] = 0;
    }
    idx = 0;
    
    /* Run all test functions */
    test_unordered_ordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    /* Compute checksum to ensure all code executed */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        use_result(results[i]);  /* Prevent optimization */
    }
    
    /* Print checksum to ensure observable behavior */
    printf("Checksum: %d\n", checksum);
    printf("Tests executed: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy function to prevent optimization */
void use_result(int val) {
    /* Empty but prevents dead code elimination */
    volatile int dummy = val;
    (void)dummy;
}
