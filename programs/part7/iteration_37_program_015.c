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
    const double d2 = floor(5.9);
    const double d3 = ceil(5.1);
    const double d4 = round(5.5);
    const double d5 = nearbyint(5.3);
    const double d6 = rint(5.7);
    
    /* Use in static assertions (C++ style, but valid in C with _Static_assert) */
    _Static_assert(trunc(5.9) == 5, "trunc failed");
    _Static_assert(floor(5.9) == 5, "floor failed");
    _Static_assert(ceil(5.1) == 6, "ceil failed");
    
    /* Array sizes using integer-valued real functions */
    char arr1[(int)trunc(10.7)];
    char arr2[(int)floor(10.2)];
    char arr3[(int)ceil(10.1)];
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6 + sizeof(arr1) + sizeof(arr2) + sizeof(arr3));
}

/* Test 2: Nested calls to trigger recursive integer_valued_real_p */
static int test_nested_calls(void) {
    /* Nested integer-valued real function calls */
    double d1 = floor(ceil(3.7));      /* ceil(3.7)=4, floor(4)=4 */
    double d2 = trunc(round(2.3));     /* round(2.3)=2, trunc(2)=2 */
    double d3 = nearbyint(rint(4.8));  /* rint(4.8)=5, nearbyint(5)=5 */
    double d4 = ceil(floor(6.2));      /* floor(6.2)=6, ceil(6)=6 */
    
    /* More complex nesting */
    double d5 = trunc(floor(ceil(3.14)));
    double d6 = round(nearbyint(trunc(7.89)));
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6);
}

/* Test 3: Calls within conditional expressions */
static int test_conditional_calls(void) {
    int x = vi;
    double result = 0;
    
    /* Conditional operator with integer-valued real calls */
    result += (x > 0) ? trunc(4.7) : floor(4.7);
    result += (x < 0) ? ceil(3.2) : round(3.2);
    result += (x == 42) ? nearbyint(5.6) : rint(5.6);
    
    /* Nested conditional with calls */
    result += (x > 10) ? trunc((x < 20) ? floor(6.5) : ceil(6.5)) : round(6.5);
    
    return (int)result;
}

/* Test 4: Builtin functions with integer return types */
static int test_builtin_functions(void) {
    long long ll1 = __builtin_llround(3.14);
    long long ll2 = __builtin_llround(2.718);
    long long ll3 = __builtin_llrint(4.99);
    long long ll4 = __builtin_llrint(1.01);
    
    /* Use in constant expressions */
    enum { 
        VAL1 = __builtin_llround(10.1),
        VAL2 = __builtin_llround(20.9)
    };
    
    return (int)(ll1 + ll2 + ll3 + ll4 + VAL1 + VAL2);
}

/* Test 5: Complex number real/imag part extraction */
static int test_complex_parts(void) {
    double _Complex cd = 3.0 + 4.0 * I;
    float _Complex cf = 1.5f + 2.5f * I;
    
    /* These should be recognized as integer-valued real operations */
    double real_d = __real__ cd;
    double imag_d = __imag__ cd;
    float real_f = __real__ cf;
    float imag_f = __imag__ cf;
    
    /* With volatile to prevent front-end folding */
    double real_v = __real__ vcd;
    double imag_v = __imag__ vcd;
    float real_vf = __real__ vcf;
    float imag_vf = __imag__ vcf;
    
    return (int)(real_d + imag_d + real_f + imag_f + real_v + imag_v + real_vf + imag_vf);
}

/* Test 6: Mixed expressions with arithmetic */
static int test_mixed_expressions(void) {
    double x = vd;
    
    /* Arithmetic with integer-valued real calls */
    double d1 = trunc(x) * 2.0;
    double d2 = floor(x + 1.0) / ceil(x);
    double d3 = (round(x) > floor(x)) ? trunc(x) : nearbyint(x);
    double d4 = rint(x) + nearbyint(x * 2.0);
    
    /* Casts to integer types */
    int i1 = (int)trunc(x);
    int i2 = (int)floor(x);
    int i3 = (int)ceil(x);
    int i4 = (int)round(x);
    
    return (int)(d1 + d2 + d3 + d4) + i1 + i2 + i3 + i4;
}

/* Test 7: Calls with different argument counts */
static int test_varying_arguments(void) {
    /* Most integer-valued real functions take 1 argument */
    double d1 = trunc(5.5);
    double d2 = floor(5.5);
    
    /* Some builtins might have optional arguments */
    /* Use fmax/fmin which take 2 arguments and can return integer values */
    double d3 = fmax(trunc(3.7), floor(4.2));  /* fmax(3, 4) = 4 */
    double d4 = fmin(ceil(2.3), round(2.6));   /* fmin(3, 3) = 3 */
    
    /* Hypot can also produce integer results with integer inputs */
    double d5 = hypot(trunc(3.0), floor(4.0)); /* hypot(3, 4) = 5 */
    
    return (int)(d1 + d2 + d3 + d4 + d5);
}

/* Test 8: Negative values and edge cases */
static int test_edge_cases(void) {
    double d1 = trunc(-3.7);
    double d2 = floor(-3.7);
    double d3 = ceil(-3.7);
    double d4 = round(-3.5);
    double d5 = nearbyint(-2.3);
    double d6 = rint(-2.7);
    
    /* Large values */
    double d7 = trunc(1e10 + 0.7);
    double d8 = floor(1e10 + 0.2);
    double d9 = ceil(1e10 - 0.2);
    
    /* Exact integers */
    double d10 = trunc(5.0);
    double d11 = floor(5.0);
    double d12 = ceil(5.0);
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 + d11 + d12);
}

/* Test 9: In constexpr context (C++ style, but using static for similar effect) */
static int test_constexpr_style(void) {
    /* Simulate constexpr behavior with static const */
    static const double d1 = trunc(7.8);
    static const double d2 = floor(7.8);
    static const double d3 = ceil(7.2);
    static const double d4 = round(7.5);
    
    /* Use in switch cases */
    int val = vi % 4;
    switch(val) {
        case (int)trunc(1.0): val += 1; break;
        case (int)floor(2.0): val += 2; break;
        case (int)ceil(3.0): val += 3; break;
        case (int)round(4.0): val += 4; break;
    }
    
    return (int)(d1 + d2 + d3 + d4) + val;
}

/* Test 10: Template-like patterns using macros */
#define INTEGER_VALUED_CALL(fn, arg) ((int)fn(arg))

static int test_macro_patterns(void) {
    int sum = 0;
    sum += INTEGER_VALUED_CALL(trunc, 4.7);
    sum += INTEGER_VALUED_CALL(floor, 4.7);
    sum += INTEGER_VALUED_CALL(ceil, 4.2);
    sum += INTEGER_VALUED_CALL(round, 4.5);
    sum += INTEGER_VALUED_CALL(nearbyint, 4.3);
    sum += INTEGER_VALUED_CALL(rint, 4.8);
    
    /* Chain macro expansions */
    sum += INTEGER_VALUED_CALL(trunc, floor(5.9));
    sum += INTEGER_VALUED_CALL(floor, ceil(5.1));
    
    return sum;
}

/* Main driver that combines all tests */
int main(void) {
    int checksum = 0;
    
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_functions();
    checksum += test_complex_parts();
    checksum += test_mixed_expressions();
    checksum += test_varying_arguments();
    checksum += test_edge_cases();
    checksum += test_constexpr_style();
    checksum += test_macro_patterns();
    
    printf("Result: %d\n", checksum);
    
    /* Additional compile-time tests */
    _Static_assert(__builtin_llround(3.0) == 3, "llround failed");
    _Static_assert(trunc(trunc(4.9)) == 4, "nested trunc failed");
    
    return 0;
}
