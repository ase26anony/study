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

/* Test functions for specific condition codes */

/* UNORDERED and ORDERED conditions */
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0 / 0.0;  /* NaN */
    volatile double v3 = v2;         /* Another NaN */
    
    /* UNORDERED: x != x when x is NaN */
    if (v2 != v2) {
        results[idx++] = 1;  /* UNORDERED true */
    } else {
        results[idx++] = 0;
    }
    
    /* ORDERED: x == x when x is not NaN */
    if (v1 == v1) {
        results[idx++] = 1;  /* ORDERED true */
    } else {
        results[idx++] = 0;
    }
    
    /* Mixed: ORDERED with potential NaN */
    if (v1 == v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  /* Likely false due to NaN */
    }
}

/* UNEQ and UNGE conditions */
void test_uneq_unge(void) {
    volatile double a = 2.5;
    volatile double b = 2.5;
    volatile double nan = NAN;
    
    /* UNEQ: a == b (unordered equal) */
    if (a == b) {
        results[idx++] = 1;  /* UNEQ true */
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE: a >= nan (unordered not less than) */
    if (a >= nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  /* UNGE false when unordered */
    }
    
    /* UNGE with normal values */
    if (a >= 2.0) {
        results[idx++] = 1;  /* UNGE true */
    } else {
        results[idx++] = 0;
    }
}

/* UNGT, UNLE, UNLT conditions */
void test_ungt_unle_unlt(void) {
    volatile double x = 3.0;
    volatile double y = 4.0;
    volatile double nan1 = NAN;
    volatile double nan2 = -NAN;
    
    /* UNGT: x > nan1 (unordered not less or equal) */
    if (x > nan1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  /* UNGT false when unordered */
    }
    
    /* UNLE: nan1 <= y (unordered less or equal) */
    if (nan1 <= y) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  /* UNLE false when unordered */
    }
    
    /* UNLT: nan2 < x (unordered less than) */
    if (nan2 < x) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  /* UNLT false when unordered */
    }
    
    /* Normal comparisons that may still generate condition codes */
    if (x > y) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1;  /* Normal ordered comparison */
    }
}

/* LTGT condition (not equal and ordered) */
void test_ltgt(void) {
    volatile double p = 5.0;
    volatile double q = 6.0;
    volatile double r = 5.0;
    volatile double nan = NAN;
    
    /* LTGT: p != q (ordered not equal) */
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
    
    /* This should not be LTGT (equal values) */
    if (p != r) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  /* LTGT false */
    }
    
    /* Comparison with NaN - not LTGT */
    if (p != nan) {
        results[idx++] = 1;  /* Always true, but not ordered */
    } else {
        results[idx++] = 0;
    }
}

/* Complex function mixing multiple condition types */
void test_mixed_conditions(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan = NAN;
    volatile double inf = 1.0 / 0.0;
    
    /* Ternary operators generate condition codes */
    int r1 = (a < b) ? 1 : 0;
    int r2 = (a >= nan) ? 1 : 0;
    int r3 = (nan == nan) ? 1 : 0;
    int r4 = (inf > a) ? 1 : 0;
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    
    /* Nested conditionals */
    if (a != a) {  /* UNORDERED check */
        if (b == b) {  /* ORDERED check */
            results[idx++] = 1;
        }
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
    
    /* Print checksum to ensure execution */
    printf("Checksum: %d\n", checksum);
    printf("Number of tests: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy function definition to satisfy external reference */
void use_result(int val) {
    /* This could be implemented to actually use the result */
    volatile static int sink = 0;
    sink += val;
}
