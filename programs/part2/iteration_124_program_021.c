/* test_fp_conditions.c
 * Designed to trigger x86 floating-point condition code generation
 * for UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, and LTGT
 */

#include <stdio.h>
#include <math.h>

/* Global results array to prevent optimization */
volatile int results[32];
volatile int idx = 0;

/* External function to prevent inlining */
extern void use_result(int);

/* Initialize with volatile to prevent constant folding */
volatile double v_normal1 = 1.0;
volatile double v_normal2 = 3.14159;
volatile double v_normal3 = -2.5;
volatile double v_nan = 0.0 / 0.0;  /* NaN */
volatile double v_inf = 1.0 / 0.0;  /* Infinity */

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
    
    /* Mixed: UNORDERED with two NaNs */
    volatile double c = v_nan;
    if (a < c) {  /* UNORDERED comparison */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNEQ (Unordered or Equal) */
void test_uneq(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal1;
    volatile double c = v_nan;
    
    /* UNEQ: a == b (ordered equal) */
    if (a == b) {  /* Should generate UNEQ condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNEQ with NaN possibility */
    if (a == c) {  /* UNEQ when unordered */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGE (Unordered or Greater or Equal) */
void test_unge(void) {
    volatile double a = v_normal2;
    volatile double b = v_normal1;
    volatile double c = v_nan;
    
    /* UNGE: a >= b (ordered) */
    if (a >= b) {  /* Should generate UNGE condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE with NaN */
    if (a >= c) {  /* UNGE when unordered */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGT (Unordered or Greater Than) */
void test_ungt(void) {
    volatile double a = v_normal2;
    volatile double b = v_normal1;
    volatile double c = v_nan;
    
    /* UNGT: a > b (ordered) */
    if (a > b) {  /* Should generate UNGT condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGT with NaN */
    if (c > a) {  /* UNGT when unordered */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLE (Unordered or Less or Equal) */
void test_unle(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal2;
    volatile double c = v_nan;
    
    /* UNLE: a <= b (ordered) */
    if (a <= b) {  /* Should generate UNLE condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLE with NaN */
    if (c <= a) {  /* UNLE when unordered */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLT (Unordered or Less Than) */
void test_unlt(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal2;
    volatile double c = v_nan;
    
    /* UNLT: a < b (ordered) */
    if (a < b) {  /* Should generate UNLT condition */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLT with NaN */
    if (c < a) {  /* UNLT when unordered */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test LTGT (Less Than or Greater Than - ordered and not equal) */
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
    
    /* LTGT should be false for NaN */
    if (a != c) {  /* This might generate different condition */
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
    
    /* Complex expression triggering multiple conditions */
    int r1 = (a < b) ? 1 : 0;      /* UNLT */
    int r2 = (b >= a) ? 1 : 0;     /* UNGE */
    int r3 = (c == c) ? 1 : 0;     /* UNORDERED */
    int r4 = (a == a) ? 1 : 0;     /* ORDERED */
    int r5 = (a != b) ? 1 : 0;     /* LTGT */
    int r6 = (d > a) ? 1 : 0;      /* UNGT */
    int r7 = (a <= d) ? 1 : 0;     /* UNLE */
    int r8 = (a == b) ? 1 : 0;     /* UNEQ */
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    results[idx++] = r5;
    results[idx++] = r6;
    results[idx++] = r7;
    results[idx++] = r8;
}

/* Function to use results, preventing dead code elimination */
void use_result(int val) {
    /* External linkage would be better, but this prevents optimization */
    volatile static int sink = 0;
    sink += val;
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
    
    /* Compute checksum to ensure all comparisons executed */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        use_result(results[i]);  /* Prevent optimization */
    }
    
    /* Print checksum to ensure execution */
    printf("Checksum: %d\n", checksum);
    printf("Total comparisons: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}
