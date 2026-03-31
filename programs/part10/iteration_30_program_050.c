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

/* Helper to generate NaN */
double make_nan(void) {
    return __builtin_nan("");
}

/* Helper to generate infinity */
double make_inf(void) {
    return __builtin_inf();
}

int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Test each condition code pattern */
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
    
    /* Compute checksum based on comparison results */
    {
        double a = 1.5;
        double b = __builtin_nan("");
        float c = 3.14f;
        float d = __builtin_nanf("");
        
        /* UNORDERED checks */
        checksum += __builtin_isunordered(a, b);
        checksum += __builtin_isunordered(c, d);
        checksum += __builtin_isunordered(vd1, vd_nan);
        
        /* ORDERED checks */
        checksum += !__builtin_isunordered(a, a);
        checksum += !__builtin_isunordered(c, c);
        
        /* UNEQ: unordered or equal */
        checksum += (__builtin_isunordered(a, b) || (a == b));
        
        /* UNGE: not less than (nlt) */
        checksum += !(a < b);
        
        /* UNGT: not less than or equal (nle) */
        checksum += !(a <= b);
        
        /* UNLE: unordered or less than or equal (ule) */
        checksum += (__builtin_isunordered(a, b) || (a <= b));
        
        /* UNLT: unordered or less than (ult) */
        checksum += (__builtin_isunordered(a, b) || (a < b));
        
        /* LTGT: less than or greater than (une) */
        checksum += __builtin_islessgreater(a, a + 1.0);
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}

/* Test UNORDERED condition code */
void test_unordered(void) {
    double d1 = vd1;
    double d2 = vd_nan;
    float f1 = vf1;
    float f2 = vf_nan;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(d1, d2)) {
        volatile int x = 1;
        (void)x;
    }
    
    if (__builtin_isunordered(f1, f2)) {
        volatile int x = 2;
        (void)x;
    }
    
    /* Using !(a == a) NaN test */
    if (!(d2 == d2)) {
        volatile int x = 3;
        (void)x;
    }
    
    /* Complex expression with unordered */
    int res1 = __builtin_isunordered(d1, d2) && __builtin_isunordered(f1, f2);
    volatile int vr1 = res1;
    (void)vr1;
}

/* Test ORDERED condition code */
void test_ordered(void) {
    double d1 = vd1;
    double d2 = vd2;
    float f1 = vf1;
    float f2 = vf2;
    
    /* Ordered checks */
    if (!__builtin_isunordered(d1, d2)) {
        volatile int x = 1;
        (void)x;
    }
    
    if (!__builtin_isunordered(f1, f2)) {
        volatile int x = 2;
        (void)x;
    }
    
    /* Ordered comparison in loop */
    while (!__builtin_isunordered(d1, d2)) {
        d1 += 0.1;
        if (d1 > 10.0) break;
    }
}

/* Test UNEQ (unordered or equal) condition code */
void test_uneq(void) {
    double a = vd1;
    double b = vd_nan;
    double c = vd2;
    
    /* Using explicit unordered or equal */
    if (__builtin_isunordered(a, b) || (a == b)) {
        volatile int x = 1;
        (void)x;
    }
    
    /* Should generate ueq when both sides might be NaN */
    if (!(a != b)) {  /* This is !(ne) which is ueq */
        volatile int x = 2;
        (void)x;
    }
    
    /* With volatile to force code generation */
    volatile int res = !(a != b) || !(c != b);
    (void)res;
}

/* Test UNGE (not less than - nlt) condition code */
void test_unge(void) {
    double a = vd1;
    double b = vd_nan;
    float c = vf1;
    float d = vf_nan;
    
    /* Inverse condition: !(a < b) generates nlt */
    if (!(a < b)) {
        volatile int x = 1;
        (void)x;
    }
    
    if (!(c < d)) {
        volatile int x = 2;
        (void)x;
    }
    
    /* In conditional expression */
    volatile int res = !(a < b) ? 10 : 20;
    (void)res;
}

/* Test UNGT (not less than or equal - nle) condition code */
void test_ungt(void) {
    double a = vd1;
    double b = vd_nan;
    float c = vf1;
    float d = vf_nan;
    
    /* Inverse condition: !(a <= b) generates nle */
    if (!(a <= b)) {
        volatile int x = 1;
        (void)x;
    }
    
    if (!(c <= d)) {
        volatile int x = 2;
        (void)x;
    }
    
    /* Complex expression */
    volatile int res = (!(a <= b) && !(c <= d)) ? 1 : 0;
    (void)res;
}

/* Test UNLE (unordered or less than or equal - ule) condition code */
void test_unle(void) {
    double a = vd1;
    double b = vd_nan;
    double c = 3.0;
    
    /* Explicit unordered or less than or equal */
    if (__builtin_isunordered(a, b) || (a <= b)) {
        volatile int x = 1;
        (void)x;
    }
    
    /* With normal values */
    if (__builtin_isunordered(a, c) || (a <= c)) {
        volatile int x = 2;
        (void)x;
    }
    
    /* In loop condition */
    volatile double v = a;
    while (__builtin_isunordered(v, b) || (v <= b)) {
        v += 0.5;
        if (v > 5.0) break;
    }
}

/* Test UNLT (unordered or less than - ult) condition code */
void test_unlt(void) {
    double a = vd1;
    double b = vd_nan;
    float c = vf1;
    float d = vf_nan;
    
    /* Explicit unordered or less than */
    if (__builtin_isunordered(a, b) || (a < b)) {
        volatile int x = 1;
        (void)x;
    }
    
    if (__builtin_isunordered(c, d) || (c < d)) {
        volatile int x = 2;
        (void)x;
    }
    
    /* Array indexing based on condition */
    int array[10] = {0};
    volatile int idx = (__builtin_isunordered(a, b) || (a < b)) ? 0 : 1;
    array[idx] = 42;
}

/* Test LTGT (less than or greater than - une) condition code */
void test_ltgt(void) {
    double a = vd1;
    double b = vd2;
    float c = vf1;
    float d = vf2;
    
    /* Using __builtin_islessgreater */
    if (__builtin_islessgreater(a, b)) {
        volatile int x = 1;
        (void)x;
    }
    
    if (__builtin_islessgreater(c, d)) {
        volatile int x = 2;
        (void)x;
    }
    
    /* Manual: (a < b) || (a > b) which is LTGT for ordered values */
    if ((a < b) || (a > b)) {
        volatile int x = 3;
        (void)x;
    }
    
    /* With NaN - should be false when NaN involved */
    double nan = __builtin_nan("");
    volatile int res = __builtin_islessgreater(a, nan);
    (void)res;
}

/* Test mixed precision comparisons */
void test_mixed_precision(void) {
    float f = vf1;
    double d = vd1;
    float f_nan = vf_nan;
    double d_nan = vd_nan;
    
    /* Mixed with NaN */
    if (__builtin_isunordered(f, d_nan)) {
        volatile int x = 1;
        (void)x;
    }
    
    if (!(f < d_nan)) {  /* Should generate nlt */
        volatile int x = 2;
        (void)x;
    }
    
    /* Mixed normal values */
    volatile int res1 = !(f <= d);  /* Should generate nle */
    volatile int res2 = __builtin_islessgreater(f, d);  /* Should generate une */
    (void)res1;
    (void)res2;
    
    /* Promotion and comparison */
    if ((double)f < d_nan) {
        volatile int x = 3;
        (void)x;
    }
}

/* Test with constants */
void test_with_constants(void) {
    volatile double d = vd1;
    volatile float f = vf1;
    
    /* Comparisons with 0.0 */
    if (__builtin_isunordered(d, 0.0)) {
        volatile int x = 1;
        (void)x;
    }
    
    if (!(f < 0.0f)) {  /* nlt */
        volatile int x = 2;
        (void)x;
    }
    
    if (!(d <= 0.0)) {  /* nle */
        volatile int x = 3;
        (void)x;
    }
    
    /* With INFINITY */
    if (__builtin_isunordered(d, INFINITY)) {
        volatile int x = 4;
        (void)x;
    }
    
    /* With NAN constant */
    if (__builtin_isunordered(f, NAN)) {
        volatile int x = 5;
        (void)x;
    }
    
    /* Complex constant expression */
    volatile int res = (!(d < 1.0) && __builtin_islessgreater(f, 2.0f)) ? 1 : 0;
    (void)res;
}

/* Test with function returns */
void test_function_returns(void) {
    /* Functions that may return NaN */
    double d1 = make_nan();
    double d2 = make_inf();
    double d3 = sqrt(-1.0);  /* Returns NaN */
    
    /* Comparisons with function returns */
    if (__builtin_isunordered(vd1, d1)) {
        volatile int x = 1;
        (void)x;
    }
    
    if (!(d2 < d3)) {  /* nlt with inf and nan */
        volatile int x = 2;
        (void)x;
    }
    
    if (__builtin_islessgreater(sin(0.0), cos(0.0))) {
        volatile int x = 3;
        (void)x;
    }
    
    /* Chain of comparisons */
    volatile int res = __builtin_isunordered(d1, d3) || 
                      !(sin(0.0) <= cos(0.0)) ||
                      __builtin_islessgreater(d2, 100.0);
    (void)res;
}
