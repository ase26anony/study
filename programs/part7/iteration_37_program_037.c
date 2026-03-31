/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Global variables to force folding in static initializers */
static const double cd = 5.67;
static const float cf = 9.87f;
static const int ci = 123;

/* Test 1: Basic integer-valued real functions in constant contexts */
enum {
    ENUM_TRUNC = (int)trunc(10.7),
    ENUM_FLOOR = (int)floor(10.7),
    ENUM_CEIL = (int)ceil(10.3),
    ENUM_ROUND = (int)round(10.5)
};

/* Test 2: Array sizes using these functions */
char buffer1[(int)trunc(20.3)];
char buffer2[(int)floor(20.7)];
char buffer3[(int)ceil(20.1)];

/* Test 3: Static assertions */
static_assert(trunc(5.9) == 5, "trunc failed");
static_assert(floor(5.9) == 5, "floor failed");
static_assert(ceil(5.1) == 6, "ceil failed");
static_assert(round(5.5) == 6, "round failed");

/* Test 4: Complex number real/imag parts */
static const _Complex int ci1 = 3 + 4I;
static const _Complex long cl1 = 10L + 20LI;

/* Test 5: Template metaprogramming (C++ only) */
#ifdef __cplusplus
template<int N>
struct TestTemplate {
    static const int value = N;
};

constexpr int constexpr_floor(double x) {
    return (int)floor(x);
}

constexpr int constexpr_ceil(double x) {
    return (int)ceil(x);
}
#endif

/* Test functions for different patterns */

/* Pattern 1: Nested calls */
double test_nested_calls(double x) {
    return floor(ceil(trunc(round(x))));
}

/* Pattern 2: Calls in conditional expressions */
double test_conditional_calls(double x, int flag) {
    return flag ? trunc(x) : floor(x);
}

/* Pattern 3: Calls as function arguments */
double test_nested_args(double x) {
    return round(trunc(floor(x)));
}

/* Pattern 4: Mixed with arithmetic */
double test_mixed_arithmetic(double x, double y) {
    return (trunc(x) * 2.0) + floor(y) / ceil(x + 1.0);
}

/* Pattern 5: Builtins with different argument counts */
long long test_builtin_llround(double x) {
    return __builtin_llround(x);
}

long long test_builtin_llrint(double x) {
    return __builtin_llrint(x);
}

/* Pattern 6: Complex part extraction */
int test_complex_parts() {
    _Complex int c = 3 + 4I;
    return __real__(c) + __imag__(c);
}

/* Pattern 7: Nearbyint and rint */
double test_nearbyint_rint(double x) {
    return nearbyint(rint(x));
}

/* Pattern 8: Multiple calls in one expression */
double test_multiple_calls(double x, double y) {
    return trunc(x) + floor(y) - ceil(x + y) * round(x - y);
}

/* Pattern 9: Integer arguments */
double test_integer_args(int x) {
    return floor(x) + trunc(x * 2.0);
}

/* Pattern 10: Negative values */
double test_negative_values(double x) {
    return floor(-x) + ceil(-x * 2.0);
}

/* Pattern 11: Large values */
double test_large_values(double x) {
    return trunc(x * 1e6) + floor(x * 1e6 + 0.5);
}

/* Pattern 12: Const variable arguments */
double test_const_args() {
    const double local_const = 7.89;
    return round(local_const) + trunc(cd);
}

/* Pattern 13: Volatile to prevent pre-folding */
double test_volatile_args() {
    return floor(vd) + ceil(vf);
}

/* Pattern 14: Type casting combinations */
int test_type_casts(double x) {
    return (int)trunc(x) + (int)floor(x * 2.0);
}

/* Pattern 15: Comparison with calls */
int test_comparisons(double x, double y) {
    return trunc(x) > floor(y) ? 1 : 0;
}

/* Pattern 16: Loop bound using these functions */
int test_loop_bound(double x) {
    int bound = (int)ceil(x);
    int sum = 0;
    for (int i = 0; i < bound; i++) {
        sum += i;
    }
    return sum;
}

/* Pattern 17: Switch with case values from these functions */
int test_switch_case(double x) {
    int val = (int)round(x);
    switch (val) {
        case 0: return 1;
        case 1: return 2;
        case 2: return 3;
        default: return 0;
    }
}

/* Pattern 18: Recursive-like nesting */
double test_deep_nesting(double x) {
    return trunc(floor(ceil(round(nearbyint(rint(x))))));
}

/* Pattern 19: Conditional with multiple calls */
double test_complex_conditional(double x, double y, int flag) {
    return flag ? trunc(x) + floor(y) : ceil(x) - round(y);
}

/* Pattern 20: Array indexing with these functions */
double test_array_indexing(double x, double arr[10]) {
    int idx = (int)floor(x) % 10;
    return arr[idx] + trunc(x);
}

/* Main driver that exercises all patterns */
int main() {
    double checksum = 0.0;
    double test_input = 3.7;
    double test_input2 = 2.3;
    
    /* Static initialization tests */
    checksum += ENUM_TRUNC + ENUM_FLOOR + ENUM_CEIL + ENUM_ROUND;
    
    /* Complex part extraction */
    checksum += test_complex_parts();
    checksum += __real__(ci1) + __imag__(ci1);
    
    /* Exercise all test patterns */
    checksum += test_nested_calls(test_input);
    checksum += test_conditional_calls(test_input, 1);
    checksum += test_conditional_calls(test_input, 0);
    checksum += test_nested_args(test_input);
    checksum += test_mixed_arithmetic(test_input, test_input2);
    
    checksum += test_builtin_llround(test_input);
    checksum += test_builtin_llrint(test_input);
    
    checksum += test_nearbyint_rint(test_input);
    checksum += test_multiple_calls(test_input, test_input2);
    checksum += test_integer_args(vi);
    checksum += test_negative_values(test_input);
    checksum += test_large_values(test_input);
    checksum += test_const_args();
    checksum += test_volatile_args();
    checksum += test_type_casts(test_input);
    checksum += test_comparisons(test_input, test_input2);
    checksum += test_loop_bound(test_input);
    checksum += test_switch_case(test_input);
    checksum += test_deep_nesting(test_input);
    checksum += test_complex_conditional(test_input, test_input2, 1);
    checksum += test_complex_conditional(test_input, test_input2, 0);
    
    double arr[10] = {0,1,2,3,4,5,6,7,8,9};
    checksum += test_array_indexing(test_input, arr);
    
    /* C++ specific tests */
    #ifdef __cplusplus
    constexpr int ct1 = constexpr_floor(8.9);
    constexpr int ct2 = constexpr_ceil(8.1);
    checksum += ct1 + ct2;
    
    TestTemplate<constexpr_floor(5.5)> t1;
    TestTemplate<constexpr_ceil(5.5)> t2;
    checksum += t1.value + t2.value;
    #endif
    
    printf("Result: %f\n", checksum);
    
    /* Additional static checks */
    static double static_var1 = trunc(100.7);
    static double static_var2 = floor(100.7);
    static double static_var3 = ceil(100.3);
    static double static_var4 = round(100.5);
    
    printf("Static vars: %f %f %f %f\n", 
           static_var1, static_var2, static_var3, static_var4);
    
    return (int)checksum % 256;
}
