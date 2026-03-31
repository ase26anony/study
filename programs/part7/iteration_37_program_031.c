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
static int test_basic_constants(void) {
    /* These should be folded by fold-const.cc */
    const double d1 = trunc(5.9);
    const double d2 = floor(5.9);
    const double d3 = ceil(5.1);
    const double d4 = round(5.5);
    const double d5 = nearbyint(5.3);
    const double d6 = rint(5.7);
    
    /* Use in static assertions (C++ would use static_assert) */
    struct {
        char buf1[(int)trunc(10.5)];
        char buf2[(int)floor(10.5)];
        char buf3[(int)ceil(10.5)];
    } s;
    
    /* Force evaluation in constant context */
    return (int)(d1 + d2 + d3 + d4 + d5 + d6) + sizeof(s.buf1);
}

/* Test 2: Nested calls to trigger recursive integer_valued_real_p */
static int test_nested_calls(void) {
    /* Nested calls - should trigger depth > 0 */
    double d1 = floor(ceil(3.7));
    double d2 = trunc(round(2.3));
    double d3 = nearbyint(rint(4.9));
    double d4 = ceil(floor(6.2));
    
    /* More complex nesting */
    double d5 = trunc(floor(ceil(5.8)));
    double d6 = round(nearbyint(trunc(7.1)));
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6);
}

/* Test 3: Calls in conditional expressions */
static int test_conditional_calls(void) {
    int x = vi;
    double result = 0;
    
    /* Conditional with integer-valued real calls */
    for (int i = 0; i < 5; i++) {
        result += (x > 5) ? trunc(vd + i) : floor(vf * i);
    }
    
    /* Nested conditional */
    result += (x % 2) ? ceil(result) : round(result);
    
    return (int)result;
}

/* Test 4: Builtin functions with different argument counts */
static int test_builtin_calls(void) {
    /* Builtins that might have different argument handling */
    long long ll1 = __builtin_llround(3.14);
    long long ll2 = __builtin_llrint(2.71);
    
    /* Complex part extraction - integer-valued real operations */
    double r1 = __real__(vcd);
    double i1 = __imag__(vcd);
    float r2 = __real__(vcf);
    float i2 = __imag__(vcf);
    
    /* Mix with other integer-valued functions */
    double d1 = trunc(__real__(vcd));
    double d2 = floor(__imag__(vcd));
    
    return (int)(ll1 + ll2 + r1 + i1 + r2 + i2 + d1 + d2);
}

/* Test 5: Arithmetic expressions with integer-valued calls */
static int test_arithmetic_expressions(void) {
    double x = vd;
    double y = vf;
    
    /* Complex arithmetic expressions */
    double expr1 = (trunc(x) * 2.0) / floor(y + 1.0);
    double expr2 = ceil(x * 2.0) + round(y * 3.0);
    double expr3 = nearbyint(expr1) * rint(expr2);
    
    /* Comparison triggering folding */
    int cmp = (ceil(x) > floor(y)) ? 1 : 0;
    
    /* Type casts */
    int cast1 = (int)trunc(x);
    int cast2 = (int)round(y);
    
    return (int)(expr1 + expr2 + expr3) + cmp + cast1 + cast2;
}

/* Test 6: Template/constexpr context (C++ style simulation) */
#ifdef __cplusplus
template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};
#endif

static int test_constexpr_context(void) {
    /* Simulate constexpr evaluation */
    const double d1 = trunc(15.9);
    const double d2 = floor(20.1);
    const double d3 = ceil(25.5);
    
    /* Use in array size */
    char buffer1[(int)d1];
    char buffer2[(int)d2];
    char buffer3[(int)d3];
    
    /* Enum values */
    enum {
        VAL1 = (int)trunc(30.7),
        VAL2 = (int)floor(35.2),
        VAL3 = (int)ceil(40.8)
    };
    
    return sizeof(buffer1) + sizeof(buffer2) + sizeof(buffer3) + VAL1 + VAL2 + VAL3;
}

/* Test 7: Mixed argument types */
static int test_mixed_arguments(void) {
    /* Integer arguments */
    double d1 = trunc(5);      /* Exact integer */
    double d2 = floor(4.0);    /* Real but integer value */
    double d3 = ceil(4.7);     /* Real with fractional part */
    double d4 = round(-2.3);   /* Negative value */
    double d5 = nearbyint(1e6); /* Large value */
    
    /* Mixed in expressions */
    double d6 = trunc(vd) + floor(vf);
    double d7 = ceil(d4) * round(d5);
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7);
}

/* Test 8: Loop with integer-valued calls */
static int test_loop_context(void) {
    double sum = 0;
    
    /* Loop with compile-time known bounds */
    for (int i = (int)trunc(1.5); i < (int)ceil(10.3); i++) {
        sum += floor(i * 1.1);
    }
    
    /* Conditional in loop */
    for (int i = 0; i < 5; i++) {
        sum += (i % 2) ? trunc(sum + i) : round(sum - i);
    }
    
    return (int)sum;
}

/* Main driver */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_constants();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_calls();
    checksum += test_arithmetic_expressions();
    checksum += test_constexpr_context();
    checksum += test_mixed_arguments();
    checksum += test_loop_context();
    
    printf("Result: %d\n", checksum);
    
    /* Additional verification */
    printf("Direct calls verification:\n");
    printf("trunc(5.9) = %.0f\n", trunc(5.9));
    printf("floor(5.9) = %.0f\n", floor(5.9));
    printf("ceil(5.1) = %.0f\n", ceil(5.1));
    printf("round(5.5) = %.0f\n", round(5.5));
    
    return 0;
}
