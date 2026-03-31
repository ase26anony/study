/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Global to accumulate results */
static int checksum = 0;

/* Test 1: Basic integer-valued real functions in constant contexts */
void test_basic_functions(void) {
    /* These should be folded by fold-const.cc */
    const double d1 = trunc(5.9);
    const double d2 = floor(4.7);
    const double d3 = ceil(3.2);
    const double d4 = round(6.5);
    const double d5 = nearbyint(2.3);
    const double d6 = rint(1.8);
    
    checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6;
}

/* Test 2: Builtin functions with integer return types */
void test_builtin_functions(void) {
    /* __builtin_llround and __builtin_llrint return long long */
    const long long ll1 = __builtin_llround(3.14);
    const long long ll2 = __builtin_llround(2.71);
    const long long ll3 = __builtin_llrint(1.618);
    const long long ll4 = __builtin_llrint(0.577);
    
    checksum += (int)(ll1 + ll2 + ll3 + ll4);
}

/* Test 3: Complex number real/imag parts */
void test_complex_parts(void) {
    /* Complex integer types with __real__ and __imag__ */
    complex int ci = 3 + 4 * I;
    complex long cl = 5L + 6L * I;
    
    const int real_ci = __real__ ci;
    const int imag_ci = __imag__ ci;
    const long real_cl = __real__ cl;
    const long imag_cl = __imag__ cl;
    
    checksum += real_ci + imag_ci + (int)real_cl + (int)imag_cl;
}

/* Test 4: Nested calls to exercise recursive depth */
void test_nested_calls(void) {
    /* Nested integer-valued real calls */
    const double d1 = floor(ceil(2.3));
    const double d2 = trunc(round(4.7));
    const double d3 = nearbyint(rint(1.5));
    const double d4 = ceil(floor(trunc(3.8)));
    
    checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4;
}

/* Test 5: Calls within conditional expressions */
void test_conditional_calls(void) {
    /* Conditional operator with integer-valued real calls */
    const int cond = vi > 0;
    const double d1 = cond ? trunc(5.9) : floor(4.7);
    const double d2 = (vi < 100) ? ceil(3.2) : round(6.5);
    const double d3 = (vd > 0) ? nearbyint(2.3) : rint(1.8);
    
    checksum += (int)d1 + (int)d2 + (int)d3;
}

/* Test 6: Calls as function arguments */
void test_call_arguments(void) {
    /* Integer-valued calls as arguments to other calls */
    const double d1 = trunc(round(4.7));
    const double d2 = floor(ceil(trunc(3.8)));
    const double d3 = nearbyint(floor(2.9));
    
    checksum += (int)d1 + (int)d2 + (int)d3;
}

/* Test 7: Mixed argument types */
void test_mixed_arguments(void) {
    /* Various argument types and values */
    const double d1 = floor(5);          /* Integer argument */
    const double d2 = ceil(4.0);         /* Exact integer real */
    const double d3 = trunc(4.7);        /* Fractional real */
    const double d4 = round(-2.3);       /* Negative value */
    const double d5 = nearbyint(1e10);   /* Large value */
    
    checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
}

/* Test 8: In arithmetic expressions */
void test_arithmetic_expressions(void) {
    /* Integer-valued calls in larger expressions */
    const int val1 = (int)(trunc(5.9) * 2) / (int)floor(2.5);
    const int val2 = (int)(ceil(3.2) + round(1.5)) * 3;
    const double val3 = (rint(4.8) - nearbyint(2.1)) * 2.0;
    
    checksum += val1 + val2 + (int)val3;
}

/* Test 9: In comparisons */
void test_comparisons(void) {
    /* Comparisons that should be folded */
    const int cmp1 = (ceil(3.2) > floor(2.8));
    const int cmp2 = (trunc(5.9) == 5);
    const int cmp3 = (round(6.5) != 7);
    const int cmp4 = (nearbyint(1.3) <= rint(1.8));
    
    checksum += cmp1 + cmp2 + cmp3 + cmp4;
}

/* Test 10: In array sizes (compile-time constants) */
void test_array_sizes(void) {
    /* Use in array sizes - requires constant folding */
    char buffer1[(int)floor(10.5)];
    char buffer2[(int)ceil(7.2)];
    char buffer3[(int)trunc(8.9)];
    
    checksum += sizeof(buffer1) + sizeof(buffer2) + sizeof(buffer3);
}

/* Test 11: Static assertions */
void test_static_assertions(void) {
    /* These should be evaluated at compile time */
    _Static_assert(trunc(5.9) == 5, "trunc should work");
    _Static_assert(floor(4.7) == 4, "floor should work");
    _Static_assert(ceil(3.2) == 4, "ceil should work");
    _Static_assert(round(6.5) == 7, "round should work");
    
    checksum += 4; /* All assertions passed */
}

/* Test 12: Global initializers */
static const double g1 = round(3.14);
static const double g2 = floor(ceil(2.71));
static const float g3 = nearbyint(1.618f);
static const long long g4 = __builtin_llround(0.577);

void test_global_init(void) {
    checksum += (int)g1 + (int)g2 + (int)g3 + (int)g4;
}

/* C++ specific tests (if compiled as C++) */
#ifdef __cplusplus
#include <type_traits>

constexpr double cpp_trunc(double x) { return trunc(x); }
constexpr double cpp_floor(double x) { return floor(x); }
constexpr double cpp_ceil(double x) { return ceil(x); }

template<int N>
struct TestTemplate {
    static const int value = (int)floor(N * 1.5);
};

void test_cpp_features(void) {
    /* constexpr functions */
    constexpr double d1 = cpp_trunc(5.9);
    constexpr double d2 = cpp_floor(4.7);
    constexpr double d3 = cpp_ceil(3.2);
    
    /* Template arguments */
    const int val1 = TestTemplate<5>::value;
    const int val2 = TestTemplate<10>::value;
    
    /* static_assert in C++ */
    static_assert(cpp_trunc(5.9) == 5, "");
    static_assert(cpp_floor(4.7) == 4, "");
    static_assert(cpp_ceil(3.2) == 4, "");
    
    checksum += (int)d1 + (int)d2 + (int)d3 + val1 + val2;
}
#endif

/* Test 13: Mixed with volatile to prevent full pre-evaluation */
void test_volatile_mix(void) {
    /* Use volatile in expressions to ensure fold pass sees them */
    const double d1 = trunc(vd);
    const double d2 = floor(vd + 1.0);
    const double d3 = ceil(vf * 2.0f);
    const double d4 = round((double)vi / 3.0);
    
    checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4;
}

/* Test 14: Zero, one, and two argument calls */
void test_argument_counts(void) {
    /* Test various argument counts */
    const double d1 = trunc(1.5);      /* 1 argument */
    const double d2 = pow(2.0, 3.0);   /* 2 arguments (not integer-valued, but tests call_expr_nargs) */
    
    /* Some builtins might have optional arguments */
    checksum += (int)d1 + (int)d2;
}

/* Main driver */
int main(void) {
    /* Run all tests */
    test_basic_functions();
    test_builtin_functions();
    test_complex_parts();
    test_nested_calls();
    test_conditional_calls();
    test_call_arguments();
    test_mixed_arguments();
    test_arithmetic_expressions();
    test_comparisons();
    test_array_sizes();
    test_static_assertions();
    test_global_init();
    test_volatile_mix();
    test_argument_counts();
    
#ifdef __cplusplus
    test_cpp_features();
#endif
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    return 0;
}
