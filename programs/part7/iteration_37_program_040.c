/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 10;

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

/* Test 2: Builtins with long long return */
void test_builtin_ll(void) {
    /* These use __builtin_llround, __builtin_llrint */
    long long ll1 = __builtin_llround(9.7);
    long long ll2 = __builtin_llrint(8.4);
    
    checksum += (int)(ll1 % 100) + (int)(ll2 % 100);
}

/* Test 3: Complex number real/imag parts */
void test_complex_parts(void) {
    /* Complex integer types with __real__ and __imag__ */
    _Complex int ci = 3 + 4 * I;
    double cr = __real__(ci);
    double ci_imag = __imag__(ci);
    
    checksum += (int)cr + (int)ci_imag;
}

/* Test 4: Nested calls (recursive depth) */
void test_nested_calls(void) {
    /* Nested integer-valued real calls */
    double d1 = floor(ceil(3.7));
    double d2 = trunc(round(4.2));
    double d3 = nearbyint(rint(5.9));
    
    checksum += (int)d1 + (int)d2 + (int)d3;
}

/* Test 5: Calls within conditional expressions */
void test_conditional_calls(void) {
    /* Conditional operator with integer-valued calls */
    double d1 = (vi > 5) ? trunc(vd) : floor(vf);
    double d2 = (vd < 4.0) ? ceil(2.3) : round(3.8);
    
    checksum += (int)d1 + (int)d2;
}

/* Test 6: Calls as function arguments */
void test_call_arguments(void) {
    /* Integer-valued calls as arguments to other calls */
    double d1 = round(trunc(7.8));
    double d2 = floor(ceil(6.1));
    
    checksum += (int)d1 + (int)d2;
}

/* Test 7: Mixed arithmetic with folded results */
void test_mixed_arithmetic(void) {
    /* Arithmetic expressions with integer-valued calls */
    double d1 = (trunc(8.9) * 2) / floor(4.5);
    double d2 = ceil(3.2) + round(2.7) - nearbyint(1.5);
    
    checksum += (int)d1 + (int)d2;
}

/* Test 8: Comparisons with folded calls */
void test_comparisons(void) {
    /* Comparisons that might be folded */
    int cmp1 = (ceil(4.1) > floor(3.9));
    int cmp2 = (trunc(5.5) == round(5.5));
    int cmp3 = (rint(2.1) <= nearbyint(2.1));
    
    checksum += cmp1 + cmp2 + cmp3;
}

/* Test 9: Type casts of folded results */
void test_type_casts(void) {
    /* Explicit casts of integer-valued real results */
    int i1 = (int)trunc(9.99);
    int i2 = (int)floor(8.01);
    int i3 = (int)ceil(7.0);
    
    checksum += i1 + i2 + i3;
}

/* Test 10: Array sizes using folded results (C++ style) */
#ifdef __cplusplus
template<int N>
struct ArrayTest {
    char data[N];
    static int get_size() { return N; }
};

void test_array_sizes() {
    /* Use integer-valued calls in template arguments */
    constexpr int sz1 = static_cast<int>(trunc(10.5));
    constexpr int sz2 = static_cast<int>(floor(9.9));
    
    ArrayTest<sz1> a1;
    ArrayTest<sz2> a2;
    
    checksum += a1.get_size() + a2.get_size();
}
#endif

/* Test 11: Static assertions with folded calls */
void test_static_asserts(void) {
    /* These should be evaluated during compilation */
    #define STATIC_ASSERT(cond) typedef char static_assert_##__LINE__[(cond)?1:-1]
    
    STATIC_ASSERT(trunc(5.9) == 5);
    STATIC_ASSERT(floor(4.7) == 4);
    STATIC_ASSERT(ceil(3.2) == 4);
    STATIC_ASSERT(round(6.5) == 7);
}

/* Test 12: Global initializers with folded calls */
static const double g1 = trunc(15.7);
static const double g2 = floor(12.3);
static const double g3 = ceil(18.9);
static const double g4 = round(20.5);

void test_global_init(void) {
    checksum += (int)g1 + (int)g2 + (int)g3 + (int)g4;
}

/* Test 13: Negative values and edge cases */
void test_edge_cases(void) {
    double d1 = trunc(-3.7);
    double d2 = floor(-2.3);
    double d3 = ceil(-1.8);
    double d4 = round(-4.5);
    double d5 = nearbyint(-0.5);
    
    checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
}

/* Test 14: Exact integer arguments */
void test_exact_integers(void) {
    double d1 = floor(5.0);
    double d2 = ceil(2.0);
    double d3 = trunc(7.0);
    double d4 = round(3.0);
    
    checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4;
}

/* Test 15: Large values */
void test_large_values(void) {
    double d1 = trunc(1e10 + 0.7);
    double d2 = floor(1e9 + 0.3);
    double d3 = ceil(1e8 + 0.1);
    
    checksum += (int)(d1 / 1e9) + (int)(d2 / 1e8) + (int)(d3 / 1e7);
}

/* Test 16: Zero and one argument calls (if supported) */
void test_arg_counts(void) {
    /* Some builtins might have optional arguments */
    double d1 = __builtin_rint(5.5);
    double d2 = __builtin_round(3.3);
    
    checksum += (int)d1 + (int)d2;
}

/* Test 17: In if conditions and loop bounds */
void test_control_flow(void) {
    int count = 0;
    
    /* if with folded call */
    if (trunc(4.8) > 4) {
        count++;
    }
    
    /* loop bound with folded call */
    int limit = (int)floor(5.2);
    for (int i = 0; i < limit; i++) {
        count++;
    }
    
    checksum += count;
}

/* Test 18: constexpr functions (C++) */
#ifdef __cplusplus
constexpr double cexpr_func(double x) {
    return floor(x) + ceil(x * 2);
}

void test_constexpr_func() {
    constexpr double r1 = cexpr_func(3.3);
    constexpr double r2 = cexpr_func(4.7);
    
    checksum += (int)r1 + (int)r2;
}
#endif

/* Main driver */
int main(void) {
    /* Run all tests */
    test_basic_functions();
    test_builtin_ll();
    test_complex_parts();
    test_nested_calls();
    test_conditional_calls();
    test_call_arguments();
    test_mixed_arithmetic();
    test_comparisons();
    test_type_casts();
    
    #ifdef __cplusplus
    test_array_sizes();
    test_constexpr_func();
    #endif
    
    test_static_asserts();
    test_global_init();
    test_edge_cases();
    test_exact_integers();
    test_large_values();
    test_arg_counts();
    test_control_flow();
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
