/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 10;

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
    const double d6 = rint(1.8);
    
    /* Use in static assertions (compile-time evaluation) */
    static_assert(trunc(5.9) == 5, "trunc should work at compile time");
    static_assert(floor(4.7) == 4, "floor should work at compile time");
    static_assert(ceil(3.2) == 4, "ceil should work at compile time");
    
    /* Array sizes using integer-valued real functions */
    char buffer1[(int)floor(10.5)];
    char buffer2[(int)ceil(9.1)];
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6) + 
           sizeof(buffer1) + sizeof(buffer2);
}

/* Test 2: Builtin functions with long long return */
static int test_builtin_ll_functions(void) {
    /* __builtin_llround and __builtin_llrint take real args, return long long */
    long long ll1 = __builtin_llround(3.14);
    long long ll2 = __builtin_llround(2.718);
    long long ll3 = __builtin_llrint(5.9);
    long long ll4 = __builtin_llrint(1.2);
    
    /* Use in enum to force constant folding */
    enum { 
        VAL1 = (int)__builtin_llround(10.5),
        VAL2 = (int)__builtin_llrint(20.3)
    };
    
    return (int)(ll1 + ll2 + ll3 + ll4) + VAL1 + VAL2;
}

/* Test 3: Complex number real/imag parts */
static int test_complex_parts(void) {
    /* __real__ and __imag__ on complex integer types */
    double _Complex cd = 3.0 + 4.0 * I;
    float _Complex cf = 1.5f + 2.5f * I;
    
    /* These extract real/imag parts as integer-valued real expressions */
    double re1 = __real__ cd;
    double im1 = __imag__ cd;
    float re2 = __real__ cf;
    float im2 = __imag__ cf;
    
    /* With volatile to prevent front-end folding */
    double re3 = __real__ vcd;
    double im3 = __imag__ vcd;
    float re4 = __real__ vcf;
    float im4 = __imag__ vcf;
    
    return (int)(re1 + im1 + re2 + im2 + re3 + im3 + re4 + im4);
}

/* Test 4: Nested integer-valued real calls */
static int test_nested_calls(void) {
    /* Nested calls to trigger recursive integer_valued_real_p */
    double d1 = floor(ceil(3.7));      /* ceil(3.7)=4, floor(4)=4 */
    double d2 = trunc(round(2.3));     /* round(2.3)=2, trunc(2)=2 */
    double d3 = nearbyint(rint(5.9));  /* rint(5.9)=6, nearbyint(6)=6 */
    
    /* Multiple levels of nesting */
    double d4 = floor(ceil(trunc(4.8 * 2.0) / 3.0));
    
    /* Calls as arguments to other calls */
    double d5 = round(trunc(vd) + floor(vf));
    
    return (int)(d1 + d2 + d3 + d4 + d5);
}

/* Test 5: Conditional expressions with integer-valued calls */
static int test_conditional_calls(void) {
    /* Conditional operator with integer-valued real calls in branches */
    double d1 = (vi > 5) ? trunc(3.14) : floor(2.718);
    double d2 = (vd < 4.0) ? ceil(vd) : round(vd);
    
    /* Nested conditionals */
    double d3 = (vf > 0) ? 
                ((vi < 10) ? nearbyint(vf) : rint(vf)) : 
                trunc(vf);
    
    return (int)(d1 + d2 + d3);
}

/* Test 6: Mixed expressions with arithmetic */
static int test_mixed_expressions(void) {
    /* Integer-valued calls in arithmetic expressions */
    double d1 = (trunc(5.9) * 2) / floor(2.1);
    double d2 = ceil(3.2) + round(4.7) - nearbyint(1.5);
    
    /* Comparisons that might be folded */
    int cmp1 = (ceil(vd) > floor(vf));
    int cmp2 = (trunc(3.14) == 3);
    
    /* Type casts */
    int i1 = (int)rint(5.4);
    int i2 = (int)trunc(6.8);
    
    return (int)(d1 + d2) + cmp1 + cmp2 + i1 + i2;
}

/* Test 7: Template metaprogramming (C++ only) */
#ifdef __cplusplus
template<double (*FUNC)(double)>
struct MathWrapper {
    static constexpr int apply(double x) {
        return (int)FUNC(x);
    }
};

static int test_template_metaprogramming() {
    constexpr int v1 = MathWrapper<trunc>::apply(5.9);
    constexpr int v2 = MathWrapper<floor>::apply(4.7);
    constexpr int v3 = MathWrapper<ceil>::apply(3.2);
    
    return v1 + v2 + v3;
}
#endif

/* Test 8: constexpr functions (C++ only) */
#ifdef __cplusplus
constexpr double constexpr_floor(double x) {
    return floor(x);
}

constexpr double constexpr_trunc(double x) {
    return trunc(x);
}

static int test_constexpr_functions() {
    constexpr double d1 = constexpr_floor(5.9);
    constexpr double d2 = constexpr_trunc(3.14);
    constexpr double d3 = floor(constexpr_trunc(4.8));
    
    return (int)(d1 + d2 + d3);
}
#endif

/* Test 9: Different argument types and values */
static int test_various_arguments(void) {
    /* Integer arguments */
    double d1 = floor(5);
    double d2 = trunc(2);
    
    /* Real arguments that are exact integers */
    double d3 = ceil(4.0);
    double d4 = round(6.0);
    
    /* Real arguments with fractional parts */
    double d5 = floor(4.7);
    double d6 = trunc(3.14159);
    
    /* Negative values */
    double d7 = round(-2.3);
    double d8 = ceil(-3.7);
    
    /* Large values */
    double d9 = trunc(1e10 + 0.5);
    double d10 = floor(1e15 - 0.2);
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10);
}

/* Test 10: Calls with different numbers of arguments */
static int test_varying_arg_counts(void) {
    /* Most math functions take 1 argument */
    double d1 = trunc(3.14);
    double d2 = floor(2.718);
    
    /* Some builtins might have optional arguments */
    /* Note: We need to check GCC documentation for specific builtins
       with optional arguments that are integer-valued real functions */
    
    /* Using fmax/fmin which take 2 args and are integer-valued 
       when both args are integers */
    double d3 = fmax(5.0, 3.0);  /* Should be 5.0 */
    double d4 = fmin(2.0, 4.0);  /* Should be 2.0 */
    
    /* Hypot with integer arguments */
    double d5 = hypot(3.0, 4.0);  /* Should be 5.0 */
    
    return (int)(d1 + d2 + d3 + d4 + d5);
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_functions();
    checksum += test_builtin_ll_functions();
    checksum += test_complex_parts();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_mixed_expressions();
    
    #ifdef __cplusplus
    checksum += test_template_metaprogramming();
    checksum += test_constexpr_functions();
    #endif
    
    checksum += test_various_arguments();
    checksum += test_varying_arg_counts();
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    return 0;
}
