/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Global volatile to prevent premature constant folding */
volatile double g_input = 3.14159;
volatile double g_neg_input = -2.71828;

/* Test 1: Basic integer-valued real functions in constant contexts */
int test_basic_functions(void) {
    /* Use in static initializers */
    static const double t1 = trunc(5.9);
    static const double t2 = floor(5.9);
    static const double t3 = ceil(5.1);
    static const double t4 = round(5.5);
    
    /* Use in array sizes (via integer constant expressions) */
    char buffer1[(int)trunc(10.5)];
    char buffer2[(int)floor(10.5)];
    
    /* Use in static assertions */
    _Static_assert(trunc(5.9) == 5, "trunc failed");
    _Static_assert(floor(5.9) == 5, "floor failed");
    _Static_assert(ceil(5.1) == 6, "ceil failed");
    _Static_assert(round(5.5) == 6, "round failed");
    
    return (int)(t1 + t2 + t3 + t4);
}

/* Test 2: Nested calls and complex expressions */
int test_nested_calls(void) {
    /* Nested calls */
    double n1 = floor(ceil(g_input));
    double n2 = trunc(round(g_neg_input));
    double n3 = nearbyint(rint(g_input * 2.0));
    
    /* Calls as arguments to other calls */
    double n4 = round(trunc(g_input + 1.5));
    double n5 = ceil(floor(g_neg_input - 0.5));
    
    /* Mixed arithmetic with integer-valued calls */
    double n6 = (trunc(g_input) * 2.0) / floor(g_input + 1.0);
    double n7 = ceil(g_input) > floor(g_neg_input) ? 1.0 : 0.0;
    
    return (int)(n1 + n2 + n3 + n4 + n5 + n6 + n7);
}

/* Test 3: Builtin functions with different argument counts */
int test_builtins(void) {
    /* Builtins that may have different argument handling */
    long long ll1 = __builtin_llround(3.14);
    long long ll2 = __builtin_llrint(2.718);
    
    /* Complex number parts extraction */
    double complex z = 3.0 + 4.0 * I;
    double r1 = __real__(z);
    double r2 = __imag__(z);
    
    /* Use in constant expressions */
    enum { VAL1 = (int)__builtin_llround(3.14) };
    enum { VAL2 = (int)__builtin_llrint(2.718) };
    
    return (int)(ll1 + ll2 + r1 + r2 + VAL1 + VAL2);
}

/* Test 4: Conditional expressions with integer-valued calls */
int test_conditional_calls(void) {
    volatile int flag = 1;
    
    /* Conditional operator with integer-valued calls */
    double c1 = flag ? trunc(g_input) : floor(g_input);
    double c2 = (g_input > 0) ? ceil(g_input) : round(g_input);
    double c3 = (g_neg_input < 0) ? nearbyint(g_neg_input) : rint(g_neg_input);
    
    /* Nested conditionals */
    double c4 = (flag > 0) ? 
                (g_input > 3.0 ? floor(g_input) : ceil(g_input)) :
                trunc(g_input);
    
    return (int)(c1 + c2 + c3 + c4);
}

/* Test 5: Various argument types and values */
int test_various_arguments(void) {
    /* Integer arguments */
    double v1 = floor(5);
    double v2 = trunc(2);
    
    /* Exact integer real arguments */
    double v3 = ceil(4.0);
    double v4 = round(-2.0);
    
    /* Fractional arguments */
    double v5 = floor(4.7);
    double v6 = trunc(-3.3);
    
    /* Large values */
    double v7 = round(1e10 + 0.5);
    double v8 = nearbyint(1e15 - 0.2);
    
    /* Const variables as arguments */
    const double c1 = 7.8;
    const double c2 = -9.1;
    double v9 = ceil(c1);
    double v10 = floor(c2);
    
    return (int)(v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10);
}

/* Test 6: Loop bounds and comparisons */
int test_loops_and_comparisons(void) {
    int sum = 0;
    
    /* Use in loop bounds */
    int limit = (int)floor(fabs(g_input) + 2.0);
    for (int i = 0; i < limit; i++) {
        sum += i;
    }
    
    /* Use in if conditions */
    if (ceil(g_input) > floor(g_neg_input)) {
        sum += 100;
    }
    
    if (trunc(g_input * 2.0) == 6) {
        sum += 200;
    }
    
    /* Switch with computed case */
    switch ((int)round(g_input)) {
        case 3: sum += 300; break;
        case 4: sum += 400; break;
        default: sum += 500;
    }
    
    return sum;
}

/* C++ specific tests (compile with C++) */
#ifdef __cplusplus
#include <type_traits>

constexpr int cpp_test_constexpr() {
    /* constexpr math calls */
    constexpr double ct1 = std::trunc(5.9);
    constexpr double ct2 = std::floor(5.9);
    constexpr double ct3 = std::ceil(5.1);
    
    /* Use in template parameters */
    std::integral_constant<int, (int)std::round(3.5)> ic;
    
    return (int)(ct1 + ct2 + ct3 + ic.value);
}

template<int N>
struct TestTemplate {
    static const int value = N;
};

constexpr int cpp_test_templates() {
    /* Template argument with integer-valued call */
    TestTemplate<(int)std::floor(7.8)> t1;
    TestTemplate<(int)std::ceil(-3.2)> t2;
    
    return t1.value + t2.value;
}
#endif

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_builtins();
    checksum += test_conditional_calls();
    checksum += test_various_arguments();
    checksum += test_loops_and_comparisons();
    
#ifdef __cplusplus
    checksum += cpp_test_constexpr();
    checksum += cpp_test_templates();
#endif
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    return 0;
}
