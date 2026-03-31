/* test-float-conds.c
 * Designed to trigger specific floating-point condition code output
 * in GCC's i386 backend (i386.cc lines 13992-14017)
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global results array to prevent optimization */
volatile int results[32];
volatile int idx = 0;

/* External function to prevent inlining */
extern void use_result(int);

/* Test UNORDERED and ORDERED conditions */
void test_unordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0 / 0.0;  /* NaN */
    volatile double v3 = v2;         /* Another NaN */
    
    /* UNORDERED: (v2 != v2) should be true for NaN */
    if (v2 != v2) {
        results[idx++] = 1;  /* UNORDERED true */
    } else {
        results[idx++] = 0;
    }
    
    /* ORDERED: (v1 == v1) should be true for non-NaN */
    if (v1 == v1) {
        results[idx++] = 1;  /* ORDERED true */
    } else {
        results[idx++] = 0;
    }
    
    /* Mixed: ORDERED comparison with NaN operand */
    if (v1 == v3) {  /* Should be false (UNEQ false) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNEQ and UNGE conditions */
void test_uneq_unge(void) {
    volatile double a = 2.5;
    volatile double b = 2.5;
    volatile double nan = NAN;
    
    /* UNEQ: (a == b) where both are ordered */
    if (a == b) {
        results[idx++] = 1;  /* UNEQ true */
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE: (a >= nan) with NaN operand */
    if (a >= nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  /* UNGE false */
    }
    
    /* Another UNGE variant */
    if (nan >= a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  /* UNGE false */
    }
}

/* Test UNGT, UNLE, UNLT conditions */
void test_ungt_unle_unlt(void) {
    volatile double x = 3.0;
    volatile double y = 5.0;
    volatile double nan1 = NAN;
    volatile double nan2 = -NAN;
    
    /* UNGT: (x > nan1) with NaN operand */
    if (x > nan1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  /* UNGT false */
    }
    
    /* UNLE: (nan1 <= y) with NaN operand */
    if (nan1 <= y) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  /* UNLE false */
    }
    
    /* UNLT: (nan2 < x) with NaN operand */
    if (nan2 < x) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  /* UNLT false */
    }
    
    /* Additional UNLE/UNLT with both NaN */
    if (nan1 <= nan2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test LTGT (not equal and ordered) condition */
void test_ltgt(void) {
    volatile double p = 7.0;
    volatile double q = 8.0;
    volatile double nan = NAN;
    
    /* LTGT: (p != q) where both are ordered */
    if (p != q) {
        results[idx++] = 1;  /* LTGT true */
    } else {
        results[idx++] = 0;
    }
    
    /* Using __builtin_islessgreater for explicit LTGT */
    if (__builtin_islessgreater(p, q)) {
        results[idx++] = 1;  /* LTGT true */
    } else {
        results[idx++] = 0;
    }
    
    /* LTGT with NaN operand should be false */
    if (p != nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  /* Not LTGT (unordered) */
    }
}

/* Test mixed conditions in complex expressions */
void test_mixed(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan = NAN;
    
    /* Complex expression that might generate multiple condition codes */
    int r1 = (a < b) ? 1 : 0;           /* Possible UNLT */
    int r2 = (nan > a) ? 1 : 0;         /* Possible UNGT */
    int r3 = (b >= nan) ? 1 : 0;        /* Possible UNGE */
    int r4 = (a == a) ? 1 : 0;          /* ORDERED */
    int r5 = (nan != nan) ? 1 : 0;      /* UNORDERED */
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    results[idx++] = r5;
    
    /* Ternary with float comparison */
    double val = (a != b) ? 3.0 : 4.0;  /* Possible LTGT */
    results[idx++] = (int)val;
}

int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 32; i++) {
        results[i] = -1;
    }
    idx = 0;
    
    /* Run all test functions */
    test_unordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed();
    
    /* Compute checksum to ensure all code executed */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        use_result(results[i]);  /* Prevent dead code elimination */
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Number of tests: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy function definition to satisfy external declaration */
void use_result(int val) {
    /* This prevents the compiler from optimizing away the results */
    static volatile int sink;
    sink = val;
}
