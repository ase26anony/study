/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent constant folding at front-end level */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Complex types */
volatile double complex vcd = 1.5 + 2.5 * I;
volatile float complex vcf = 3.0f + 4.0f * I;

/* Global constants to force folding in static initializers */
const double cd = 7.89;
const float cf = 1.23f;

/* Test 1: Basic integer-valued real functions in constant contexts */
static int test_basic_folding(void) {
    /* These should be folded by fold-const.cc */
    double d1 = trunc(vd);
    double d2 = floor(vd + 0.5);
    double d3 = ceil(vf * 2.0f);
    double d4 = round(cd);
    double d5 = nearbyint(cf * 2.0f);
    double d6 = rint(vd - 0.5);
    
    /* Builtins with long long return */
    long long ll1 = __builtin_llround(vd);
    long long ll2 = __builtin_llrint(vd * 2.0);
    
    /* Complex part extraction */
    double re1 = __real__(vcd);
    double im1 = __imag__(vcd);
    float re2 = __real__(vcf);
    float im2 = __imag__(vcf);
    
    /* Combine results */
    int sum = (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6;
    sum += (int)ll1 + (int)ll2;
    sum += (int)re1 + (int)im1 + (int)re2 + (int)im2;
    
    return sum % 1000;
}

/* Test 2: Nested calls to exercise recursive depth */
static int test_nested_calls(void) {
    double d1 = floor(ceil(vd));
    double d2 = trunc(round(vd * 2.0));
    double d3 = nearbyint(rint(vf));
    double d4 = __builtin_llround(__builtin_llrint(vd));
    
    /* Multi-level nesting */
    double d5 = floor(ceil(trunc(round(vd))));
    
    /* Complex nesting with different types */
    float f1 = __real__(vcf) + __imag__(vcf);
    double d6 = trunc(floor(f1));
    
    int sum = (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6;
    return sum % 1000;
}

/* Test 3: Calls in conditional expressions */
static int test_conditional_calls(void) {
    int result = 0;
    
    /* Conditional operator with integer-valued calls */
    double d1 = (vi > 0) ? trunc(vd) : floor(vd);
    double d2 = (vf < 5.0f) ? ceil(vf) : round(vf);
    
    /* Nested conditionals */
    double d3 = (vi % 2) ? 
                ((vd > 0) ? nearbyint(vd) : rint(vd)) :
                __builtin_llround(vd);
    
    /* Conditional with complex extraction */
    double d4 = (vi < 100) ? __real__(vcd) : __imag__(vcd);
    
    result = (int)d1 + (int)d2 + (int)d3 + (int)d4;
    return result % 1000;
}

/* Test 4: Calls as function arguments */
static int test_argument_calls(void) {
    /* Integer-valued calls as arguments to other integer-valued calls */
    double d1 = trunc(floor(vd));
    double d2 = round(ceil(vf));
    double d3 = nearbyint(rint(cd));
    
    /* Mixed argument types */
    long long ll1 = __builtin_llround(trunc(vd));
    double d4 = __builtin_llrint(__builtin_llround(vd));
    
    int sum = (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)ll1;
    return sum % 1000;
}

/* Test 5: Compile-time constant contexts (C++ style in C) */
#ifdef __cplusplus
constexpr double cpp_test() {
    return trunc(5.9) + floor(4.7) + ceil(3.2) + round(2.5);
}

template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};
#endif

/* Using static assertions and array sizes */
static int test_constant_contexts(void) {
    /* These force folding during compilation */
    static char buffer1[(int)trunc(10.5)];
    static char buffer2[(int)floor(20.7)];
    
    /* Enum with folded values */
    enum {
        VAL1 = (int)ceil(15.1),
        VAL2 = (int)round(25.6),
        VAL3 = (int)nearbyint(30.4)
    };
    
    /* Static assertions (C11/C++11) */
    _Static_assert(trunc(5.9) == 5, "trunc folding failed");
    _Static_assert(floor(4.7) == 4, "floor folding failed");
    _Static_assert(ceil(3.2) == 4, "ceil folding failed");
    _Static_assert(round(2.5) == 3, "round folding failed");
    
    int sum = sizeof(buffer1) + sizeof(buffer2) + VAL1 + VAL2 + VAL3;
    
    /* Global static initializer */
    static double global_val = trunc(cd) + floor(cf);
    sum += (int)global_val;
    
    return sum % 1000;
}

/* Test 6: Mixed expressions with arithmetic */
static int test_mixed_expressions(void) {
    double d1 = (trunc(vd) * 2.0) / floor(vd + 1.0);
    double d2 = ceil(vf) + round(vf) - nearbyint(vf);
    
    /* Comparison expressions */
    int cmp1 = (ceil(vd) > floor(vd)) ? 1 : 0;
    int cmp2 = (trunc(cd) == (int)cd) ? 1 : 0;
    
    /* Type casts */
    int i1 = (int)rint(vd);
    int i2 = (int)__builtin_llround(vd * 2.0);
    
    /* Complex arithmetic */
    double d3 = __real__(vcd) * 2.0 + __imag__(vcd);
    
    int sum = (int)d1 + (int)d2 + cmp1 + cmp2 + i1 + i2 + (int)d3;
    return sum % 1000;
}

/* Test 7: Edge cases and special values */
static int test_edge_cases(void) {
    /* Exact integers */
    double d1 = floor(4.0);
    double d2 = ceil(-3.0);
    double d3 = trunc(0.0);
    
    /* Large values */
    double d4 = round(1e10 + 0.5);
    double d5 = nearbyint(-1e10 - 0.3);
    
    /* Negative fractional values */
    double d6 = floor(-4.7);
    double d7 = ceil(-2.3);
    double d8 = round(-1.5);
    
    /* Integer arguments */
    double d9 = trunc(vi);
    double d10 = floor(vi * 2);
    
    int sum = (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
              (int)d6 + (int)d7 + (int)d8 + (int)d9 + (int)d10;
    return sum % 1000;
}

/* Test 8: Loop with compile-time bounds using integer-valued calls */
static int test_loop_bounds(void) {
    int sum = 0;
    
    /* Loop bound determined by folded expression */
    int limit = (int)trunc(vd * 10.0);
    if (limit > 100) limit = 100;
    if (limit < 0) limit = 0;
    
    for (int i = 0; i < limit; i++) {
        /* Use integer-valued calls in loop body */
        double d = floor(i * 0.5);
        sum += (int)d;
    }
    
    /* Another loop with different bound */
    int limit2 = (int)ceil(vf * 5.0f);
    for (int i = 0; i < limit2 && i < 50; i++) {
        double d = round(i * 1.7);
        sum += (int)d;
    }
    
    return sum % 1000;
}

/* Main driver */
int main(void) {
    int checksum = 0;
    
    checksum += test_basic_folding();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_argument_calls();
    checksum += test_constant_contexts();
    checksum += test_mixed_expressions();
    checksum += test_edge_cases();
    checksum += test_loop_bounds();
    
    printf("Result: %d\n", checksum);
    
    /* Verify with runtime computation */
    double verify = trunc(3.14) + floor(2.71) + ceil(1.41) + round(1.62);
    printf("Verify: %f\n", verify);
    
    return 0;
}
