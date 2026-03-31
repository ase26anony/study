/* test_fp_conditions.c
 * Designed to trigger x86 floating-point condition code generation
 * for UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, and LTGT
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
volatile double v_nan = 0.0/0.0;  /* NaN */
volatile double v_inf = 1.0/0.0;  /* Infinity */

/* Test UNORDERED and ORDERED conditions */
void test_unordered_ordered(void) {
    volatile double a = v_nan;
    volatile double b = v_normal1;
    
    /* UNORDERED: (a != a) when a is NaN */
    if (a != a) {  /* Should generate UNORDERED condition code */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* ORDERED: (b == b) when b is normal */
    if (b == b) {  /* Should generate ORDERED condition code */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Mixed: UNORDERED comparison */
    if (a < b) {  /* Should generate UNLT when a is NaN */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNEQ (unordered or equal) */
void test_uneq(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal1;  /* Equal values */
    volatile double c = v_nan;      /* NaN */
    
    /* UNEQ: a == b (ordered equal) */
    if (a == b) {  /* Should generate UNEQ condition code */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNEQ with NaN: c == a (unordered) */
    if (c == a) {  /* Should also generate UNEQ */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGE (unordered or greater or equal) */
void test_unge(void) {
    volatile double a = v_normal2;
    volatile double b = v_normal1;
    volatile double c = v_nan;
    
    /* UNGE: a >= b (ordered) */
    if (a >= b) {  /* Should generate UNGE condition code */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE with NaN: c >= a (unordered) */
    if (c >= a) {  /* Should also generate UNGE */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGT (unordered or greater than) */
void test_ungt(void) {
    volatile double a = v_normal2;
    volatile double b = v_normal1;
    volatile double c = v_nan;
    
    /* UNGT: a > b (ordered) */
    if (a > b) {  /* Should generate UNGT condition code */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGT with NaN: c > a (unordered) */
    if (c > a) {  /* Should also generate UNGT */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLE (unordered or less or equal) */
void test_unle(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal2;
    volatile double c = v_nan;
    
    /* UNLE: a <= b (ordered) */
    if (a <= b) {  /* Should generate UNLE condition code */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLE with NaN: c <= a (unordered) */
    if (c <= a) {  /* Should also generate UNLE */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLT (unordered or less than) */
void test_unlt(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal2;
    volatile double c = v_nan;
    
    /* UNLT: a < b (ordered) */
    if (a < b) {  /* Should generate UNLT condition code */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLT with NaN: c < a (unordered) */
    if (c < a) {  /* Should also generate UNLT */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test LTGT (less than or greater than - ordered and not equal) */
void test_ltgt(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal2;
    volatile double c = v_nan;
    
    /* LTGT: a != b (ordered not equal) */
    if (a != b) {  /* Should generate LTGT condition code */
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
    
    /* LTGT should be false for NaN comparisons */
    if (c != a) {  /* This might generate different code */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test mixed conditions in complex expressions */
void test_mixed_conditions(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal2;
    volatile double c = v_nan;
    volatile double d = v_inf;
    
    /* Complex expression triggering multiple condition codes */
    int r1 = (a < b) ? 1 : 0;      /* UNLT */
    int r2 = (c >= d) ? 1 : 0;     /* UNGE (unordered) */
    int r3 = (b == b) ? 1 : 0;     /* ORDERED */
    int r4 = (c != c) ? 1 : 0;     /* UNORDERED */
    int r5 = (a != b) ? 1 : 0;     /* LTGT */
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    results[idx++] = r5;
    
    /* Nested conditions */
    if ((a > b) || (c != c)) {     /* UNGT or UNORDERED */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Ternary with NaN operand */
    double result = (a <= c) ? 1.0 : 0.0;  /* UNLE with NaN */
    results[idx++] = (int)result;
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
    
    /* Compute checksum to ensure all code executed */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        checksum ^= (results[i] << (i % 16));
    }
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    printf("Tests executed: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}
