/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Global volatile to prevent premature constant folding */
volatile double g_input = 3.14159;
volatile double g_input2 = 2.71828;

/* Test 1: Basic integer-valued real functions in constant contexts */
static int test_basic_functions(void) {
    /* Use in static initializers */
    static const double t1 = trunc(5.9);
    static const double t2 = floor(5.9);
    static const double t3 = ceil(5.1);
    static const double t4 = round(5.5);
    
    /* Use in array sizes (C99 VLA or C++ array) */
    char buffer1[(int)trunc(10.5)];
    char buffer2[(int)floor(10.5)];
    
    /* Use in static asserts */
    _Static_assert(trunc(5.9) == 5, "trunc failed");
    _Static_assert(floor(5.9) == 5, "floor failed");
    _Static_assert(ceil(5.1) == 6, "ceil failed");
    _Static_assert(round(5.5) == 6, "round failed");
    
    return (int)(t1 + t2 + t3 + t4);
}

/* Test 2: Nested calls and complex expressions */
static int test_nested_calls(void) {
    /* Nest calls */
    double n1 = floor(ceil(g_input));
    double n2 = trunc(round(g_input2));
    double n3 = nearbyint(rint(g_input));
    
    /* Calls as arguments to other calls */
    double n4 = round(trunc(g_input * 2.0));
    double n5 = ceil(floor(g_input2 / 2.0));
    
    /* Arithmetic with calls */
    double n6 = (trunc(g_input) * 2.0) / floor(g_input2);
    double n7 = nearbyint(g_input) + rint(g_input2);
    
    return (int)(n1 + n2 + n3 + n4 + n5 + n6 + n7);
}

/* Test 3: Conditional expressions with calls */
static int test_conditional_calls(void) {
    int condition = (g_input > 3.0);
    
    /* Ternary operator with calls */
    double c1 = condition ? trunc(g_input) : floor(g_input2);
    double c2 = (g_input2 < 3.0) ? ceil(g_input) : round(g_input2);
    
    /* Nested conditional with calls */
    double c3 = (condition) ? 
                (trunc(g_input) > 2.0 ? nearbyint(g_input) : rint(g_input2)) :
                floor(g_input2);
    
    return (int)(c1 + c2 + c3);
}

/* Test 4: Builtin functions with different argument counts */
static int test_builtin_calls(void) {
    /* Builtins that may have different argument counts */
    long long ll1 = __builtin_llround(g_input);
    long long ll2 = __builtin_llrint(g_input2);
    
    /* Complex number real/imag parts */
    complex int ci = 3 + 4I;
    double cr = __real__(ci);  /* Should be integer-valued real */
    double ci_imag = __imag__(ci);
    
    /* Mix with standard functions */
    double b1 = trunc(__real__(ci));
    double b2 = floor(cr + g_input);
    
    return (int)(ll1 + ll2 + cr + ci_imag + b1 + b2);
}

/* Test 5: Various argument types and values */
static int test_various_arguments(void) {
    /* Integer arguments */
    double v1 = floor(5);
    double v2 = trunc(2);
    
    /* Exact integer real arguments */
    double v3 = ceil(4.0);
    double v4 = round(100.0);
    
    /* Fractional arguments */
    double v5 = floor(4.7);
    double v6 = trunc(-3.8);
    
    /* Negative values */
    double v7 = round(-2.3);
    double v8 = ceil(-5.1);
    
    /* Large values */
    double v9 = trunc(1e10 + 0.5);
    double v10 = floor(1e15 - 0.1);
    
    return (int)(v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10);
}

/* Test 6: Calls in loop bounds and comparisons */
static int test_loop_comparisons(void) {
    int sum = 0;
    
    /* Use in loop bounds */
    int limit = (int)trunc(g_input * 2.0);
    for (int i = 0; i < limit; i++) {
        sum += i;
    }
    
    /* Use in comparisons */
    if (ceil(g_input) > floor(g_input2)) {
        sum += 100;
    }
    
    /* Switch with computed case */
    switch ((int)round(g_input)) {
        case 3: sum += 50; break;
        case 4: sum += 60; break;
        default: sum += 10;
    }
    
    return sum;
}

/* C++ specific tests (compile with g++) */
#ifdef __cplusplus
namespace cpp_tests {
    /* Test 7: constexpr functions with integer-valued real calls */
    constexpr double constexpr_floor(double x) {
        return floor(x);
    }
    
    constexpr double constexpr_trunc(double x) {
        return trunc(x);
    }
    
    /* Test 8: Template arguments with calls */
    template<int N>
    struct TestArray {
        char data[N];
    };
    
    /* Test 9: constexpr variables with calls */
    constexpr double ce1 = trunc(9.99);
    constexpr double ce2 = floor(8.01);
    
    static int test_cpp_features() {
        /* Use constexpr functions */
        constexpr double cf1 = constexpr_floor(7.8);
        constexpr double cf2 = constexpr_trunc(6.7);
        
        /* Use in template instantiation */
        TestArray<(int)ceil(5.5)> arr;
        
        /* Static assert with constexpr */
        static_assert(constexpr_floor(10.2) == 10, "C++ floor failed");
        static_assert(constexpr_trunc(10.9) == 10, "C++ trunc failed");
        
        return (int)(cf1 + cf2 + ce1 + ce2 + sizeof(arr));
    }
}
#endif

/* Main driver that accumulates results */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_calls();
    checksum += test_various_arguments();
    checksum += test_loop_comparisons();
    
    #ifdef __cplusplus
    checksum += cpp_tests::test_cpp_features();
    #endif
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    /* Additional verification */
    printf("Verification calls:\n");
    printf("trunc(3.14) = %.0f\n", trunc(3.14));
    printf("floor(3.14) = %.0f\n", floor(3.14));
    printf("ceil(3.14) = %.0f\n", ceil(3.14));
    printf("round(3.14) = %.0f\n", round(3.14));
    printf("nearbyint(3.14) = %.0f\n", nearbyint(3.14));
    printf("rint(3.14) = %.0f\n", rint(3.14));
    
    return 0;
}
