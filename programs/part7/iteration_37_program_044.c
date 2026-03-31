/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Complex types */
volatile double _Complex vcd = 3.0 + 4.0 * I;
volatile float _Complex vcf = 1.5f + 2.5f * I;

/* Test 1: Basic integer-valued real functions in constant contexts */
static int test_basic_functions(void) {
    /* These should be folded by fold-const.cc */
    const double d1 = trunc(5.9);
    const double d2 = floor(4.7);
    const double d3 = ceil(3.2);
    const double d4 = round(6.5);
    const double d5 = nearbyint(2.3);
    const double d6 = rint(7.8);
    
    /* Use in array size (C99 VLA or C++ template would be better) */
    char buf1[(int)d1];
    char buf2[(int)d2];
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6);
}

/* Test 2: Builtin functions with integer return types */
static long long test_builtin_ll(void) {
    /* These return long long but take real arguments */
    long long ll1 = __builtin_llround(9.7);
    long long ll2 = __builtin_llrint(8.4);
    
    /* Mix with volatile to prevent early folding */
    long long ll3 = __builtin_llround(vd);
    long long ll4 = __builtin_llrint(vf);
    
    return ll1 + ll2 + ll3 + ll4;
}

/* Test 3: Complex number real/imag parts */
static int test_complex_parts(void) {
    double _Complex cd = 3.0 + 4.0 * I;
    float _Complex cf = 1.5f + 2.5f * I;
    
    /* __real__ and __imag__ on complex integer types */
    double r1 = __real__(cd);
    double i1 = __imag__(cd);
    float r2 = __real__(cf);
    float i2 = __imag__(cf);
    
    /* With volatile complex */
    double r3 = __real__(vcd);
    double i3 = __imag__(vcd);
    float r4 = __real__(vcf);
    float i4 = __imag__(vcf);
    
    return (int)(r1 + i1 + r2 + i2 + r3 + i3 + r4 + i4);
}

/* Test 4: Nested integer-valued real calls */
static double test_nested_calls(void) {
    /* Nested calls to trigger recursive integer_valued_real_p */
    double d1 = floor(ceil(5.3));
    double d2 = trunc(round(7.6));
    double d3 = nearbyint(rint(3.9));
    double d4 = ceil(floor(trunc(8.7)));
    
    /* More complex nesting */
    double d5 = round(trunc(floor(4.2)));
    double d6 = rint(ceil(nearbyint(6.1)));
    
    return d1 + d2 + d3 + d4 + d5 + d6;
}

/* Test 5: Calls in conditional expressions */
static double test_conditional_calls(void) {
    int flag = vi > 0;
    
    /* Conditional operator with integer-valued real calls */
    double d1 = flag ? trunc(5.9) : floor(4.7);
    double d2 = (vd > 0) ? ceil(3.2) : round(6.5);
    
    /* Nested conditionals */
    double d3 = (flag && vd > 0) ? nearbyint(2.3) : rint(7.8);
    
    /* Conditional as argument to another call */
    double d4 = floor(flag ? 8.4 : 9.1);
    double d5 = round((vf > 0) ? 1.5 : 2.5);
    
    return d1 + d2 + d3 + d4 + d5;
}

/* Test 6: Calls as arguments to other integer-valued functions */
static double test_argument_calls(void) {
    /* Calls as arguments - should extract arg0 and arg1 properly */
    double d1 = round(trunc(5.9));
    double d2 = floor(ceil(round(4.7)));
    double d3 = nearbyint(floor(3.2));
    double d4 = rint(ceil(6.5));
    
    /* Multiple levels */
    double d5 = trunc(round(floor(ceil(2.3))));
    
    return d1 + d2 + d3 + d4 + d5;
}

/* Test 7: Mixed with arithmetic operations */
static double test_mixed_arithmetic(void) {
    /* Integer-valued real calls in arithmetic expressions */
    double d1 = trunc(5.9) * 2.0;
    double d2 = floor(4.7) / ceil(2.1);
    double d3 = (round(6.5) + nearbyint(3.2)) * rint(1.8);
    
    /* More complex expressions */
    double d4 = (trunc(vd) * floor(vf)) / ceil(2.0);
    double d5 = round(3.7) > floor(2.9) ? trunc(4.1) : nearbyint(5.6);
    
    return d1 + d2 + d3 + d4 + d5;
}

/* Test 8: Different argument types and values */
static double test_various_arguments(void) {
    /* Integer arguments */
    double d1 = floor(5);
    double d2 = trunc(2);
    
    /* Exact integer real arguments */
    double d3 = ceil(4.0);
    double d4 = round(6.0);
    
    /* Fractional arguments */
    double d5 = floor(4.7);
    double d6 = trunc(3.14159);
    
    /* Negative values */
    double d7 = round(-2.3);
    double d8 = ceil(-3.7);
    double d9 = floor(-4.2);
    
    /* Large values */
    double d10 = trunc(1e10 + 0.7);
    double d11 = floor(1e15 - 0.3);
    
    return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 + d11;
}

/* Test 9: In constant expressions (compile-time evaluation) */
#ifdef __cplusplus
template<int N>
struct TestTemplate {
    enum { value = N };
};

constexpr double constexpr_test() {
    return trunc(5.9) + floor(4.7) + ceil(3.2);
}
#endif

static int test_constant_contexts(void) {
    int result = 0;
    
    /* Use in enum (C) or constexpr (C++) */
    enum { 
        VAL1 = (int)trunc(5.9),
        VAL2 = (int)floor(4.7),
        VAL3 = (int)ceil(3.2)
    };
    
    result += VAL1 + VAL2 + VAL3;
    
    /* Use in array size (GCC extension for non-VLA) */
    char buffer1[(int)floor(10.5)];
    char buffer2[(int)trunc(20.3)];
    
    result += sizeof(buffer1) + sizeof(buffer2);
    
#ifdef __cplusplus
    /* C++ specific constant evaluation */
    constexpr double ce = constexpr_test();
    result += (int)ce;
    
    /* Template argument */
    TestTemplate<(int)round(15.7)> t;
    result += t.value;
#endif
    
    return result;
}

/* Test 10: Real/imag with integer complex */
static int test_integer_complex(void) {
    /* Complex integer types */
    int _Complex ci = 3 + 4 * I;
    long _Complex cl = 5L + 6L * I;
    
    /* __real__ and __imag__ on integer complex types */
    int r1 = __real__(ci);
    int i1 = __imag__(ci);
    long r2 = __real__(cl);
    long i2 = __imag__(cl);
    
    /* These should be recognized as integer-valued */
    double d1 = __real__(ci);  /* real part of int complex -> int */
    double d2 = __imag__(cl);  /* imag part of long complex -> long */
    
    return (int)(r1 + i1 + r2 + i2 + d1 + d2);
}

/* Main driver */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_functions();
    checksum += (int)test_builtin_ll();
    checksum += test_complex_parts();
    checksum += (int)test_nested_calls();
    checksum += (int)test_conditional_calls();
    checksum += (int)test_argument_calls();
    checksum += (int)test_mixed_arithmetic();
    checksum += (int)test_various_arguments();
    checksum += test_constant_contexts();
    checksum += test_integer_complex();
    
    printf("Result: %d\n", checksum);
    
    /* Additional static asserts for compile-time testing */
    _Static_assert(trunc(5.9) == 5, "trunc should work at compile time");
    _Static_assert(floor(4.7) == 4, "floor should work at compile time");
    _Static_assert(ceil(3.2) == 4, "ceil should work at compile time");
    _Static_assert(round(6.5) == 7, "round should work at compile time");
    
    return 0;
}
