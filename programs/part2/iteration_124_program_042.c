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
    if (a < b) {  /* Should generate UNORDERED branch */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNEQ (Unordered or Equal) condition */
void test_uneq(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal2;
    volatile double c = v_nan;
    
    /* UNEQ: a == b (ordered equal) */
    if (a == a) {  /* Equal comparison that could be unordered */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNEQ with NaN operand */
    if (a == c) {  /* Should handle unordered case */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGE (Unordered or Greater or Equal) condition */
void test_unge(void) {
    volatile double a = v_normal1;
    volatile double b = v_nan;
    volatile double c = v_normal2;
    
    /* UNGE: a >= c (ordered) */
    if (a >= c) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE with NaN operand */
    if (a >= b) {  /* Should generate UNGE condition */
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

/* Test UNGT (Unordered or Greater Than) condition */
void test_ungt(void) {
    volatile double a = v_normal2;
    volatile double b = v_nan;
    volatile double c = v_normal1;
    
    /* UNGT: a > c (ordered) */
    if (a > c) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGT with NaN operand */
    if (a > b) {  /* Should generate UNGT condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Reverse with NaN first */
    if (b > a) {  /* NaN > normal */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLE (Unordered or Less or Equal) condition */
void test_unle(void) {
    volatile double a = v_normal1;
    volatile double b = v_nan;
    volatile double c = v_normal2;
    
    /* UNLE: a <= c (ordered) */
    if (a <= c) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLE with NaN operand */
    if (a <= b) {  /* Should generate UNLE condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* NaN <= normal */
    if (b <= a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLT (Unordered or Less Than) condition */
void test_unlt(void) {
    volatile double a = v_normal1;
    volatile double b = v_nan;
    volatile double c = v_normal2;
    
    /* UNLT: a < c (ordered) */
    if (a < c) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLT with NaN operand */
    if (a < b) {  /* Should generate UNLT condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* NaN < normal */
    if (b < a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test LTGT (Less Than or Greater Than - ordered not equal) condition */
void test_ltgt(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal2;
    volatile double c = v_nan;
    
    /* LTGT: a != b (ordered not equal) */
    if (a != b) {  /* Should generate LTGT condition */
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
    
    /* LTGT with potential NaN (should be false) */
    if (a != c) {  /* Comparison with NaN */
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
    volatile double d = v_inf;
    
    /* Complex expression triggering multiple condition codes */
    int r1 = (a < b) ? 1 : 0;  /* UNLT */
    int r2 = (b >= c) ? 1 : 0; /* UNGE */
    int r3 = (c != a) ? 1 : 0; /* LTGT */
    int r4 = (d == d) ? 1 : 0; /* ORDERED */
    int r5 = (b != b) ? 1 : 0; /* UNORDERED */
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    results[idx++] = r5;
    
    /* Nested conditionals */
    if (a > b) {          /* UNGT */
        if (c <= d) {     /* UNLE with inf */
            results[idx++] = 1;
        } else {
            results[idx++] = 0;
        }
    } else {
        results[idx++] = 2;
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
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    /* Compute checksum to ensure all comparisons executed */
    int checksum = 0;
    for (int i = 0; i < idx && i < 32; i++) {
        checksum = (checksum * 31 + results[i]) & 0xFF;
        use_result(results[i]);  /* Prevent optimization */
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Total comparisons: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy function to prevent optimization */
void use_result(int val) {
    /* This function is implemented elsewhere or as inline assembly */
    /* For testing, we can use a simple volatile store */
    static volatile int sink;
    sink = val;
}
