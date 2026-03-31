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

/* Function to generate UNORDERED condition code (unord) */
int test_unordered(void) {
    double a = vd_nan;
    double b = vd1;
    float c = vf_nan;
    float d = vf1;
    
    /* Direct unordered checks using builtins */
    int res1 = __builtin_isunordered(a, b);
    int res2 = __builtin_isunordered(c, d);
    
    /* Unordered check via NaN test */
    int res3 = !(a == a) || !(b == b);
    
    /* Mixed types */
    int res4 = __builtin_isunordered(a, (double)c);
    
    return res1 + res2 + res3 + res4;
}

/* Function to generate ORDERED condition code (ord) */
int test_ordered(void) {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    /* Direct ordered checks */
    int res1 = __builtin_isordered(a, b);
    int res2 = __builtin_isordered(a, c);
    
    /* Ordered check via negation of unordered */
    int res3 = !__builtin_isunordered(a, b);
    
    /* With constants */
    int res4 = __builtin_isordered(3.14, 2.71);
    
    return res1 + res2 + res3 + res4;
}

/* Function to generate UNEQ condition code (ueq) */
int test_uneq(void) {
    double a = vd1;
    double b = vd1;  /* Equal values */
    double c = vd_nan;
    
    /* UNEQ: unordered or equal */
    int res1 = __builtin_isunordered(a, b) || (a == b);
    int res2 = __builtin_isunordered(c, a) || (c == a);
    
    /* Using !(a != b) which includes unordered case */
    int res3 = !(a != b);
    
    /* Mixed float/double */
    float f1 = vf1;
    float f2 = vf1;
    int res4 = __builtin_isunordered(f1, f2) || (f1 == f2);
    
    return res1 + res2 + res3 + res4;
}

/* Function to generate UNGE condition code (nlt) */
int test_unge(void) {
    double a = vd2;
    double b = vd1;
    double c = vd_nan;
    
    /* UNGE: unordered or not less than (nlt) */
    int res1 = __builtin_isunordered(a, b) || !(a < b);
    int res2 = __builtin_isunordered(c, b) || !(c < b);
    
    /* Inverse of less than */
    int res3 = !(a < b);  /* May generate nlt */
    
    /* With function return values */
    double d = sin(1.0);
    double e = cos(1.0);
    int res4 = __builtin_isunordered(d, e) || !(d < e);
    
    return res1 + res2 + res3 + res4;
}

/* Function to generate UNGT condition code (nle) */
int test_ungt(void) {
    double a = vd2;
    double b = vd1;
    double c = vd_nan;
    
    /* UNGT: unordered or not less than or equal (nle) */
    int res1 = __builtin_isunordered(a, b) || !(a <= b);
    int res2 = __builtin_isunordered(c, b) || !(c <= b);
    
    /* Inverse of less than or equal */
    int res3 = !(a <= b);  /* May generate nle */
    
    /* Using > which is equivalent to !(<=) for ordered values */
    int res4 = (a > b);
    
    return res1 + res2 + res3 + res4;
}

/* Function to generate UNLE condition code (ule) */
int test_unle(void) {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    /* UNLE: unordered or less than or equal */
    int res1 = __builtin_isunordered(a, b) || (a <= b);
    int res2 = __builtin_isunordered(c, b) || (c <= b);
    
    /* Direct <= with NaN possibility */
    int res3 = (a <= b);
    
    /* Mixed precision */
    float f1 = vf1;
    float f2 = vf2;
    int res4 = __builtin_isunordered(f1, f2) || (f1 <= f2);
    
    return res1 + res2 + res3 + res4;
}

/* Function to generate UNLT condition code (ult) */
int test_unlt(void) {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    /* UNLT: unordered or less than */
    int res1 = __builtin_isunordered(a, b) || (a < b);
    int res2 = __builtin_isunordered(c, b) || (c < b);
    
    /* Direct < with NaN possibility */
    int res3 = (a < b);
    
    /* With volatile forcing memory reads */
    volatile double v1 = 1.5;
    volatile double v2 = 2.5;
    int res4 = __builtin_isunordered(v1, v2) || (v1 < v2);
    
    return res1 + res2 + res3 + res4;
}

/* Function to generate LTGT condition code (une) */
int test_ltgt(void) {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    /* LTGT: less than or greater than (ordered and not equal) */
    int res1 = __builtin_islessgreater(a, b);
    int res2 = __builtin_islessgreater(b, a);
    int res3 = __builtin_islessgreater(c, a);
    
    /* Equivalent using ordered comparisons */
    int res4 = (__builtin_isordered(a, b) && ((a < b) || (a > b)));
    
    return res1 + res2 + res3 + res4;
}

/* Advanced: Direct assembly with condition codes */
#ifdef USE_ASM
int test_asm_conditions(void) {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    int res1, res2, res3, res4;
    
    /* Using inline asm to force specific condition codes */
    __asm__ volatile (
        "fucomip %2, %1\n\t"
        "seta %0"
        : "=r"(res1)
        : "t"(a), "u"(b)
        : "cc"
    );
    
    __asm__ volatile (
        "fucomip %2, %1\n\t"
        "setb %0"
        : "=r"(res2)
        : "t"(b), "u"(a)
        : "cc"
    );
    
    /* Unordered check */
    __asm__ volatile (
        "fucomip %2, %1\n\t"
        "setp %0"
        : "=r"(res3)
        : "t"(a), "u"(c)
        : "cc"
    );
    
    return res1 + res2 + res3;
}
#endif

/* Control flow based on comparisons */
void test_control_flow(int *results) {
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    /* if statements with various conditions */
    if (__builtin_isunordered(a, nan)) {
        results[0] = 1;
    }
    
    if (__builtin_isordered(a, b)) {
        results[1] = 2;
    }
    
    if (!(a < b)) {  /* Potential nlt */
        results[2] = 3;
    }
    
    if (!(a <= b)) { /* Potential nle */
        results[3] = 4;
    }
    
    if (__builtin_islessgreater(a, b)) { /* une */
        results[4] = 5;
    }
    
    /* Ternary operator */
    results[5] = __builtin_isunordered(a, nan) ? 6 : 7;
    results[6] = !(a < b) ? 8 : 9;  /* nlt */
}

/* Main driver function */
int main(void) {
    int results[10] = {0};
    int checksum = 0;
    
    /* Call all test functions */
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    
    /* Test control flow patterns */
    test_control_flow(results);
    
    /* Add results to checksum */
    for (int i = 0; i < 10; i++) {
        checksum += results[i];
    }
    
    /* Use checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    /* Additional runtime tests with function returns */
    double d1 = sqrt(-1.0);  /* Returns NaN */
    double d2 = log(0.0);    /* Returns -inf */
    
    if (__builtin_isunordered(d1, d2)) {
        checksum += 100;
    }
    
    if (__builtin_islessgreater(vd1, vd2)) {
        checksum += 200;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
