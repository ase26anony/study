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
volatile double v_nan = 0.0/0.0;  /* NaN */
volatile double v_inf = 1.0/0.0;  /* Infinity */

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
    
    /* Mixed: ORDERED/UNORDERED depending on values */
    volatile double c = v_nan;
    volatile double d = v_normal2;
    
    if (c == d) {  /* UNEQ when unordered */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNEQ, UNGE, UNGT conditions */
void test_uneq_unge_ungt(void) {
    volatile double a = v_normal1;
    volatile double b = v_nan;
    volatile double c = v_normal2;
    
    /* UNEQ: a == c (ordered equal) */
    if (a == a) {  /* Same value comparison for UNEQ */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE: a >= b (unordered or greater) */
    if (a >= b) {  /* Should generate UNGE (nlt) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGT: a > b (unordered or greater) */
    if (a > b) {  /* Should generate UNGT (nle) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Additional UNGE with normal values */
    if (c >= a) {  /* Ordered greater or equal */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLE, UNLT conditions */
void test_unle_unlt(void) {
    volatile double a = v_nan;
    volatile double b = v_normal1;
    volatile double c = v_normal3;
    
    /* UNLE: a <= b (unordered or less/equal) */
    if (a <= b) {  /* Should generate UNLE (ule) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLT: a < b (unordered or less) */
    if (a < b) {  /* Should generate UNLT (ult) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Additional comparisons with mixing */
    if (b <= c) {  /* Ordered less or equal */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if (b < c) {   /* Ordered less */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test LTGT (not equal and ordered) condition */
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
    if (__builtin_islessgreater(a, b)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Mixed with NaN to ensure different paths */
    if (a != c) {  /* Unordered not equal (different from LTGT) */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Complex function with multiple condition patterns */
void test_mixed_conditions(void) {
    volatile double x = v_nan;
    volatile double y = v_normal1;
    volatile double z = v_normal2;
    
    /* Generate various condition codes in one function */
    int r = 0;
    
    /* Chain of comparisons to force different condition codes */
    if (x == x) r |= 1;      /* ORDERED */
    if (x != x) r |= 2;      /* UNORDERED */
    if (y == z) r |= 4;      /* UNEQ */
    if (y >= x) r |= 8;      /* UNGE */
    if (y > x)  r |= 16;     /* UNGT */
    if (x <= y) r |= 32;     /* UNLE */
    if (x < y)  r |= 64;     /* UNLT */
    if (y != z) r |= 128;    /* LTGT */
    
    results[idx++] = r;
    
    /* Use ternary operator with FP comparisons */
    double t1 = (y >= x) ? 1.0 : 0.0;  /* UNGE */
    double t2 = (x < y)  ? 1.0 : 0.0;  /* UNLT */
    double t3 = (y != z) ? 1.0 : 0.0;  /* LTGT */
    
    results[idx++] = (int)(t1 + t2 + t3);
}

/* Function that uses results to prevent dead code elimination */
void use_result(int val) {
    /* External linkage prevents optimization */
    results[idx % 32] ^= val;
}

int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 32; i++) {
        results[i] = 0;
    }
    idx = 0;
    
    /* Execute all test functions */
    test_unordered_ordered();
    test_uneq_unge_ungt();
    test_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    /* Force evaluation and use of all condition codes */
    volatile double a = v_normal1;
    volatile double b = v_nan;
    
    /* Additional inline tests to ensure coverage */
    if (!(a == a)) use_result(1);   /* ORDERED */
    if (b != b)    use_result(2);   /* UNORDERED */
    if (a == a)    use_result(4);   /* UNEQ (ordered equal) */
    if (a >= b)    use_result(8);   /* UNGE */
    if (a > b)     use_result(16);  /* UNGT */
    if (b <= a)    use_result(32);  /* UNLE */
    if (b < a)     use_result(64);  /* UNLT */
    if (a != v_normal2) use_result(128); /* LTGT */
    
    /* Compute checksum to ensure all code executed */
    int checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += results[i];
        checksum ^= (results[i] << (i % 16));
    }
    
    printf("Result checksum: %d\n", checksum);
    printf("Tests completed. idx = %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}
