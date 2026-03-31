/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent constant folding at parse time */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 10;

/* Complex types */
volatile double _Complex vcd = 3.0 + 4.0 * I;
volatile float _Complex vcf = 1.5f + 2.5f * I;

/* Test 1: Basic integer-valued real functions in constant contexts */
static int test_basic_functions(void) {
    /* These should be folded during constant folding pass */
    const double d1 = trunc(5.9);
    const double d2 = floor(5.9);
    const double d3 = ceil(5.1);
    const double d4 = round(5.5);
    const double d5 = nearbyint(5.3);
    const double d6 = rint(5.7);
    
    /* Use in array size (C99 VLA or C++ template would be better) */
    char arr1[(int)d1];
    char arr2[(int)d2];
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6) + sizeof(arr1) + sizeof(arr2);
}

/* Test 2: Builtin functions with long long return */
static int test_builtin_ll(void) {
    /* These builtins return long long but take real arguments */
    long long ll1 = __builtin_llround(3.14);
    long long ll2 = __builtin_llround(-3.14);
    long long ll3 = __builtin_llrint(2.718);
    long long ll4 = __builtin_llrint(-2.718);
    
    return (int)(ll1 + ll2 + ll3 + ll4);
}

/* Test 3: Complex part extraction */
static int test_complex_parts(void) {
    double _Complex cd = 3.5 + 4.5 * I;
    float _Complex cf = 1.25f + 2.75f * I;
    
    /* __real__ and __imag__ extract integer-valued real parts */
    double rd = __real__ cd;
    double id = __imag__ cd;
    float rf = __real__ cf;
    float imf = __imag__ cf;
    
    /* Use in expressions */
    double sum1 = rd + id;
    float sum2 = rf + imf;
    
    return (int)(sum1 + sum2);
}

/* Test 4: Nested calls */
static int test_nested_calls(void) {
    /* Nested integer-valued real calls */
    double d1 = floor(ceil(3.7));
    double d2 = trunc(round(4.2));
    double d3 = nearbyint(rint(5.8));
    double d4 = __builtin_llround(floor(6.3));
    
    /* Multiple levels of nesting */
    double d5 = trunc(floor(ceil(2.9)));
    
    return (int)(d1 + d2 + d3 + d4 + d5);
}

/* Test 5: Calls in conditional expressions */
static int test_conditional_calls(void) {
    int condition = vi > 5;
    
    /* Conditional operator with integer-valued real calls */
    double d1 = condition ? trunc(7.8) : floor(7.8);
    double d2 = (vi % 2) ? ceil(3.2) : round(3.2);
    
    /* Nested conditionals */
    double d3 = (vi > 0) ? 
                ((vi < 10) ? nearbyint(4.6) : rint(4.6)) :
                floor(4.6);
    
    return (int)(d1 + d2 + d3);
}

/* Test 6: Calls as function arguments */
static double helper1(double x) { return floor(x); }
static double helper2(double x) { return ceil(x); }

static int test_as_arguments(void) {
    /* Integer-valued calls as arguments to other calls */
    double d1 = trunc(helper1(5.7));
    double d2 = round(helper2(3.3));
    
    /* Multiple arguments */
    double d3 = fmax(floor(2.9), ceil(2.1));
    double d4 = fmin(trunc(3.8), round(3.8));
    
    return (int)(d1 + d2 + d3 + d4);
}

/* Test 7: In arithmetic expressions */
static int test_arithmetic_expressions(void) {
    /* Mixed with arithmetic */
    double d1 = trunc(5.9) * 2.0;
    double d2 = floor(4.7) + ceil(4.3);
    double d3 = (round(3.5) / floor(1.5)) - nearbyint(2.2);
    
    /* Complex expressions */
    double d4 = (__builtin_llround(3.14) % 5) + rint(2.8);
    
    return (int)(d1 + d2 + d3 + d4);
}

/* Test 8: With volatile arguments (prevents some folding) */
static int test_volatile_args(void) {
    /* Using volatile variables as arguments */
    double d1 = trunc(vd);
    double d2 = floor(vd + 1.0);
    double d3 = ceil(vf * 2.0f);
    double d4 = round((double)vi);
    
    return (int)(d1 + d2 + d3 + d4);
}

/* Test 9: In comparison expressions */
static int test_comparisons(void) {
    int result = 0;
    
    /* Comparisons that might be folded */
    if (trunc(5.9) > floor(5.9)) result += 1;
    if (ceil(4.1) >= round(4.1)) result += 2;
    if (nearbyint(3.7) == rint(3.7)) result += 4;
    if (__builtin_llround(2.5) <= 3LL) result += 8;
    
    /* Complex comparisons */
    result += (floor(2.9) < ceil(2.9)) ? 16 : 0;
    
    return result;
}

/* Test 10: Type casting scenarios */
static int test_type_casts(void) {
    /* Various casts of integer-valued real results */
    int i1 = (int)trunc(6.7);
    long l1 = (long)floor(8.9);
    float f1 = (float)ceil(3.2);
    double d1 = (double)__builtin_llround(4.5);
    
    /* Cast in expressions */
    double d2 = (double)((int)round(5.5));
    
    return i1 + (int)l1 + (int)f1 + (int)d1 + (int)d2;
}

/* C++ specific tests (compile with g++) */
#ifdef __cplusplus
#include <type_traits>

constexpr double cpp_trunc(double x) { return trunc(x); }
constexpr double cpp_floor(double x) { return floor(x); }

template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};

static int test_cpp_features(void) {
    /* constexpr functions */
    constexpr double d1 = cpp_trunc(7.8);
    constexpr double d2 = cpp_floor(7.8);
    
    /* Template arguments */
    constexpr int tval = TestTemplate<5>::value;
    
    /* static_assert */
    static_assert(trunc(5.9) == 5, "trunc should work at compile time");
    static_assert(floor(5.9) == 5, "floor should work at compile time");
    static_assert(ceil(5.1) == 6, "ceil should work at compile time");
    
    /* constexpr if (C++17) */
    if constexpr (round(3.5) == 4) {
        return (int)(d1 + d2 + tval + 100);
    }
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
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_as_arguments();
    checksum += test_arithmetic_expressions();
    checksum += test_volatile_args();
    checksum += test_comparisons();
    checksum += test_type_casts();
    
#ifdef __cplusplus
    checksum += test_cpp_features();
#endif
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    /* Additional compile-time tests (may appear in fold-const.cc) */
    enum { 
        E1 = (int)trunc(10.5),
        E2 = (int)floor(10.5),
        E3 = (int)ceil(10.5),
        E4 = (int)round(10.5)
    };
    
    /* Array size using integer-valued real function */
    char compile_time_array[(int)trunc(15.3)];
    
    return checksum % 256;
}
