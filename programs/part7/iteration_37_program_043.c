/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 10;

/* Global variables to force folding in initializers */
static const double cd = 4.5;
static const float cf = -2.3f;
static const int ci = 7;

/* Test 1: Basic integer-valued real functions in constant contexts */
int test_basic_functions(void) {
    /* These should be folded by fold-const.cc */
    const double t1 = trunc(cd);        /* 4.0 */
    const double t2 = floor(cd);        /* 4.0 */
    const double t3 = ceil(cd);         /* 5.0 */
    const double t4 = round(cd);        /* 5.0 */
    const double t5 = nearbyint(cd);    /* 4.0 */
    const double t6 = rint(cd);         /* 4.0 */
    
    /* Use in array size (C99 VLA or C++ array) */
    char buffer1[(int)trunc(10.5)];     /* size 10 */
    char buffer2[(int)floor(10.5)];     /* size 10 */
    
    /* Static assertions (compile-time checks) */
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(5.9) == 5, "floor failed");
    static_assert(ceil(5.1) == 6, "ceil failed");
    static_assert(round(5.5) == 6, "round failed");
    
    return (int)(t1 + t2 + t3 + t4 + t5 + t6);
}

/* Test 2: Builtin functions with long long return */
long long test_builtin_ll(void) {
    /* These builtins return long long but take real arguments */
    long long r1 = __builtin_llround(cd);      /* 5 */
    long long r2 = __builtin_llrint(cd);       /* 4 */
    long long r3 = __builtin_llroundf(cf);     /* -2 */
    long long r4 = __builtin_llrintf(cf);      /* -2 */
    
    /* Use in constant expression */
    enum { E1 = __builtin_llround(3.14) };     /* 3 */
    enum { E2 = __builtin_llrint(3.14) };      /* 3 */
    
    return r1 + r2 + r3 + r4 + E1 + E2;
}

/* Test 3: Complex number real/imag parts */
double test_complex_parts(void) {
    /* Complex integer types */
    const complex int ci1 = 3 + 4 * I;
    const complex long cl1 = 5L + 6L * I;
    
    /* Extract real and imag parts (integer-valued real operations) */
    double r1 = __real__ ci1;    /* 3.0 */
    double r2 = __imag__ ci1;    /* 4.0 */
    double r3 = __real__ cl1;    /* 5.0 */
    double r4 = __imag__ cl1;    /* 6.0 */
    
    /* Complex float/double with integer values */
    const complex double cd1 = 7.0 + 8.0 * I;
    double r5 = __real__ cd1;    /* 7.0 */
    double r6 = __imag__ cd1;    /* 8.0 */
    
    return r1 + r2 + r3 + r4 + r5 + r6;
}

/* Test 4: Nested calls and conditional expressions */
double test_nested_conditional(void) {
    /* Nested integer-valued calls */
    const double n1 = floor(ceil(cd));          /* floor(5.0) = 5.0 */
    const double n2 = trunc(round(cf));         /* trunc(-2.0) = -2.0 */
    const double n3 = nearbyint(rint(2.7));     /* nearbyint(3.0) = 3.0 */
    
    /* Conditional operator with integer-valued calls */
    const double c1 = (ci > 5) ? trunc(3.14) : floor(2.71);
    const double c2 = (cd > 0) ? ceil(cd) : floor(cd);
    
    /* Nested conditionals */
    const double c3 = (ci % 2) ? 
                     (cd > 0 ? round(cd) : trunc(cd)) : 
                     (cf < 0 ? floor(cf) : ceil(cf));
    
    return n1 + n2 + n3 + c1 + c2 + c3;
}

/* Test 5: Calls with different argument counts */
double test_various_argument_counts(void) {
    /* Most standard functions take 1 argument */
    double r1 = trunc(cd);      /* 1 arg */
    
    /* Some builtins might have optional arguments */
    /* Using fmax/fmin which take 2 args and are integer-valued for integer inputs */
    double r2 = fmax(trunc(3.2), floor(4.8));    /* fmax(3.0, 4.0) = 4.0 */
    double r3 = fmin(ceil(2.3), round(3.6));     /* fmin(3.0, 4.0) = 3.0 */
    
    /* Nested with different arg counts */
    double r4 = trunc(fmax(2.5, 3.5));           /* trunc(3.5) = 3.0 */
    
    return r1 + r2 + r3 + r4;
}

/* Test 6: Integer-valued calls in arithmetic expressions */
double test_arithmetic_expressions(void) {
    /* Mixed arithmetic with integer-valued calls */
    const double expr1 = (trunc(cd) * 2.0) / floor(cd + 1.0);
    const double expr2 = ceil(cf) + round(cd) - nearbyint(cf * 2.0);
    const double expr3 = rint(cd * 2.0) * trunc(cf / 2.0);
    
    /* Comparisons that should fold */
    const int cmp1 = (ceil(cd) > floor(cd + 0.5));
    const int cmp2 = (trunc(cf) == floor(cf));
    const int cmp3 = (round(cd) != nearbyint(cd));
    
    return expr1 + expr2 + expr3 + cmp1 + cmp2 + cmp3;
}

/* Test 7: Template and constexpr contexts (C++) */
#ifdef __cplusplus
template<typename T>
constexpr T template_test(T x) {
    return floor(x) + ceil(x) + trunc(x);
}

constexpr double constexpr_test(double x) {
    return round(x) * nearbyint(x) - rint(x);
}

int test_cpp_features(void) {
    constexpr double ct1 = template_test(2.7);      /* 2 + 3 + 2 = 7 */
    constexpr double ct2 = constexpr_test(3.2);     /* 3 * 3 - 3 = 6 */
    
    /* Use in template parameter */
    struct TestStruct<size_t N> {
        char data[N];
    };
    
    TestStruct<(size_t)trunc(10.5)> ts1;    /* N = 10 */
    TestStruct<(size_t)ceil(10.1)> ts2;     /* N = 11 */
    
    return (int)(ct1 + ct2);
}
#endif

/* Test 8: Large and edge case values */
double test_edge_cases(void) {
    /* Large values */
    const double large = 1e15;
    double r1 = trunc(large + 0.7);      /* 1e15 */
    double r2 = floor(large - 0.3);      /* 1e15 - 1 */
    double r3 = ceil(large + 0.001);     /* 1e15 + 1 */
    
    /* Exact integers */
    double r4 = round(100.0);            /* 100.0 */
    double r5 = nearbyint(-50.0);        /* -50.0 */
    double r6 = rint(0.0);               /* 0.0 */
    
    /* Negative fractional values */
    double r7 = trunc(-3.7);             /* -3.0 */
    double r8 = floor(-3.7);             /* -4.0 */
    double r9 = ceil(-3.7);              /* -3.0 */
    double r10 = round(-3.7);            /* -4.0 */
    
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
}

/* Test 9: Mixed volatile and const arguments */
double test_mixed_arguments(void) {
    /* Mix volatile and const to prevent full compile-time evaluation */
    double r1 = trunc(vd);               /* volatile argument */
    double r2 = floor((double)vi);       /* volatile cast */
    double r3 = ceil(vf * cf);           /* mixed volatile/const */
    
    /* Use results in non-const context */
    volatile double result = r1 + r2 + r3;
    
    return result;
}

/* Main driver that exercises all tests */
int main(void) {
    int checksum = 0;
    
    /* Run all tests and accumulate results */
    checksum += test_basic_functions();
    checksum += (int)test_builtin_ll();
    checksum += (int)test_complex_parts();
    checksum += (int)test_nested_conditional();
    checksum += (int)test_various_argument_counts();
    checksum += (int)test_arithmetic_expressions();
    
    #ifdef __cplusplus
    checksum += test_cpp_features();
    #endif
    
    checksum += (int)test_edge_cases();
    checksum += (int)test_mixed_arguments();
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    /* Additional compile-time tests */
    static_assert(__builtin_llround(2.5) == 3, "llround failed");
    static_assert(__real__(3 + 4i) == 3, "__real__ failed");
    
    return 0;
}
