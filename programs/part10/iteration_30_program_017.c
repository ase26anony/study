/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = __builtin_nan("");
volatile double vd_inf = __builtin_inf();
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
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
void test_function_results(void);

/* Helper to generate checksum */
static uint32_t checksum = 0;

static void add_to_checksum(int val) {
    checksum = checksum * 31 + (uint32_t)val;
}

/* Test UNORDERED condition code */
void test_unordered(void) {
    double a = vd_nan;
    double b = vd1;
    float fa = vf_nan;
    float fb = vf1;
    
    /* Direct unordered checks */
    int res1 = __builtin_isunordered(a, b);
    int res2 = __builtin_isunordered(b, a);
    int res3 = __builtin_isunordered(a, a);  /* NaN vs NaN */
    int res4 = __builtin_isunordered(fa, fb);
    
    /* Using !(a == a) pattern */
    int res5 = !(a == a);
    int res6 = !(fa == fa);
    
    /* Control flow based on unordered */
    if (__builtin_isunordered(a, b)) {
        add_to_checksum(1);
    }
    
    if (!__builtin_isunordered(b, b)) {  /* b is not NaN */
        add_to_checksum(2);
    }
    
    add_to_checksum(res1 + res2 + res3 + res4 + res5 + res6);
}

/* Test ORDERED condition code */
void test_ordered(void) {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    /* Direct ordered checks */
    int res1 = __builtin_isordered(a, b);
    int res2 = __builtin_isordered(a, nan);
    int res3 = __builtin_isordered(nan, b);
    
    /* Control flow with ordered */
    while (__builtin_isordered(a, b)) {
        add_to_checksum(3);
        break;  /* Execute once */
    }
    
    /* Ternary operator with ordered */
    int res4 = __builtin_isordered(a, b) ? 4 : 5;
    
    add_to_checksum(res1 + res2 + res3 + res4);
}

/* Test UNEQ (unordered or equal) */
void test_uneq(void) {
    double a = vd1;
    double b = vd1;  /* Equal values */
    double nan = vd_nan;
    
    /* Using !(a != b) which includes unordered case */
    int res1 = !(a != b);  /* Should be true for equal or unordered */
    int res2 = !(nan != nan);  /* NaN != NaN is false, so !false = true */
    
    /* Complex expression that might generate UNEQ */
    int res3 = (a == b) || __builtin_isunordered(a, b);
    
    /* Control flow */
    if (!(a != b)) {
        add_to_checksum(6);
    }
    
    add_to_checksum(res1 + res2 + res3);
}

/* Test UNGE (not less than) = greater than or equal or unordered */
void test_unge(void) {
    double a = vd2;
    double b = vd1;
    double nan = vd_nan;
    
    /* Inverse of less than */
    int res1 = !(a < b);  /* a >= b or unordered */
    int res2 = !(nan < b);  /* NaN < b is false, so !false = true (unordered) */
    
    /* Direct comparison that might generate nlt */
    int res3 = (a >= b) || __builtin_isunordered(a, b);
    
    /* With different values */
    if (!(vd1 < vd2)) {
        add_to_checksum(7);
    }
    
    add_to_checksum(res1 + res2 + res3);
}

/* Test UNGT (not less than or equal) = greater than or unordered */
void test_ungt(void) {
    double a = vd2;
    double b = vd1;
    double nan = vd_nan;
    
    /* Inverse of less than or equal */
    int res1 = !(a <= b);  /* a > b or unordered */
    int res2 = !(nan <= b);  /* NaN <= b is false, so !false = true */
    
    /* Complex expression */
    int res3 = (a > b) || __builtin_isunordered(a, b);
    
    /* Control flow */
    while (!(vd1 <= vd_nan)) {
        add_to_checksum(8);
        break;
    }
    
    add_to_checksum(res1 + res2 + res3);
}

/* Test UNLE (unordered or less than or equal) */
void test_unle(void) {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    /* Using <= operator which handles NaN */
    int res1 = (a <= b) || __builtin_isunordered(a, b);
    int res2 = (nan <= b);  /* Should be true (unordered case) */
    
    /* Direct comparison */
    if (vd1 <= vd2) {
        add_to_checksum(9);
    }
    
    /* With NaN */
    int res3 = (nan <= nan);
    
    add_to_checksum(res1 + res2 + res3);
}

/* Test UNLT (unordered or less than) */
void test_unlt(void) {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    /* Using < operator */
    int res1 = (a < b) || __builtin_isunordered(a, b);
    int res2 = (nan < b);  /* Should be true (unordered case) */
    
    /* Direct comparison in control flow */
    if (vd1 < vd2) {
        add_to_checksum(10);
    }
    
    /* Ternary operator */
    int res3 = (nan < nan) ? 11 : 12;
    
    add_to_checksum(res1 + res2 + res3);
}

/* Test LTGT (less than or greater than, ordered) = not equal and ordered */
void test_ltgt(void) {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    /* Using __builtin_islessgreater */
    int res1 = __builtin_islessgreater(a, b);
    int res2 = __builtin_islessgreater(a, a);  /* Equal, should be false */
    int res3 = __builtin_islessgreater(nan, b);  /* NaN, should be false */
    
    /* Manual version: (a < b) || (a > b) but both ordered */
    int res4 = (a < b) || (a > b);
    int res5 = (__builtin_isless(a, b) || __builtin_isgreater(a, b));
    
    /* Control flow */
    if (__builtin_islessgreater(vd1, vd2)) {
        add_to_checksum(13);
    }
    
    add_to_checksum(res1 + res2 + res3 + res4 + res5);
}

/* Test mixed precision comparisons */
void test_mixed_precision(void) {
    float f1 = vf1;
    double d1 = vd1;
    float f_nan = vf_nan;
    double d_nan = vd_nan;
    
    /* Mixed float/double comparisons */
    int res1 = __builtin_isunordered(f1, d1);
    int res2 = __builtin_isordered(f_nan, d1);
    int res3 = !(f1 < d1);  /* Potential UNGE */
    int res4 = !(f1 <= d1); /* Potential UNGT */
    int res5 = (f1 <= d1) || __builtin_isunordered(f1, d1); /* UNLE */
    int res6 = (f1 < d1) || __builtin_isunordered(f1, d1);  /* UNLT */
    int res7 = __builtin_islessgreater(f1, d1); /* LTGT */
    
    /* Control flow with mixed types */
    if (__builtin_isunordered(f_nan, d1)) {
        add_to_checksum(14);
    }
    
    add_to_checksum(res1 + res2 + res3 + res4 + res5 + res6 + res7);
}

/* Test with constants */
void test_with_constants(void) {
    /* Comparisons with various constants */
    int res1 = __builtin_isunordered(vd_nan, 0.0);
    int res2 = __builtin_isordered(1.0, 2.0);
    int res3 = !(vd1 < 0.0);      /* UNGE */
    int res4 = !(vd1 <= 0.0);     /* UNGT */
    int res5 = (vd1 <= 0.0) || __builtin_isunordered(vd1, 0.0); /* UNLE */
    int res6 = (vd1 < 0.0) || __builtin_isunordered(vd1, 0.0);  /* UNLT */
    int res7 = __builtin_islessgreater(vd1, 0.0); /* LTGT */
    
    /* Special constants */
    int res8 = __builtin_isunordered(vd_inf, -vd_inf);
    int res9 = !(vd_inf < vd_inf); /* UNGE */
    
    /* Control flow with constants */
    if (!(3.14 < 2.71)) {
        add_to_checksum(15);
    }
    
    add_to_checksum(res1 + res2 + res3 + res4 + res5 + res6 + res7 + res8 + res9);
}

/* Test with function results */
double get_nan(void) {
    return __builtin_nan("");
}

double get_value(double x) {
    return x * 2.0;
}

void test_function_results(void) {
    /* Compare function results */
    int res1 = __builtin_isunordered(get_nan(), get_value(1.0));
    int res2 = __builtin_isordered(get_value(1.0), get_value(2.0));
    int res3 = !(get_value(1.0) < get_value(2.0)); /* UNGE */
    int res4 = !(get_value(1.0) <= get_value(1.0)); /* UNGT (false) */
    int res5 = (get_value(1.0) <= get_value(2.0)) || 
               __builtin_isunordered(get_value(1.0), get_value(2.0)); /* UNLE */
    int res6 = (get_value(1.0) < get_value(2.0)) || 
               __builtin_isunordered(get_value(1.0), get_value(2.0)); /* UNLT */
    int res7 = __builtin_islessgreater(get_value(1.0), get_value(2.0)); /* LTGT */
    
    /* Math functions that might return NaN */
    double sqrt_neg = sqrt(-1.0);
    int res8 = __builtin_isunordered(sqrt_neg, 0.0);
    
    /* Control flow with function results */
    if (__builtin_islessgreater(get_value(2.0), get_value(1.0))) {
        add_to_checksum(16);
    }
    
    add_to_checksum(res1 + res2 + res3 + res4 + res5 + res6 + res7 + res8);
}

/* Main driver */
int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests */
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
    test_function_results();
    
    printf("Checksum: %u\n", checksum);
    printf("Test completed.\n");
    
    /* Return value based on checksum to ensure all code paths matter */
    return (checksum % 256) == 0 ? 0 : 1;
}
