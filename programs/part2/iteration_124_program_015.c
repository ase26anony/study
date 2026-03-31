/* test_fp_conditions.c
 * Designed to trigger x86 floating-point condition code generation
 * for UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, and LTGT cases
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global results array to prevent optimization */
volatile int results[32];
volatile int idx = 0;

/* External function to force code generation */
extern void external_call(int);

/* Initialize with NaN using various methods */
static double get_nan(void) {
    return 0.0 / 0.0;  /* Produces NaN */
}

static double get_snan(void) {
    union { double d; uint64_t u; } val;
    val.u = 0x7FF0000000000001ULL; /* Signaling NaN */
    return val.d;
}

/* Test UNORDERED and ORDERED conditions */
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = get_nan();
    volatile double v3 = v2;  /* Another NaN */
    
    /* UNORDERED: v2 != v2 when v2 is NaN */
    if (v2 != v2) {  /* Should generate 'unord' */
        results[idx++] = 1;
        external_call(1);
    } else {
        results[idx++] = 0;
        external_call(0);
    }
    
    /* ORDERED: v1 == v1 when v1 is normal */
    if (v1 == v1) {  /* Should generate 'ord' */
        results[idx++] = 1;
        external_call(2);
    } else {
        results[idx++] = 0;
        external_call(3);
    }
    
    /* Mixed: ORDERED comparison with NaN */
    if (v1 == v2) {  /* Should handle unordered case */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNEQ (unordered or equal) */
void test_uneq(void) {
    volatile double a = 5.0;
    volatile double b = 5.0;
    volatile double c = get_nan();
    
    /* UNEQ: a == b (both ordered) */
    if (a == b) {  /* Should generate 'ueq' */
        results[idx++] = 1;
        external_call(4);
    } else {
        results[idx++] = 0;
        external_call(5);
    }
    
    /* UNEQ with potential NaN */
    if (a == c) {  /* Should handle unordered case */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGE (unordered or greater or equal) */
void test_unge(void) {
    volatile double x = 10.0;
    volatile double y = 5.0;
    volatile double nan = get_snan();
    
    /* UNGE: x >= y (ordered) */
    if (x >= y) {  /* Should generate 'nlt' */
        results[idx++] = 1;
        external_call(6);
    } else {
        results[idx++] = 0;
        external_call(7);
    }
    
    /* UNGE with NaN operand */
    if (x >= nan) {  /* Unordered case */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Reverse UNGE */
    if (y >= x) {  /* False case */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNGT (unordered or greater than) */
void test_ungt(void) {
    volatile double p = 7.0;
    volatile double q = 3.0;
    volatile double nan = get_nan();
    
    /* UNGT: p > q (ordered) */
    if (p > q) {  /* Should generate 'nle' */
        results[idx++] = 1;
        external_call(8);
    } else {
        results[idx++] = 0;
        external_call(9);
    }
    
    /* UNGT with NaN */
    if (p > nan) {  /* Unordered case */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLE (unordered or less or equal) */
void test_unle(void) {
    volatile double m = 2.0;
    volatile double n = 8.0;
    volatile double nan = get_nan();
    
    /* UNLE: m <= n (ordered) */
    if (m <= n) {  /* Should generate 'ule' */
        results[idx++] = 1;
        external_call(10);
    } else {
        results[idx++] = 0;
        external_call(11);
    }
    
    /* UNLE with NaN */
    if (nan <= m) {  /* Unordered case */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test UNLT (unordered or less than) */
void test_unlt(void) {
    volatile double r = 1.0;
    volatile double s = 9.0;
    volatile double nan = get_snan();
    
    /* UNLT: r < s (ordered) */
    if (r < s) {  /* Should generate 'ult' */
        results[idx++] = 1;
        external_call(12);
    } else {
        results[idx++] = 0;
        external_call(13);
    }
    
    /* UNLT with NaN */
    if (nan < r) {  /* Unordered case */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

/* Test LTGT (less than or greater than - ordered and not equal) */
void test_ltgt(void) {
    volatile double u = 4.0;
    volatile double v = 6.0;
    volatile double w = 4.0;
    volatile double nan = get_nan();
    
    /* LTGT: u != v (ordered) */
    if (u != v) {  /* Should generate 'une' */
        results[idx++] = 1;
        external_call(14);
    } else {
        results[idx++] = 0;
        external_call(15);
    }
    
    /* LTGT: equal case (should be false) */
    if (u != w) {  /* Equal, not LTGT */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* LTGT with NaN (unordered, not LTGT) */
    if (u != nan) {  /* Unordered case */
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* Using __builtin_islessgreater for explicit LTGT */
    if (__builtin_islessgreater(u, v)) {  /* Explicit LTGT */
        results[idx++] = 1;
        external_call(16);
    } else {
        results[idx++] = 0;
        external_call(17);
    }
}

/* Complex test mixing multiple conditions */
void test_mixed_conditions(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan1 = get_nan();
    volatile double nan2 = get_snan();
    
    /* Chain of comparisons to force different condition codes */
    int count = 0;
    
    /* Use ternary operator which often generates condition codes */
    count += (a < b) ? 1 : 0;      /* UNLT */
    count += (a <= b) ? 1 : 0;     /* UNLE */
    count += (a > b) ? 1 : 0;      /* UNGT */
    count += (a >= b) ? 1 : 0;     /* UNGE */
    count += (a == b) ? 1 : 0;     /* UNEQ */
    count += (a != b) ? 1 : 0;     /* LTGT */
    
    /* NaN comparisons */
    count += (nan1 == nan1) ? 0 : 1;  /* UNORDERED */
    count += (a == a) ? 1 : 0;        /* ORDERED */
    
    results[idx++] = count;
    external_call(count);
}

/* Dummy external function definition to satisfy linker */
void external_call(int val) {
    /* Empty but volatile to prevent optimization */
    __asm__ volatile ("" : : "r"(val) : "memory");
}

int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 32; i++) {
        results[i] = -1;
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
    for (int i = 0; i < idx && i < 32; i++) {
        checksum = (checksum * 31 + results[i]) & 0xFFFF;
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Result checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
