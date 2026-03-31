/* test-float-conditions.c
 * Designed to trigger x86 floating-point condition code generation
 * for uncovered lines in i386.cc (lines 13992-14017)
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global results array to prevent optimization */
volatile int results[32];
volatile int result_idx = 0;

/* External function to prevent inlining */
extern void use_result(int);

/* Initialize with NaN using standard method */
double get_nan() {
    return 0.0 / 0.0;
}

/* Test UNORDERED (x != x when x is NaN) and ORDERED (x == x) */
void test_unordered_ordered(void) {
    volatile double v_normal = 1.5;
    volatile double v_nan = get_nan();
    volatile double v_self = v_nan;
    
    /* UNORDERED: v_nan != v_nan should be true */
    if (v_self != v_self) {
        results[result_idx++] = 1;  /* UNORDERED true */
    } else {
        results[result_idx++] = 0;
    }
    
    /* ORDERED: v_normal == v_normal should be true */
    if (v_normal == v_normal) {
        results[result_idx++] = 1;  /* ORDERED true */
    } else {
        results[result_idx++] = 0;
    }
    
    /* Mixed: ORDERED check on NaN should be false */
    if (v_nan == v_nan) {
        results[result_idx++] = 0;
    } else {
        results[result_idx++] = 1;  /* UNORDERED true */
    }
}

/* Test UNEQ (== with possible NaN) and UNGE (>= with possible NaN) */
void test_uneq_unge(void) {
    volatile double a = 2.0;
    volatile double b = 2.0;
    volatile double c = get_nan();
    volatile double d = 3.0;
    
    /* UNEQ: a == b (both ordered, equal) */
    if (a == b) {
        results[result_idx++] = 1;  /* UNEQ true */
    } else {
        results[result_idx++] = 0;
    }
    
    /* UNEQ with NaN operand: a == c */
    if (a == c) {
        results[result_idx++] = 0;
    } else {
        results[result_idx++] = 1;  /* UNEQ false */
    }
    
    /* UNGE: a >= c (c is NaN) */
    if (a >= c) {
        results[result_idx++] = 0;
    } else {
        results[result_idx++] = 1;  /* UNGE false (unordered) */
    }
    
    /* UNGE: d >= a (both ordered) */
    if (d >= a) {
        results[result_idx++] = 1;  /* UNGE true */
    } else {
        results[result_idx++] = 0;
    }
}

/* Test UNGT (> with possible NaN), UNLE (<= with possible NaN), UNLT (< with possible NaN) */
void test_ungt_unle_unlt(void) {
    volatile double x = 5.0;
    volatile double y = get_nan();
    volatile double z = 3.0;
    
    /* UNGT: x > y (y is NaN) */
    if (x > y) {
        results[result_idx++] = 0;
    } else {
        results[result_idx++] = 1;  /* UNGT false (unordered) */
    }
    
    /* UNGT: x > z (both ordered) */
    if (x > z) {
        results[result_idx++] = 1;  /* UNGT true */
    } else {
        results[result_idx++] = 0;
    }
    
    /* UNLE: y <= x (y is NaN) */
    if (y <= x) {
        results[result_idx++] = 0;
    } else {
        results[result_idx++] = 1;  /* UNLE false (unordered) */
    }
    
    /* UNLE: z <= x (both ordered) */
    if (z <= x) {
        results[result_idx++] = 1;  /* UNLE true */
    } else {
        results[result_idx++] = 0;
    }
    
    /* UNLT: y < x (y is NaN) */
    if (y < x) {
        results[result_idx++] = 0;
    } else {
        results[result_idx++] = 1;  /* UNLT false (unordered) */
    }
    
    /* UNLT: z < x (both ordered) */
    if (z < x) {
        results[result_idx++] = 1;  /* UNLT true */
    } else {
        results[result_idx++] = 0;
    }
}

/* Test LTGT (not equal and ordered) */
void test_ltgt(void) {
    volatile double p = 7.0;
    volatile double q = 8.0;
    volatile double r = get_nan();
    
    /* LTGT: p != q (both ordered, not equal) */
    if (p != q) {
        results[result_idx++] = 1;  /* LTGT true */
    } else {
        results[result_idx++] = 0;
    }
    
    /* LTGT: p != p (equal, should be false) */
    if (p != p) {
        results[result_idx++] = 0;
    } else {
        results[result_idx++] = 1;  /* LTGT false */
    }
    
    /* LTGT with NaN: p != r */
    if (p != r) {
        results[result_idx++] = 1;  /* Always true when r is NaN */
    } else {
        results[result_idx++] = 0;
    }
    
    /* Using __builtin_islessgreater for explicit LTGT */
    if (__builtin_islessgreater(p, q)) {
        results[result_idx++] = 1;  /* LTGT true */
    } else {
        results[result_idx++] = 0;
    }
    
    if (__builtin_islessgreater(p, r)) {
        results[result_idx++] = 0;  /* false when NaN involved */
    } else {
        results[result_idx++] = 1;
    }
}

/* Additional test with ternary operators */
void test_ternary(void) {
    volatile double m = 10.0;
    volatile double n = get_nan();
    
    /* Ternary with UNORDERED comparison */
    int res1 = (m != m) ? 100 : 200;  /* Should be 200 */
    int res2 = (n != n) ? 300 : 400;  /* Should be 300 (UNORDERED true) */
    
    results[result_idx++] = res1 / 100 - 1;  /* 1 */
    results[result_idx++] = res2 / 100 - 3;  /* 0 */
    
    /* Ternary with ORDERED comparison */
    int res3 = (m == m) ? 500 : 600;  /* Should be 500 */
    int res4 = (n == n) ? 700 : 800;  /* Should be 800 (ORDERED false) */
    
    results[result_idx++] = res3 / 100 - 4;  /* 1 */
    results[result_idx++] = res4 / 100 - 7;  /* 1 */
}

int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 32; i++) {
        results[i] = 0;
    }
    result_idx = 0;
    
    /* Execute all test functions */
    test_unordered_ordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_ternary();
    
    /* Compute checksum to ensure execution */
    int checksum = 0;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
        checksum *= 31;
    }
    
    /* Print checksum to prevent optimization */
    printf("Result checksum: %d\n", checksum);
    printf("Tests executed: %d\n", result_idx);
    
    return 0;
}
