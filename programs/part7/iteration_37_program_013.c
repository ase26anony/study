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
enum {
    VAL1 = (int)trunc(5.9),
    VAL2 = (int)floor(5.9),
    VAL3 = (int)ceil(5.1),
    VAL4 = (int)round(5.5),
    VAL5 = (int)nearbyint(5.3),
    VAL6 = (int)rint(5.7)
};

/* Test 2: Array sizes using integer-valued functions */
char buffer1[(int)floor(10.5)];
char buffer2[(int)ceil(9.1)];
char buffer3[(int)trunc(8.9)];

/* Test 3: Static assertions */
static_assert(trunc(5.9) == 5, "trunc failed");
static_assert(floor(5.9) == 5, "floor failed");
static_assert(ceil(5.1) == 6, "ceil failed");
static_assert(round(5.5) == 6, "round failed");

/* Test 4: Global initializers with integer-valued functions */
static double g1 = trunc(7.8);
static double g2 = floor(7.8);
static double g3 = ceil(7.2);
static double g4 = round(7.5);
static double g5 = nearbyint(7.3);
static double g6 = rint(7.7);

/* Test 5: Builtins with long long return */
static long long g7 = __builtin_llround(9.6);
static long long g8 = __builtin_llrint(9.3);

/* Test 6: Complex part extraction */
static double g9 = __real__(3.0 + 4.0 * I);
static double g10 = __imag__(3.0 + 4.0 * I);
static float g11 = __real__(1.5f + 2.5f * I);
static float g12 = __imag__(1.5f + 2.5f * I);

/* Test functions that will be folded */
static double test_nested_calls(void) {
    /* Nested calls to trigger recursive integer_valued_real_p */
    return floor(ceil(trunc(round(vd))));
}

static double test_conditional_calls(void) {
    /* Conditional operator with integer-valued calls */
    return (vi > 0) ? trunc(vd) : floor(vd);
}

static double test_mixed_arguments(void) {
    /* Mix of argument types */
    double r1 = trunc(5);      /* Integer argument */
    double r2 = ceil(4.0);     /* Exact integer real */
    double r3 = floor(4.7);    /* Fractional real */
    double r4 = round(-2.3);   /* Negative value */
    double r5 = nearbyint(1e10); /* Large value */
    
    return r1 + r2 + r3 + r4 + r5;
}

static double test_arithmetic_with_calls(void) {
    /* Arithmetic with integer-valued calls */
    return (trunc(vd) * 2.0) / floor(vf + 1.0);
}

static double test_comparison_with_calls(void) {
    /* Comparison triggering folding */
    if (ceil(vd) > floor(vf)) {
        return 1.0;
    }
    return 0.0;
}

static double test_builtin_ll_functions(void) {
    /* Builtins returning long long */
    long long ll1 = __builtin_llround(vd);
    long long ll2 = __builtin_llrint(vf);
    return (double)(ll1 + ll2);
}

static double test_complex_parts(void) {
    /* Complex part extraction */
    double r1 = __real__(vcd);
    double r2 = __imag__(vcd);
    float r3 = __real__(vcf);
    float r4 = __imag__(vcf);
    
    return r1 + r2 + r3 + r4;
}

static double test_deep_nesting(void) {
    /* Deeply nested calls */
    return trunc(round(floor(ceil(nearbyint(rint(vd))))));
}

static double test_call_as_argument(void) {
    /* Calls as arguments to other calls */
    return round(trunc(floor(ceil(vd))));
}

/* C++ specific tests (if compiled as C++) */
#ifdef __cplusplus
constexpr double cpp_constexpr_test() {
    return trunc(3.14) + floor(2.71) + ceil(1.41);
}

template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};

constexpr int template_val = TestTemplate<10>::value;
#endif

/* Main test driver */
int main(void) {
    double checksum = 0.0;
    
    /* Accumulate results from all tests */
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_mixed_arguments();
    checksum += test_arithmetic_with_calls();
    checksum += test_comparison_with_calls();
    checksum += test_builtin_ll_functions();
    checksum += test_complex_parts();
    checksum += test_deep_nesting();
    checksum += test_call_as_argument();
    
    /* Add global initializers */
    checksum += g1 + g2 + g3 + g4 + g5 + g6;
    checksum += (double)g7 + (double)g8;
    checksum += g9 + g10 + g11 + g12;
    
    /* Add enum values */
    checksum += VAL1 + VAL2 + VAL3 + VAL4 + VAL5 + VAL6;
    
    /* C++ specific additions */
    #ifdef __cplusplus
    checksum += cpp_constexpr_test();
    checksum += template_val;
    #endif
    
    /* Buffer sizes (will be folded to constants) */
    checksum += sizeof(buffer1) + sizeof(buffer2) + sizeof(buffer3);
    
    printf("Result: %f\n", checksum);
    printf("Enum values: %d %d %d %d %d %d\n", VAL1, VAL2, VAL3, VAL4, VAL5, VAL6);
    printf("Global doubles: %f %f %f %f %f %f\n", g1, g2, g3, g4, g5, g6);
    printf("Global long longs: %lld %lld\n", g7, g8);
    printf("Complex parts: %f %f %f %f\n", g9, g10, g11, g12);
    
    return 0;
}
