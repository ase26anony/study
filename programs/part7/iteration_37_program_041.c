/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 10;

/* Complex types */
volatile double _Complex vcd = 3.0 + 4.0*I;
volatile float _Complex vcf = 1.5f + 2.5f*I;

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
    char arr1[(int)trunc(10.5)];
    char arr2[(int)floor(15.3)];
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6) + sizeof(arr1) + sizeof(arr2);
}

/* Test 2: Nested calls to exercise recursive depth */
static int test_nested_calls(void) {
    /* Nested integer-valued real function calls */
    double result = 0.0;
    
    /* Simple nesting */
    result += floor(ceil(3.7));          /* floor(4.0) = 4.0 */
    result += trunc(round(2.5));         /* trunc(3.0) = 3.0 */
    result += nearbyint(rint(1.3));      /* nearbyint(1.0) = 1.0 */
    
    /* Deeper nesting */
    result += ceil(floor(trunc(4.9)));   /* ceil(floor(4.0)) = ceil(4.0) = 4.0 */
    result += round(nearbyint(ceil(2.1))); /* round(nearbyint(3.0)) = round(3.0) = 3.0 */
    
    /* Mixed nesting with arithmetic */
    result += trunc(floor(3.8) + ceil(2.2)); /* trunc(3.0 + 3.0) = trunc(6.0) = 6.0 */
    
    return (int)result;
}

/* Test 3: Conditional expressions with integer-valued real calls */
static int test_conditional_calls(void) {
    double result = 0.0;
    int flag = vi > 5 ? 1 : 0;
    
    /* Conditional operator with integer-valued real calls as branches */
    result += flag ? trunc(vd) : floor(vd);
    result += (vi % 2) ? ceil(vf) : round(vf);
    
    /* Nested conditional with integer-valued real calls */
    result += (vd > 3.0) ? 
              (trunc(vd) > 3 ? floor(vd) : ceil(vd)) : 
              round(vd);
    
    /* Conditional with complex expression */
    result += (vf < 3.0f) ? 
              trunc(floor(vd) + 1.5) : 
              ceil(round(vf) - 0.5);
    
    return (int)result;
}

/* Test 4: Builtin functions with explicit integer returns */
static int test_builtin_functions(void) {
    long long llresult = 0;
    
    /* __builtin_llround and __builtin_llrint return long long */
    llresult += __builtin_llround(3.7);
    llresult += __builtin_llround(2.3);
    llresult += __builtin_llrint(4.9);
    llresult += __builtin_llrint(1.1);
    
    /* Use in constant expressions */
    enum { 
        VAL1 = (int)__builtin_llround(5.5),
        VAL2 = (int)__builtin_llrint(3.3)
    };
    
    char buffer1[VAL1];
    char buffer2[VAL2];
    
    return (int)llresult + sizeof(buffer1) + sizeof(buffer2);
}

/* Test 5: Complex number real/imag part extraction */
static int test_complex_parts(void) {
    double result = 0.0;
    
    /* Extract real and imaginary parts from complex numbers */
    result += __real__(vcd);      /* Should be 3.0 */
    result += __imag__(vcd);      /* Should be 4.0 */
    result += __real__(vcf);      /* Should be 1.5 */
    result += __imag__(vcf);      /* Should be 2.5 */
    
    /* Complex parts in expressions with integer-valued functions */
    result += trunc(__real__(vcd));
    result += floor(__imag__(vcd));
    result += ceil(__real__(vcf));
    result += round(__imag__(vcf));
    
    /* Nested: integer-valued function on complex part extraction */
    result += __real__((double _Complex)trunc(vd) + (double _Complex)floor(vd));
    
    return (int)result;
}

/* Test 6: Calls with different numbers of arguments */
static int test_varying_arguments(void) {
    double result = 0.0;
    
    /* Most math functions take 1 argument */
    result += trunc(vd);
    result += floor(vd);
    result += ceil(vd);
    
    /* Some builtins might have optional arguments */
    /* For example, fma takes 3 arguments but isn't integer-valued */
    /* We'll use conditional to simulate different argument counts */
    
    /* Create expressions that might be seen as calls with 0, 1, 2 args */
    /* by using function pointers or macros in real code */
    
    /* Use volatile to prevent complete optimization */
    volatile double (*fp1)(double) = trunc;
    volatile double (*fp2)(double) = floor;
    
    result += fp1(vd);
    result += fp2(vd);
    
    return (int)result;
}

/* Test 7: Mixed expressions and arithmetic */
static int test_mixed_expressions(void) {
    double result = 0.0;
    
    /* Arithmetic with integer-valued real functions */
    result += trunc(vd) * 2.0;
    result += floor(vf) / 2.0f;
    result += ceil(vd) + floor(vf);
    result += round(vd) - trunc(vf);
    
    /* Comparisons involving integer-valued real functions */
    if (ceil(vd) > floor(vf)) {
        result += 10.0;
    }
    
    if (trunc(vd) == 3.0) {
        result += 20.0;
    }
    
    /* Type casts */
    result += (int)rint(vd);
    result += (long)round(vf);
    
    /* In loop bounds (compile-time if possible) */
    const int limit = (int)floor(5.5);
    for (int i = 0; i < limit; i++) {
        result += i;
    }
    
    return (int)result;
}

/* Test 8: C++ constexpr functions (if compiled as C++) */
#ifdef __cplusplus
constexpr double constexpr_trunc(double x) {
    return trunc(x);
}

constexpr double constexpr_floor(double x) {
    return floor(x);
}

constexpr int test_constexpr_functions() {
    constexpr double d1 = constexpr_trunc(7.8);
    constexpr double d2 = constexpr_floor(6.4);
    constexpr double d3 = ceil(5.1);
    
    /* Use in template parameters */
    template<int N> struct TestStruct {
        static const int value = N;
    };
    
    TestStruct<(int)d1> ts1;
    TestStruct<(int)d2> ts2;
    
    return (int)(d1 + d2 + d3) + ts1.value + ts2.value;
}
#endif

/* Test 9: Large and edge case values */
static int test_edge_cases(void) {
    double result = 0.0;
    
    /* Exact integers */
    result += trunc(4.0);
    result += floor(5.0);
    result += ceil(6.0);
    
    /* Negative values */
    result += trunc(-3.7);
    result += floor(-2.3);
    result += ceil(-1.8);
    result += round(-4.5);
    
    /* Large values */
    result += trunc(1e10 + 0.7);
    result += floor(1e15 + 0.3);
    
    /* Zero */
    result += trunc(0.0);
    result += floor(0.0);
    result += ceil(0.0);
    
    return (int)result;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing integer-valued real function constant folding...\n");
    
    /* Run all tests and accumulate checksum */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_functions();
    checksum += test_complex_parts();
    checksum += test_varying_arguments();
    checksum += test_mixed_expressions();
    checksum += test_edge_cases();
    
#ifdef __cplusplus
    checksum += test_constexpr_functions();
#endif
    
    printf("Result: %d\n", checksum);
    
    /* Verify some results at runtime */
    if (trunc(5.9) != 5.0) {
        printf("ERROR: trunc(5.9) != 5.0\n");
        return 1;
    }
    
    if (floor(4.7) != 4.0) {
        printf("ERROR: floor(4.7) != 4.0\n");
        return 1;
    }
    
    if (ceil(3.2) != 4.0) {
        printf("ERROR: ceil(3.2) != 4.0\n");
        return 1;
    }
    
    printf("All basic tests passed.\n");
    return 0;
}
