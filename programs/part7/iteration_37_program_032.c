/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Global checksum accumulator */
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
    long long ll1 = __builtin_llround(3.14);
    long long ll2 = __builtin_llround(2.71);
    long long ll3 = __builtin_llrint(1.618);
    long long ll4 = __builtin_llrint(0.577);
    
    checksum += (int)(ll1 + ll2 + ll3 + ll4) % 1000;
}

/* Test 3: Complex number real/imag parts */
void test_complex_parts(void) {
    /* Complex integer types with __real__ and __imag__ */
    complex int ci = 3 + 4 * I;
    complex long cl = 10L + 20L * I;
    
    double r1 = __real__(ci);  /* Should be integer-valued real */
    double i1 = __imag__(ci);
    double r2 = __real__(cl);
    double i2 = __imag__(cl);
    
    checksum += (int)(r1 + i1 + r2 + i2);
}

/* Test 4: Nested integer-valued real calls */
void test_nested_calls(void) {
    /* Nested calls to trigger recursive integer_valued_real_p */
    double d1 = floor(ceil(2.3));      /* floor(3.0) = 3.0 */
    double d2 = trunc(round(4.6));     /* trunc(5.0) = 5.0 */
    double d3 = nearbyint(rint(1.5));  /* nearbyint(2.0) = 2.0 */
    double d4 = ceil(floor(3.8));      /* ceil(3.0) = 3.0 */
    
    checksum += (int)(d1 + d2 + d3 + d4);
}

/* Test 5: Calls within conditional expressions */
void test_conditional_calls(void) {
    /* Conditional operator with integer-valued real calls */
    double d1 = (vi > 0) ? trunc(vd) : floor(vf);
    double d2 = (vi < 100) ? ceil(vd + 1.0) : round(vf - 1.0);
    double d3 = (vd > 0.0) ? nearbyint(vd * 2.0) : rint(vf / 2.0);
    
    checksum += (int)(d1 + d2 + d3);
}

/* Test 6: Calls as arguments to other integer-valued functions */
void test_call_arguments(void) {
    /* Integer-valued calls as arguments */
    double d1 = round(trunc(7.8));      /* round(7.0) = 7.0 */
    double d2 = floor(ceil(5.1));       /* floor(6.0) = 6.0 */
    double d3 = trunc(nearbyint(3.7));  /* trunc(4.0) = 4.0 */
    
    checksum += (int)(d1 + d2 + d3);
}

/* Test 7: Mixed arithmetic with integer-valued calls */
void test_mixed_arithmetic(void) {
    /* Arithmetic expressions containing integer-valued calls */
    double d1 = trunc(vd) * 2.0 + floor(vf);
    double d2 = (ceil(vd) - round(vf)) / 2.0;
    double d3 = nearbyint(vd * 2.0) + rint(vf * 3.0);
    
    checksum += (int)(d1 + d2 + d3);
}

/* Test 8: Compile-time constant contexts (C++ style if available) */
#ifdef __cplusplus
constexpr double cpp_constexpr_test() {
    return trunc(9.9) + floor(8.1) + ceil(7.3);
}
#endif

void test_compile_time_contexts(void) {
    /* Array sizes using integer-valued calls */
    char buffer1[(int)floor(10.5)];  /* size = 10 */
    char buffer2[(int)trunc(8.9)];   /* size = 8 */
    
    /* Static assertions */
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(4.7) == 4, "floor failed");
    static_assert(ceil(3.2) == 4, "ceil failed");
    static_assert(round(6.5) == 7, "round failed");
    
    /* Enum values */
    enum {
        VAL1 = (int)trunc(15.7),
        VAL2 = (int)floor(12.3),
        VAL3 = (int)ceil(9.8),
        VAL4 = (int)round(11.5)
    };
    
    checksum += sizeof(buffer1) + sizeof(buffer2) + VAL1 + VAL2 + VAL3 + VAL4;
    
#ifdef __cplusplus
    checksum += (int)cpp_constexpr_test();
#endif
}

/* Test 9: Various argument types and values */
void test_various_arguments(void) {
    /* Integer arguments */
    double d1 = floor(5);      /* 5.0 */
    double d2 = trunc(2);      /* 2.0 */
    
    /* Exact integer real arguments */
    double d3 = ceil(4.0);     /* 4.0 */
    double d4 = round(6.0);    /* 6.0 */
    
    /* Fractional arguments */
    double d5 = floor(4.7);    /* 4.0 */
    double d6 = trunc(3.14);   /* 3.0 */
    
    /* Negative values */
    double d7 = round(-2.3);   /* -2.0 */
    double d8 = ceil(-3.7);    /* -3.0 */
    
    /* Large values */
    double d9 = floor(1e6 + 0.5);
    double d10 = trunc(1e9 - 0.1);
    
    checksum += (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9/1000 + d10/1000000);
}

/* Test 10: Complex expressions with comparisons */
void test_complex_expressions(void) {
    /* Comparisons involving integer-valued calls */
    int b1 = (ceil(vd) > floor(vf));
    int b2 = (trunc(vd * 2.0) == round(vf * 3.0));
    int b3 = (nearbyint(vd) <= rint(vf));
    
    /* Type casts */
    int i1 = (int)rint(vd);
    int i2 = (int)nearbyint(vf * 10.0);
    
    /* Loop bound using integer-valued call */
    int limit = (int)floor(vd + 2.0);
    int sum = 0;
    for (int i = 0; i < limit; i++) {
        sum += i;
    }
    
    checksum += b1 + b2 + b3 + i1 + i2 + sum;
}

/* Main driver function */
int main(void) {
    printf("Starting integer-valued real function tests...\n");
    
    /* Run all tests */
    test_basic_functions();
    test_builtin_functions();
    test_complex_parts();
    test_nested_calls();
    test_conditional_calls();
    test_call_arguments();
    test_mixed_arithmetic();
    test_compile_time_contexts();
    test_various_arguments();
    test_complex_expressions();
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum == 0 ? 0 : 1;
}
