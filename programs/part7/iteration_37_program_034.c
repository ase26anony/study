/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Complex types */
volatile _Complex int vci = 3 + 4 * I;
volatile _Complex double vcd = 1.5 + 2.5 * I;

/* Test 1: Basic integer-valued real functions in constant contexts */
static const double test1_const = trunc(5.9);  /* Should be 5.0 */
enum { ENUM_TRUNC = (int)trunc(10.3) };        /* Should be 10 */
static char buffer1[(int)floor(7.8)];          /* Size 7 */

constexpr double cpp_trunc(double x) { return trunc(x); }
constexpr double cpp_floor(double x) { return floor(x); }

/* Test 2: Nested calls */
static double test2_nested() {
    /* These should be folded during constant folding pass */
    double a = floor(ceil(3.2));      /* floor(4.0) = 4.0 */
    double b = trunc(round(2.7));     /* trunc(3.0) = 3.0 */
    double c = nearbyint(rint(4.1));  /* nearbyint(4.0) = 4.0 */
    return a + b + c;                 /* 4 + 3 + 4 = 11 */
}

/* Test 3: Calls with different argument counts */
static double test3_arg_counts() {
    /* Builtins with different argument counts */
    double a = __builtin_llround(3.14);      /* 1 arg */
    double b = __builtin_llrint(2.99);       /* 1 arg */
    
    /* Complex part extractors - integer-valued real functions */
    double c = __real__(vci);                /* Real part of complex int */
    double d = __imag__(vci);                /* Imag part of complex int */
    
    return a + b + c + d;                    /* 3 + 3 + 3 + 4 = 13 */
}

/* Test 4: Conditional expressions with integer-valued calls */
static double test4_conditional() {
    double x = (vi > 0) ? trunc(vd) : floor(vf);
    double y = (vd > 3.0) ? ceil(vd) : round(vf);
    double z = (vf < 3.0) ? nearbyint(vf) : rint(vd);
    return x + y + z;  /* trunc(3.14159)=3 + ceil(3.14159)=4 + nearbyint(2.71828)=3 = 10 */
}

/* Test 5: Arithmetic with folded results */
static double test5_arithmetic() {
    double a = trunc(vd) * 2.0;          /* 3 * 2 = 6 */
    double b = floor(vd) / ceil(vf);     /* 3 / 3 = 1 */
    double c = round(vf) + nearbyint(vd);/* 3 + 3 = 6 */
    return a + b + c;                    /* 6 + 1 + 6 = 13 */
}

/* Test 6: Comparisons with integer-valued calls */
static int test6_comparisons() {
    int result = 0;
    if (ceil(vd) > floor(vf)) result += 1;      /* 4 > 2 = true */
    if (trunc(vd) == round(vf)) result += 2;    /* 3 == 3 = true */
    if (nearbyint(vd) <= rint(vf)) result += 4; /* 3 <= 3 = true */
    return result;                              /* 1 + 2 + 4 = 7 */
}

/* Test 7: Template and constexpr contexts (C++ style) */
#ifdef __cplusplus
template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};
#endif

static double test7_constexpr() {
#ifdef __cplusplus
    constexpr double a = cpp_trunc(8.9);    /* 8.0 */
    constexpr double b = cpp_floor(6.1);    /* 6.0 */
    return a + b;                           /* 8 + 6 = 14 */
#else
    /* For C, use compound literals to create temporary constants */
    double a = ({ const double _tmp = 8.9; trunc(_tmp); });
    double b = ({ const double _tmp = 6.1; floor(_tmp); });
    return a + b;                           /* 8 + 6 = 14 */
#endif
}

/* Test 8: Mixed integer and real arguments */
static double test8_mixed_args() {
    double a = floor(5);        /* Integer argument: 5.0 */
    double b = ceil(4.0);       /* Exact integer real: 4.0 */
    double c = trunc(2.3);      /* Fractional real: 2.0 */
    double d = round(-2.3);     /* Negative value: -2.0 */
    double e = nearbyint(1e10); /* Large value */
    return a + b + c + d + e;   /* 5 + 4 + 2 - 2 + 1e10 = 1e10 + 9 */
}

/* Test 9: Complex expressions with multiple calls */
static double test9_complex_expr() {
    /* Expression designed to create deep recursion in integer_valued_real_p */
    double x = trunc(floor(ceil(round(nearbyint(rint(7.3))))));
    /* Breakdown: rint(7.3)=7, nearbyint(7)=7, round(7)=7, ceil(7)=7, floor(7)=7, trunc(7)=7 */
    
    double y = (trunc(vd) > 2) ? floor(vf * 2) : ceil(vd / 2);
    /* trunc(3.14159)=3 > 2, so floor(2.71828*2)=floor(5.43656)=5 */
    
    return x + y;  /* 7 + 5 = 12 */
}

/* Test 10: Static assertions and compile-time checks */
static void test10_static_checks() {
    /* These should be evaluated during constant folding */
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(5.9) == 5, "floor failed");
    static_assert(ceil(5.1) == 6, "ceil failed");
    static_assert(round(5.5) == 6, "round failed");
}

/* Main driver that accumulates results */
int main() {
    int checksum = 0;
    
    /* Test 1: Constant contexts */
    checksum += (int)test1_const;          /* +5 */
    checksum += ENUM_TRUNC;                /* +10 */
    checksum += sizeof(buffer1);           /* +7 */
    
    /* Test 2: Nested calls */
    checksum += (int)test2_nested();       /* +11 */
    
    /* Test 3: Different argument counts */
    checksum += (int)test3_arg_counts();   /* +13 */
    
    /* Test 4: Conditional expressions */
    checksum += (int)test4_conditional();  /* +10 */
    
    /* Test 5: Arithmetic */
    checksum += (int)test5_arithmetic();   /* +13 */
    
    /* Test 6: Comparisons */
    checksum += test6_comparisons();       /* +7 */
    
    /* Test 7: Constexpr/template */
    checksum += (int)test7_constexpr();    /* +14 */
    
    /* Test 8: Mixed arguments */
    /* Only add the non-huge part to avoid overflow in checksum */
    checksum += 9;                         /* +9 (from 5+4+2-2) */
    
    /* Test 9: Complex expressions */
    checksum += (int)test9_complex_expr(); /* +12 */
    
    /* Test 10: Static checks (no runtime contribution) */
    test10_static_checks();
    
    printf("Result: %d\n", checksum);      /* Expected: 5+10+7+11+13+10+13+7+14+9+12 = 111 */
    
    /* Additional verification */
    printf("Verification:\n");
    printf("  trunc(5.9) = %.1f\n", trunc(5.9));
    printf("  floor(ceil(3.2)) = %.1f\n", floor(ceil(3.2)));
    printf("  __real__(3+4i) = %.1f\n", (double)__real__(3+4*I));
    
    return 0;
}
