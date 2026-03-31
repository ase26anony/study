/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile double vd_nan = __builtin_nan("");
volatile float vf_nan = __builtin_nanf("");

/* Function prototypes for different condition code patterns */
void test_unordered(void);
void test_ordered(void);
void test_uneq(void);
void test_unge(void);
void test_ungt(void);
void test_unle(void);
void test_unlt(void);
void test_ltgt(void);
void test_mixed_precision(void);
void test_with_constants(void);
void test_function_returns(void);

int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Each test modifies checksum based on comparison results */
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_mixed_precision();
    test_with_constants();
    test_function_returns();
    
    /* Use checksum to ensure all comparisons affect program output */
    printf("Final checksum: %d\n", checksum);
    
    /* Return non-zero if any NaN comparisons succeeded */
    return checksum == 0 ? 0 : 1;
}

/* Test UNORDERED condition code (unord) */
void test_unordered(void) {
    double d1 = vd1;
    double d2 = vd_nan;
    float f1 = vf1;
    float f2 = vf_nan;
    
    /* Direct unordered checks using __builtin_isunordered */
    int res1 = __builtin_isunordered(d1, d2);
    int res2 = __builtin_isunordered(f1, f2);
    int res3 = __builtin_isunordered(d2, d1);
    
    /* Alternative: !(a == a) to detect NaN */
    int res4 = !(d2 == d2);
    int res5 = !(f2 == f2);
    
    /* Use results in control flow */
    if (res1) {
        printf("UNORDERED: d1 vs d_nan\n");
    }
    
    if (res2) {
        printf("UNORDERED: f1 vs f_nan\n");
    }
    
    /* Force compiler to generate code for these comparisons */
    volatile int sink = res1 + res2 + res3 + res4 + res5;
    (void)sink;
}

/* Test ORDERED condition code (ord) */
void test_ordered(void) {
    double d1 = vd1;
    double d2 = vd2;
    double d_nan = vd_nan;
    
    /* Ordered checks - both operands are not NaN */
    int res1 = !__builtin_isunordered(d1, d2);
    int res2 = !__builtin_isunordered(d1, d1);
    int res3 = !__builtin_isunordered(d_nan, d1);  /* False */
    
    /* Use in ternary operator */
    int value = res1 ? 100 : 200;
    value += res2 ? 10 : 20;
    
    /* Array indexing based on ordered check */
    int array[2] = {1, 2};
    int idx = res3 ? 0 : 1;
    volatile int elem = array[idx];
    (void)elem;
    
    if (res1 && res2) {
        printf("ORDERED: normal values\n");
    }
}

/* Test UNEQ condition code (ueq) */
void test_uneq(void) {
    double d1 = vd1;
    double d2 = vd1;  /* Same value */
    double d_nan = vd_nan;
    
    /* UNEQ: unordered or equal */
    int res1 = __builtin_isunordered(d1, d_nan) || (d1 == d2);
    int res2 = __builtin_isunordered(d_nan, d_nan) || (d_nan == d_nan);
    
    /* Using !(a != b) which includes unordered case */
    int res3 = !(d1 != d2);  /* Equal, not unordered */
    int res4 = !(d1 != d_nan);  /* Unordered */
    
    /* Control flow forcing code generation */
    while (res1) {
        printf("UNEQ test passed once\n");
        break;
    }
    
    volatile int sink = res2 + res3 + res4;
    (void)sink;
}

/* Test UNGE condition code (nlt) */
void test_unge(void) {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    /* UNGE: unordered or not less than (greater than or equal) */
    /* Using !(a < b) which handles unordered */
    int res1 = !(a < b);  /* a >= b or unordered */
    int res2 = !(nan < a);  /* Unordered case */
    int res3 = !(a < a);  /* Equal case */
    
    /* Inverse of less-than comparison */
    int res4 = (a >= b) || __builtin_isunordered(a, b);
    
    /* Use in if-else chain */
    if (res1) {
        /* This should be taken for the nan case */
    } else {
        printf("UNGE: unexpected\n");
    }
    
    volatile int sink = res1 + res2 + res3 + res4;
    (void)sink;
}

/* Test UNGT condition code (nle) */
void test_ungt(void) {
    double a = vd2;
    double b = vd1;
    double nan = vd_nan;
    
    /* UNGT: unordered or not less than or equal (greater than) */
    /* Using !(a <= b) */
    int res1 = !(b <= a);  /* b > a or unordered */
    int res2 = !(nan <= a);  /* Unordered */
    int res3 = !(a <= b);  /* a > b */
    
    /* Alternative formulation */
    int res4 = (a > b) || __builtin_isunordered(a, b);
    
    /* Nested conditionals to force branch generation */
    if (res1) {
        if (res2) {
            printf("UNGT: unordered case\n");
        }
    }
    
    volatile int sink = res1 + res2 + res3 + res4;
    (void)sink;
}

/* Test UNLE condition code (ule) */
void test_unle(void) {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    /* UNLE: unordered or less than or equal */
    int res1 = (a <= b) || __builtin_isunordered(a, b);
    int res2 = (nan <= b) || __builtin_isunordered(nan, b);
    int res3 = (a <= a) || __builtin_isunordered(a, a);  /* Equal case */
    
    /* Using the inverse of greater-than */
    int res4 = !(a > b) || __builtin_isunordered(a, b);
    
    /* Switch based on result */
    switch (res1 + res2) {
        case 0: break;
        case 1: printf("UNLE: one true\n"); break;
        case 2: printf("UNLE: both true\n"); break;
    }
    
    volatile int sink = res3 + res4;
    (void)sink;
}

/* Test UNLT condition code (ult) */
void test_unlt(void) {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    /* UNLT: unordered or less than */
    int res1 = (a < b) || __builtin_isunordered(a, b);
    int res2 = (nan < b) || __builtin_isunordered(nan, b);
    int res3 = (a < a) || __builtin_isunordered(a, a);  /* False */
    
    /* Using the inverse of greater-than-or-equal */
    int res4 = !(a >= b) || __builtin_isunordered(a, b);
    
    /* Loop conditional */
    int i = 0;
    while (i < 3 && res1) {
        printf("UNLT: in loop iteration %d\n", i);
        i++;
        break;  /* Only once */
    }
    
    volatile int sink = res2 + res3 + res4;
    (void)sink;
}

/* Test LTGT condition code (une) */
void test_ltgt(void) {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    /* LTGT: less than or greater than (ordered and not equal) */
    int res1 = __builtin_islessgreater(a, b);  /* Direct builtin */
    int res2 = __builtin_islessgreater(b, a);  /* Reverse */
    int res3 = __builtin_islessgreater(nan, a);  /* False (unordered) */
    
    /* Alternative: (a < b) || (a > b) with ordered check */
    int res4 = (!__builtin_isunordered(a, b) && (a != b));
    
    /* Complex expression forcing evaluation */
    int result = (res1 ? 1 : 0) + (res2 ? 2 : 0) + (res3 ? 4 : 0) + (res4 ? 8 : 0);
    
    if (result > 0) {
        printf("LTGT: result = %d\n", result);
    }
    
    volatile int sink = result;
    (void)sink;
}

/* Test mixed precision comparisons */
void test_mixed_precision(void) {
    float f1 = vf1;
    double d1 = vd1;
    float f_nan = vf_nan;
    double d_nan = vd_nan;
    
    /* Mixed float/double comparisons */
    int res1 = __builtin_isunordered(f1, d_nan);
    int res2 = !__builtin_isunordered(d1, f_nan);
    int res3 = __builtin_islessgreater(f1, d1);
    int res4 = !(f1 < d1) || __builtin_isunordered(f1, d_nan);
    
    /* Use in arithmetic */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        if (i == 0 && res1) sum += 1;
        if (i == 1 && res2) sum += 2;
        if (i == 2 && res3) sum += 4;
        if (i == 3 && res4) sum += 8;
    }
    
    printf("Mixed precision sum: %d\n", sum);
    
    volatile int sink = sum;
    (void)sink;
}

/* Test with constants including INFINITY and NAN */
void test_with_constants(void) {
    /* Comparisons with constants */
    int res1 = __builtin_isunordered(vd1, NAN);
    int res2 = !__builtin_isunordered(0.0, -0.0);  /* Ordered, equal */
    int res3 = __builtin_islessgreater(INFINITY, 1.0);
    int res4 = !(NAN < INFINITY);  /* UNGE: nlt */
    int res5 = !(1.0 <= NAN);      /* UNGT: nle */
    
    /* Array of results */
    int results[5] = {res1, res2, res3, res4, res5};
    
    /* Compute hash */
    int hash = 0;
    for (int i = 0; i < 5; i++) {
        hash = hash * 31 + results[i];
    }
    
    printf("Constants hash: %d\n", hash);
    
    volatile int sink = hash;
    (void)sink;
}

/* Test with function returns that may produce NaN */
void test_function_returns(void) {
    /* Functions that may return NaN */
    double sqrt_neg = sqrt(-1.0);
    double log_neg = log(-1.0);
    double div_zero = 0.0 / 0.0;
    
    /* Comparisons with function results */
    int res1 = __builtin_isunordered(sqrt_neg, vd1);
    int res2 = !__builtin_isunordered(log_neg, div_zero);  /* Both NaN */
    int res3 = __builtin_islessgreater(sqrt_neg, log_neg);
    int res4 = !(sqrt_neg >= div_zero);  /* UNGE inverse */
    
    /* Use in conditional expression */
    int value = res1 ? 1 : (res2 ? 2 : (res3 ? 3 : (res4 ? 4 : 0)));
    
    printf("Function returns test value: %d\n", value);
    
    volatile int sink = value;
    (void)sink;
}
