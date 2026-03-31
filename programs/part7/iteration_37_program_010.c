/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Global variables to force constant folding in initializers */
static const double cd = 5.5;
static const float cf = -3.3f;
static const int ci = 7;

/* Test 1: Basic integer-valued real functions in constant contexts */
int test_basic_functions(void) {
    /* These should be folded by fold-const.cc */
    const double t1 = trunc(cd);        /* 5.0 */
    const double t2 = floor(cd);        /* 5.0 */
    const double t3 = ceil(cf);         /* -3.0 */
    const double t4 = round(cd);        /* 6.0 */
    const double t5 = nearbyint(cd);    /* 6.0 */
    const double t6 = rint(cd);         /* 6.0 */
    
    /* Use in array size (C99 VLA or C++ template would be better) */
    char buffer1[(int)t1];
    char buffer2[(int)t2];
    
    return (int)(t1 + t2 + t3 + t4 + t5 + t6);
}

/* Test 2: Builtin functions with integer return types */
long long test_builtin_ll(void) {
    /* __builtin_llround and __builtin_llrint return long long */
    long long t1 = __builtin_llround(cd);    /* 6 */
    long long t2 = __builtin_llrint(cd);     /* 6 */
    long long t3 = __builtin_llround(cf);    /* -3 */
    long long t4 = __builtin_llrint(cf);     /* -3 */
    
    return t1 + t2 + t3 + t4;
}

/* Test 3: Complex number real/imag parts */
int test_complex_parts(void) {
    /* Complex integer types */
    complex int c1 = 3 + 4 * I;
    complex long c2 = -5 + 6 * I;
    
    /* These extract real/imag parts as integer-valued reals */
    double r1 = __real__ c1;    /* 3.0 */
    double i1 = __imag__ c1;    /* 4.0 */
    double r2 = __real__ c2;    /* -5.0 */
    double i2 = __imag__ c2;    /* 6.0 */
    
    /* Use in folded expressions */
    const double sum1 = trunc(r1) + floor(i1);
    const double sum2 = ceil(r2) + round(i2);
    
    return (int)(sum1 + sum2);
}

/* Test 4: Nested calls */
double test_nested_calls(void) {
    /* Multiple levels of nesting */
    const double t1 = floor(ceil(cd));           /* floor(6.0) = 6.0 */
    const double t2 = trunc(round(cf));          /* trunc(-3.0) = -3.0 */
    const double t3 = nearbyint(rint(cd));       /* nearbyint(6.0) = 6.0 */
    const double t4 = __builtin_llround(floor(cd)); /* llround(5.0) = 5 */
    
    /* Deep nesting */
    const double t5 = trunc(floor(ceil(round(cd))));
    
    return t1 + t2 + t3 + t4 + t5;
}

/* Test 5: Calls in conditional expressions */
double test_conditional_calls(void) {
    const double a = cd;
    const double b = cf;
    
    /* Conditional operator with integer-valued calls */
    const double t1 = (a > 0) ? trunc(a) : floor(b);
    const double t2 = (b < 0) ? ceil(b) : round(a);
    const double t3 = (ci == 7) ? nearbyint(a) : rint(b);
    
    /* Nested conditional with calls */
    const double t4 = (t1 > t2) ? floor(t1) : ceil(t2);
    
    return t1 + t2 + t3 + t4;
}

/* Test 6: Calls as function arguments */
double test_call_arguments(void) {
    /* Integer-valued calls as arguments to other integer-valued calls */
    const double t1 = round(trunc(cd));          /* round(5.0) = 5.0 */
    const double t2 = floor(ceil(cf));           /* floor(-3.0) = -3.0 */
    const double t3 = nearbyint(__builtin_llround(cd)); /* nearbyint(6) = 6.0 */
    
    /* Multiple arguments */
    const double t4 = fmax(floor(cd), ceil(cf)); /* fmax(5.0, -3.0) = 5.0 */
    const double t5 = fmin(round(cd), trunc(cf)); /* fmin(6.0, -3.0) = -3.0 */
    
    return t1 + t2 + t3 + t4 + t5;
}

/* Test 7: Mixed with arithmetic operations */
double test_mixed_arithmetic(void) {
    const double a = cd;
    const double b = cf;
    
    /* Arithmetic with integer-valued calls */
    const double t1 = (trunc(a) * 2.0) / floor(b + 8.0); /* (5*2)/5 = 2.0 */
    const double t2 = ceil(a) + round(b) - nearbyint(a); /* 6 + (-3) - 6 = -3.0 */
    const double t3 = rint(a) * trunc(b);                /* 6 * (-3) = -18.0 */
    
    /* Comparisons that should fold */
    const int cmp1 = (ceil(a) > floor(b)) ? 1 : 0;  /* 6 > -3 = true */
    const int cmp2 = (trunc(a) == round(b + 0.5)) ? 1 : 0; /* 5 == -3? false */
    
    return t1 + t2 + t3 + cmp1 + cmp2;
}

/* Test 8: Type casts and conversions */
int test_type_conversions(void) {
    /* Explicit casts of integer-valued reals */
    const int t1 = (int)trunc(cd);        /* 5 */
    const int t2 = (int)ceil(cf);         /* -3 */
    const long t3 = (long)round(cd);      /* 6 */
    const long long t4 = (long long)floor(cd); /* 5 */
    
    /* Implicit conversions in expressions */
    const double t5 = t1 + trunc(cd);     /* 5 + 5.0 = 10.0 */
    const float t6 = t2 * ceil(cf);       /* -3 * -3.0 = 9.0f */
    
    return t1 + t2 + t3 + t4 + (int)t5 + (int)t6;
}

/* Test 9: Edge cases and special values */
double test_edge_cases(void) {
    /* Exact integers */
    const double e1 = floor(4.0);         /* 4.0 */
    const double e2 = ceil(-2.0);         /* -2.0 */
    
    /* Large values */
    const double e3 = trunc(1e10 + 0.5);  /* 10000000000.0 */
    const double e4 = round(1e10 - 0.5);  /* 9999999999.0 */
    
    /* Negative fractional values */
    const double e5 = floor(-4.7);        /* -5.0 */
    const double e6 = ceil(-4.7);         /* -4.0 */
    const double e7 = trunc(-4.7);        /* -4.0 */
    const double e8 = round(-4.7);        /* -5.0 */
    
    return e1 + e2 + e3/1e9 + e4/1e9 + e5 + e6 + e7 + e8;
}

/* Test 10: Inline assembly to prevent optimization (but still allow folding) */
int test_with_barrier(void) {
    double a = cd;
    double b = cf;
    
    /* Compiler memory barrier */
    __asm__ volatile ("" : : "r"(&a), "r"(&b) : "memory");
    
    /* These should still be folded where possible */
    const double t1 = trunc(a);
    const double t2 = floor(b);
    
    __asm__ volatile ("" : : "r"(&t1), "r"(&t2));
    
    return (int)(t1 + t2);
}

/* C++ specific tests (compile with g++) */
#ifdef __cplusplus
#include <type_traits>

template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};

constexpr double constexpr_floor(double x) {
    return __builtin_floor(x);
}

constexpr double constexpr_round(double x) {
    return __builtin_round(x);
}

int test_cpp_features(void) {
    /* Template argument */
    const int t1 = TestTemplate<7>::value;  /* trunc(10.5) = 10 */
    
    /* constexpr function */
    constexpr double t2 = constexpr_floor(8.9);  /* 8.0 */
    constexpr double t3 = constexpr_round(8.9);  /* 9.0 */
    
    /* static_assert */
    static_assert(constexpr_floor(5.1) == 5, "");
    static_assert(constexpr_round(5.1) == 5, "");
    
    /* Array size */
    char buffer[(int)constexpr_floor(12.7)];  /* size 12 */
    
    return t1 + (int)t2 + (int)t3 + sizeof(buffer);
}
#else
int test_cpp_features(void) {
    return 0;
}
#endif

/* Main driver */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_functions();
    checksum += test_builtin_ll();
    checksum += test_complex_parts();
    checksum += (int)test_nested_calls();
    checksum += (int)test_conditional_calls();
    checksum += (int)test_call_arguments();
    checksum += (int)test_mixed_arithmetic();
    checksum += test_type_conversions();
    checksum += (int)test_edge_cases();
    checksum += test_with_barrier();
    checksum += test_cpp_features();
    
    /* Use volatile to prevent dead code elimination */
    volatile int result = checksum;
    
    printf("Result: %d\n", result);
    
    /* Additional compile-time tests */
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(5.9) == 5, "floor failed");
    static_assert(ceil(5.1) == 6, "ceil failed");
    static_assert(round(5.5) == 6, "round failed");
    
    return 0;
}
