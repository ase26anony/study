/* test_fp_conditions.c
 * 
 * This program is designed to trigger specific floating-point condition code
 * generation in GCC's i386 backend, targeting the uncovered switch cases
 * for UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, and LTGT.
 *
 * Compilation recommendations:
 *   gcc -O2 -march=x86-64 -fno-fast-math -o test test_fp_conditions.c
 *   gcc -O3 -march=native -fno-fast-math -o test test_fp_conditions.c
 *   gcc -O2 -m32 -mfpmath=387 -fno-fast-math -o test test_fp_conditions.c
 */

#include <stdio.h>
#include <math.h>

/* Global results array to prevent dead code elimination */
volatile int results[32];
volatile int idx = 0;

/* Helper to produce NaN */
static double get_nan(void) {
    return 0.0 / 0.0;
}

/* Test UNORDERED and ORDERED conditions */
void test_unordered(void) {
    volatile double v = get_nan();
    volatile double n = 5.0;
    
    /* UNORDERED: v != v should be true when v is NaN */
    if (v != v) {
        results[idx++] = 1;  /* unordered true */
    } else {
        results[idx++] = 0;
    }
    
    /* ORDERED: n == n should be true for normal number */
    if (n == n) {
        results[idx++] = 1;  /* ordered true */
    } else {
        results[idx++] = 0;
    }
}

/* Test UNEQ and UNGE conditions */
void test_uneq_unge(void) {
    volatile double a = 2.0;
    volatile double b = 2.0;
    volatile double nan = get_nan();
    
    /* UNEQ: a == b (equal or unordered) */
    /* With -fno-fast-math, this can generate UNEQ */
    if (a == b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE: a >= nan (greater, equal, or unordered) */
    if (a >= nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGT, UNLE, and UNLT conditions */
void test_ungt_unle_unlt(void) {
    volatile double x = 3.0;
    volatile double y = 4.0;
    volatile double nan1 = get_nan();
    volatile double nan2 = get_nan();
    
    /* UNGT: x > nan1 (greater or unordered) */
    if (x > nan1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLE: nan1 <= y (less, equal, or unordered) */
    if (nan1 <= y) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLT: nan2 < x (less or unordered) */
    if (nan2 < x) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test LTGT condition (not equal and ordered) */
void test_ltgt(void) {
    volatile double p = 7.0;
    volatile double q = 8.0;
    volatile double r = 7.0;
    
    /* LTGT: p != q (not equal and ordered) */
    if (p != q) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Alternative using __builtin_islessgreater */
    if (__builtin_islessgreater(p, r)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Additional test mixing ordered and unordered comparisons */
void test_mixed(void) {
    volatile double normal = 10.0;
    volatile double nan = get_nan();
    
    /* Generate various condition codes through ternary operators */
    int r1 = (normal == normal) ? 1 : 0;      /* ORDERED */
    int r2 = (nan != nan) ? 1 : 0;            /* UNORDERED */
    int r3 = (normal >= nan) ? 1 : 0;         /* UNGE */
    int r4 = (normal <= nan) ? 1 : 0;         /* UNLE */
    int r5 = (normal > nan) ? 1 : 0;          /* UNGT */
    int r6 = (normal < nan) ? 1 : 0;          /* UNLT */
    int r7 = (normal == 10.0) ? 1 : 0;        /* UNEQ */
    int r8 = (normal != 11.0) ? 1 : 0;        /* LTGT */
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    results[idx++] = r5;
    results[idx++] = r6;
    results[idx++] = r7;
    results[idx++] = r8;
}

int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 32; i++) {
        results[i] = 0;
    }
    idx = 0;
    
    /* Execute all test functions */
    test_unordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed();
    
    /* Compute checksum to ensure all code executed */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    printf("Tests executed: %d\n", idx);
    
    return 0;
}
