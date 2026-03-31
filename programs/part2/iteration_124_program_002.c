/* test_fp_conditions.c
 * 
 * This program is designed to trigger the uncovered condition code
 * output logic in i386.cc lines 13992-14017.
 * It uses volatile doubles and NaN values to force generation of
 * specific floating-point condition codes (UNORDERED, ORDERED, UNEQ,
 * UNGE, UNGT, UNLE, UNLT, LTGT) in x86 assembly output.
 */

#include <stdio.h>
#include <math.h>

/* Global results array to prevent dead code elimination */
volatile int results[32];
volatile int idx = 0;

/* External function to prevent inlining */
extern void use_result(int);

/* Helper to produce NaN */
static double make_nan(void) {
    volatile double zero = 0.0;
    return zero / zero;  /* Produces NaN */
}

/* Test UNORDERED and ORDERED conditions */
void test_unordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = make_nan();  /* NaN */
    volatile double v3 = 3.0;
    
    /* UNORDERED: v2 != v2 when v2 is NaN */
    if (v2 != v2) {
        results[idx++] = 1;  /* true: unordered */
    } else {
        results[idx++] = 0;
    }
    
    /* ORDERED: v1 == v1 (always true for non-NaN) */
    if (v1 == v1) {
        results[idx++] = 1;  /* true: ordered */
    } else {
        results[idx++] = 0;
    }
    
    /* Mixed: compare NaN with normal number (unordered) */
    if (v2 > v1) {  /* UNGT: not less or equal */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNEQ and UNGE conditions */
void test_uneq_unge(void) {
    volatile double v1 = 1.0;
    volatile double v2 = make_nan();
    volatile double v3 = 3.0;
    volatile double v4 = 1.0;  /* Same value as v1 */
    
    /* UNEQ: v1 == v4 (equal, but unordered possible) */
    if (v1 == v4) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE: v1 >= v2 where v2 is NaN */
    if (v1 >= v2) {  /* Should generate 'nlt' (not less than) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Additional UNGE with both non-NaN */
    if (v3 >= v1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGT, UNLE, UNLT conditions */
void test_ungt_unle_unlt(void) {
    volatile double v1 = 1.0;
    volatile double v2 = make_nan();
    volatile double v3 = 3.0;
    
    /* UNGT: v1 > v2 where v2 is NaN (not less or equal) */
    if (v1 > v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLE: v2 <= v3 where v2 is NaN */
    if (v2 <= v3) {  /* Should generate 'ule' (unordered or less or equal) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLT: v2 < v1 where v2 is NaN */
    if (v2 < v1) {  /* Should generate 'ult' (unordered or less than) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test LTGT (not equal and ordered) condition */
void test_ltgt(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 3.0;
    volatile double v3 = make_nan();
    
    /* LTGT: v1 != v2 (both ordered, not equal) */
    if (v1 != v2) {  /* Should generate 'une' (unordered or not equal) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Using __builtin_islessgreater for explicit LTGT */
    if (__builtin_islessgreater(v1, v2)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* LTGT with NaN (should be false) */
    if (__builtin_islessgreater(v1, v3)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Additional tests to cover more cases */
void test_mixed_conditions(void) {
    volatile double v1 = 1.0;
    volatile double v2 = make_nan();
    volatile double v3 = 3.0;
    volatile double v4 = 1.0;
    
    /* Various comparisons mixing NaN and normal values */
    if (v2 == v2) results[idx++] = 1; else results[idx++] = 0;  /* ORDERED */
    if (v1 != v2) results[idx++] = 1; else results[idx++] = 0;  /* UNEQ or NEQ */
    if (v2 >= v2) results[idx++] = 1; else results[idx++] = 0;  /* UNORDERED */
    if (v1 <= v3) results[idx++] = 1; else results[idx++] = 0;  /* UNLE or LE */
    if (v3 > v1)  results[idx++] = 1; else results[idx++] = 0;  /* UNGT or GT */
    if (v1 < v3)  results[idx++] = 1; else results[idx++] = 0;  /* UNLT or LT */
}

int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 32; i++) {
        results[i] = -1;
    }
    idx = 0;
    
    /* Execute all test functions */
    test_unordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    /* Compute checksum to ensure all comparisons were executed */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum = (checksum * 31 + results[i]) & 0xFF;
        use_result(results[i]);  /* Prevent optimization */
    }
    
    /* Print checksum to ensure observable behavior */
    printf("Checksum: %d\n", checksum);
    printf("Total comparisons: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy function definition to prevent optimization */
void use_result(int val) {
    /* This function is just to prevent dead code elimination */
    static volatile int sink;
    sink = val;
}
