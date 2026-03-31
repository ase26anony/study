/* test_fp_conditions.c
 * 
 * This program is designed to trigger the specific uncovered lines in i386.cc
 * that handle x86 assembly condition code mnemonics for floating-point comparisons.
 * It uses volatile variables and NaN values to prevent constant folding and
 * ensures all comparison conditions (UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT)
 * are generated in the RTL representation.
 */

#include <stdio.h>
#include <math.h>

/* Global results array to store comparison outcomes and prevent dead code elimination */
int results[32];
int result_idx = 0;

/* Helper to record comparison results */
#define RECORD_RESULT(cond) do { \
    results[result_idx++] = (cond) ? 1 : 0; \
} while(0)

/* Test UNORDERED and ORDERED condition codes */
void test_unordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0 / 0.0;  /* NaN */
    
    /* UNORDERED: (v2 != v2) should be true when v2 is NaN */
    if (v2 != v2) {
        RECORD_RESULT(1);  /* UNORDERED true path */
    } else {
        RECORD_RESULT(0);  /* false path */
    }
    
    /* ORDERED: (v1 == v1) should be true for normal number */
    if (v1 == v1) {
        RECORD_RESULT(1);  /* ORDERED true path */
    } else {
        RECORD_RESULT(0);  /* false path */
    }
}

/* Test UNEQ and UNGE condition codes */
void test_uneq_unge(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 2.0;
    volatile double v3 = 0.0 / 0.0;  /* NaN */
    
    /* UNEQ: (v1 == v2) when unordered is possible */
    if (v1 == v2) {
        RECORD_RESULT(1);  /* UNEQ true path */
    } else {
        RECORD_RESULT(0);  /* false path */
    }
    
    /* UNGE: (v1 >= v3) where v3 is NaN */
    if (v1 >= v3) {
        RECORD_RESULT(1);  /* UNGE true path */
    } else {
        RECORD_RESULT(0);  /* false path */
    }
}

/* Test UNGT, UNLE, and UNLT condition codes */
void test_ungt_unle_unlt(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0 / 0.0;  /* NaN */
    volatile double v3 = 3.0;
    
    /* UNGT: (v1 > v2) where v2 is NaN */
    if (v1 > v2) {
        RECORD_RESULT(1);  /* UNGT true path */
    } else {
        RECORD_RESULT(0);  /* false path */
    }
    
    /* UNLE: (v2 <= v3) where v2 is NaN */
    if (v2 <= v3) {
        RECORD_RESULT(1);  /* UNLE true path */
    } else {
        RECORD_RESULT(0);  /* false path */
    }
    
    /* UNLT: (v2 < v1) where v2 is NaN */
    if (v2 < v1) {
        RECORD_RESULT(1);  /* UNLT true path */
    } else {
        RECORD_RESULT(0);  /* false path */
    }
}

/* Test LTGT condition code */
void test_ltgt(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 2.0;
    volatile double v3 = 1.0;
    
    /* LTGT: "not equal and ordered" - use != with ordered operands */
    if (v1 != v2) {
        RECORD_RESULT(1);  /* LTGT true path (v1 != v2) */
    } else {
        RECORD_RESULT(0);  /* false path */
    }
    
    /* Alternative: use __builtin_islessgreater */
    if (__builtin_islessgreater(v1, v3)) {
        RECORD_RESULT(1);  /* false path (v1 == v3) */
    } else {
        RECORD_RESULT(0);  /* LTGT false path */
    }
}

/* Additional tests to ensure all switch cases are hit */
void test_mixed_conditions(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 2.0;
    volatile double nan1 = 0.0 / 0.0;
    volatile double nan2 = -0.0 / 0.0;
    
    /* Mix of conditions in ternary operators */
    int r1 = (v1 == v1) ? 1 : 0;      /* ORDERED */
    int r2 = (nan1 != nan1) ? 1 : 0;  /* UNORDERED */
    int r3 = (v1 == v2) ? 1 : 0;      /* UNEQ */
    int r4 = (v1 >= nan1) ? 1 : 0;    /* UNGE */
    int r5 = (v1 > nan2) ? 1 : 0;     /* UNGT */
    int r6 = (nan1 <= v2) ? 1 : 0;    /* UNLE */
    int r7 = (nan2 < v1) ? 1 : 0;     /* UNLT */
    int r8 = (v1 != v2) ? 1 : 0;      /* LTGT */
    
    results[result_idx++] = r1;
    results[result_idx++] = r2;
    results[result_idx++] = r3;
    results[result_idx++] = r4;
    results[result_idx++] = r5;
    results[result_idx++] = r6;
    results[result_idx++] = r7;
    results[result_idx++] = r8;
}

int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 32; i++) {
        results[i] = 0;
    }
    result_idx = 0;
    
    /* Execute all test functions */
    test_unordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    /* Compute checksum to ensure all comparisons executed */
    int checksum = 0;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
    }
    
    /* Print checksum to prevent optimization and verify execution */
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
