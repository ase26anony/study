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
    /* Use in static initializers */
    static const double d1 = trunc(5.9);
    static const double d2 = floor(4.7);
    static const double d3 = ceil(3.2);
    static const double d4 = round(6.5);
    
    /* Use in array sizes */
    char buffer1[(int)floor(10.5)];
    char buffer2[(int)ceil(9.1)];
    
    /* Use in static assertions */
    _Static_assert(trunc(5.9) == 5, "trunc failed");
    _Static_assert(floor(4.7) == 4, "floor failed");
    _Static_assert(ceil(3.2) == 4, "ceil failed");
    _Static_assert(round(6.5) == 7, "round failed");
    
    checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4;
    checksum += sizeof(buffer1) + sizeof(buffer2);
}

/* Test 2: Nested calls to integer-valued functions */
void test_nested_calls(void) {
    /* Simple nesting */
    double r1 = floor(ceil(3.7));      /* floor(4.0) = 4.0 */
    double r2 = trunc(round(2.3));     /* trunc(2.0) = 2.0 */
    double r3 = ceil(floor(5.9));      /* ceil(5.0) = 5.0 */
    
    /* Multiple levels of nesting */
    double r4 = round(trunc(floor(7.8))); /* round(trunc(7.0)) = round(7.0) = 7.0 */
    
    /* Mix with arithmetic */
    double r5 = 2.0 * floor(ceil(2.5) / 2.0);
    
    checksum += (int)r1 + (int)r2 + (int)r3 + (int)r4 + (int)r5;
}

/* Test 3: Conditional expressions with integer-valued calls */
void test_conditional_calls(void) {
    int x = vi;
    
    /* Conditional operator with calls in branches */
    double r1 = (x > 0) ? trunc(4.9) : floor(3.2);
    double r2 = (x < 0) ? ceil(2.1) : round(5.6);
    
    /* Nested conditional with calls */
    double r3 = (x == 42) ? 
                (x > 10 ? floor(8.9) : ceil(7.1)) : 
                round(3.4);
    
    /* Calls as arguments to conditional */
    double r4 = trunc((x > 0) ? 6.7 : 1.2);
    
    checksum += (int)r1 + (int)r2 + (int)r3 + (int)r4;
}

/* Test 4: Builtin functions with different argument counts */
void test_builtin_functions(void) {
    /* Builtins with 1 argument */
    long long r1 = __builtin_llround(3.14);
    long long r2 = __builtin_llrint(2.71);
    
    /* Use in constant expressions */
    enum { 
        VAL1 = __builtin_llround(10.5),
        VAL2 = __builtin_llrint(20.3)
    };
    
    /* Complex number real/imag parts */
    complex int ci = 3 + 4 * I;
    int r3 = __real__ ci;  /* Should be integer-valued */
    int r4 = __imag__ ci;  /* Should be integer-valued */
    
    checksum += (int)r1 + (int)r2 + VAL1 + VAL2 + r3 + r4;
}

/* Test 5: Mixed argument types and values */
void test_mixed_arguments(void) {
    /* Integer arguments */
    double r1 = floor(5);      /* 5.0 */
    double r2 = trunc(2);      /* 2.0 */
    
    /* Real arguments that are exact integers */
    double r3 = ceil(4.0);     /* 4.0 */
    double r4 = round(6.0);    /* 6.0 */
    
    /* Negative values */
    double r5 = floor(-3.7);   /* -4.0 */
    double r6 = ceil(-2.3);    /* -2.0 */
    double r7 = round(-1.5);   /* -2.0 */
    
    /* Large values */
    double r8 = trunc(1e10 + 0.7);
    double r9 = floor(1e10 - 0.3);
    
    /* Use const variables to prevent front-end folding */
    const double cd1 = 7.8;
    const double cd2 = -5.6;
    double r10 = ceil(cd1);
    double r11 = floor(cd2);
    
    checksum += (int)r1 + (int)r2 + (int)r3 + (int)r4 + 
                (int)r5 + (int)r6 + (int)r7 + (int)r8 +
                (int)r9 + (int)r10 + (int)r11;
}

/* Test 6: Calls within larger expressions */
void test_complex_expressions(void) {
    /* Arithmetic with calls */
    double r1 = (trunc(4.9) * 2) / floor(2.5);
    double r2 = ceil(3.2) + floor(2.7) - round(1.5);
    
    /* Comparisons with calls */
    int cmp1 = (ceil(3.1) > floor(2.9)) ? 1 : 0;
    int cmp2 = (trunc(5.5) == round(5.5)) ? 1 : 0;
    
    /* Type casts */
    int r3 = (int)rint(8.3);
    long r4 = (long)floor(9.99);
    
    /* In loop bounds (compile-time if possible) */
    const int limit = (int)ceil(5.1);
    int sum = 0;
    for (int i = 0; i < limit; i++) {
        sum += i;
    }
    
    checksum += (int)r1 + (int)r2 + cmp1 + cmp2 + r3 + r4 + sum;
}

/* Test 7: C++ specific tests (compile with g++) */
#ifdef __cplusplus
#include <type_traits>

constexpr double cpp_trunc(double x) { return trunc(x); }
constexpr double cpp_floor(double x) { return floor(x); }

template<int N>
struct TestTemplate {
    static const int value = (int)floor(N * 1.5);
};

void test_cpp_features(void) {
    /* constexpr functions */
    constexpr double d1 = cpp_trunc(7.8);
    constexpr double d2 = cpp_floor(6.2);
    
    /* Template arguments */
    constexpr int tval = TestTemplate<5>::value;
    
    /* static_assert with calls */
    static_assert(cpp_trunc(9.9) == 9, "C++ trunc failed");
    static_assert(cpp_floor(8.1) == 8, "C++ floor failed");
    
    /* Use in non-type template parameter */
    char buffer[(int)cpp_ceil(4.3)];
    
    checksum += (int)d1 + (int)d2 + tval + sizeof(buffer);
}
#endif

/* Test 8: nearbyint and rint functions */
void test_special_functions(void) {
    /* nearbyint with different rounding modes */
    double r1 = nearbyint(3.5);
    double r2 = nearbyint(2.5);
    double r3 = nearbyint(-1.5);
    
    /* rint function */
    double r4 = rint(4.3);
    double r5 = rint(5.7);
    double r6 = rint(-3.8);
    
    /* Mix with volatile to prevent optimization */
    double r7 = nearbyint(vd);
    double r8 = rint(vf);
    
    checksum += (int)r1 + (int)r2 + (int)r3 + (int)r4 + 
                (int)r5 + (int)r6 + (int)r7 + (int)r8;
}

/* Test 9: Zero, one, and two argument calls */
void test_argument_counts(void) {
    /* Most math functions take 1 argument */
    double r1 = trunc(1.0);
    double r2 = floor(1.0);
    
    /* Some builtins might have optional arguments */
    /* Note: Standard math functions typically have fixed arg counts */
    
    /* Create expressions that might be seen as calls with 0 or 2 args */
    /* by the compiler's internal representation */
    double r3 = __builtin_isfinite(1.0) ? trunc(2.0) : 0.0;
    double r4 = __builtin_signbit(-0.0) ? floor(3.0) : ceil(4.0);
    
    checksum += (int)r1 + (int)r2 + (int)r3 + (int)r4;
}

/* Test 10: Recursive depth testing */
void test_recursive_depth(void) {
    /* Deeply nested calls to test recursion depth */
    double r1 = floor(ceil(trunc(round(nearbyint(rint(7.3))))));
    
    /* Multiple calls in expression tree */
    double r2 = trunc(3.1) + floor(2.9) - ceil(1.1) * round(0.5);
    
    /* Calls as arguments to other calls */
    double r3 = fabs(floor(-4.3));  /* fabs is not integer-valued, but floor is */
    double r4 = sqrt(ceil(16.1));   /* sqrt(17.0) - not integer-valued */
    
    checksum += (int)r1 + (int)r2 + (int)r3 + (int)r4;
}

int main(void) {
    /* Run all tests */
    test_basic_functions();
    test_nested_calls();
    test_conditional_calls();
    test_builtin_functions();
    test_mixed_arguments();
    test_complex_expressions();
    test_special_functions();
    test_argument_counts();
    test_recursive_depth();
    
#ifdef __cplusplus
    test_cpp_features();
#endif
    
    /* Print checksum to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    /* Also use results in return value */
    return checksum == 0 ? 0 : 1;
}
