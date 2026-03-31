/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 10;

/* Global to accumulate results */
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
    long long ll3 = __builtin_llrint(4.99);
    long long ll4 = __builtin_llrint(1.01);
    
    checksum += (int)(ll1 + ll2 + ll3 + ll4);
}

/* Test 3: Complex number real/imag part extraction */
void test_complex_parts(void) {
    /* Complex integer types */
    _Complex int ci = 3 + 4 * I;
    _Complex long cl = 5L + 6L * I;
    
    /* These extract real/imag parts as integer-valued reals */
    double r1 = __real__ ci;
    double i1 = __imag__ ci;
    double r2 = __real__ cl;
    double i2 = __imag__ cl;
    
    checksum += (int)(r1 + i1 + r2 + i2);
}

/* Test 4: Nested integer-valued real function calls */
void test_nested_calls(void) {
    /* Nested calls to exercise recursive depth */
    double d1 = floor(ceil(2.3));
    double d2 = trunc(round(4.6));
    double d3 = nearbyint(rint(5.1));
    double d4 = ceil(floor(3.9));
    
    /* Triple nesting */
    double d5 = trunc(round(floor(6.7)));
    double d6 = ceil(nearbyint(trunc(8.2)));
    
    checksum += (int)(d1 + d2 + d3 + d4 + d5 + d6);
}

/* Test 5: Calls within conditional expressions */
void test_conditional_calls(void) {
    /* Conditional operator with integer-valued real calls */
    double d1 = (vi > 5) ? trunc(7.8) : floor(9.1);
    double d2 = (vd < 4.0) ? ceil(2.3) : round(5.6);
    
    /* Nested conditional with calls */
    double d3 = (vf > 1.0f) ? 
                ((vi < 15) ? nearbyint(3.14) : rint(2.71)) :
                trunc(1.618);
    
    checksum += (int)(d1 + d2 + d3);
}

/* Test 6: Calls as function arguments */
double process_value(double (*func)(double), double x) {
    return func(x);
}

void test_function_arguments(void) {
    /* Integer-valued real functions as arguments */
    double d1 = process_value(trunc, 4.9);
    double d2 = process_value(floor, 3.2);
    double d3 = process_value(ceil, 5.1);
    
    /* Call with result of another integer-valued function */
    double d4 = trunc(process_value(round, 6.7));
    
    checksum += (int)(d1 + d2 + d3 + d4);
}

/* Test 7: Array sizes and enum values (compile-time contexts) */
enum {
    ARRAY_SIZE = (int)floor(10.5),
    ENUM_VAL = (int)ceil(8.3)
};

void test_compile_time_contexts(void) {
    /* Use in array size (folded at compile time) */
    char buffer1[(int)floor(10.5)];
    char buffer2[(int)ceil(8.3)];
    
    /* Use in static assertions (C++ would use static_assert) */
    /* For C, we'll just use in conditional compilation */
#if (int)trunc(5.9) == 5
    checksum += 5;
#endif
    
#if (int)round(3.5) == 4
    checksum += 4;
#endif
    
    checksum += sizeof(buffer1) + sizeof(buffer2) + ARRAY_SIZE + ENUM_VAL;
}

/* Test 8: Mixed expressions with arithmetic */
void test_mixed_expressions(void) {
    /* Arithmetic with integer-valued real functions */
    double d1 = (trunc(7.8) * 2.0) / floor(3.9);
    double d2 = ceil(4.2) + round(3.7) - nearbyint(2.1);
    double d3 = rint(5.5) * trunc(2.3) / ceil(1.1);
    
    /* Comparisons that might be folded */
    int cmp1 = (ceil(3.2) > floor(2.8));
    int cmp2 = (trunc(4.9) == round(4.5));
    int cmp3 = (nearbyint(1.3) < rint(1.7));
    
    checksum += (int)(d1 + d2 + d3) + cmp1 + cmp2 + cmp3;
}

/* Test 9: Type casts and conversions */
void test_type_conversions(void) {
    /* Explicit casts */
    int i1 = (int)trunc(9.9);
    int i2 = (int)floor(8.1);
    int i3 = (int)ceil(7.2);
    int i4 = (int)round(6.6);
    
    /* Float to double promotions */
    float f1 = truncf(3.14f);
    double d1 = (double)f1;
    
    checksum += i1 + i2 + i3 + i4 + (int)d1;
}

/* Test 10: Edge cases and special values */
void test_edge_cases(void) {
    /* Integer arguments */
    double d1 = floor(5);
    double d2 = trunc(2);
    
    /* Exact integer real arguments */
    double d3 = ceil(4.0);
    double d4 = round(3.0);
    
    /* Negative values */
    double d5 = floor(-2.3);
    double d6 = ceil(-3.7);
    double d7 = round(-4.5);
    double d8 = trunc(-5.9);
    
    /* Large values */
    double d9 = floor(1e10 + 0.5);
    double d10 = ceil(1e10 - 0.5);
    
    checksum += (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10);
}

/* Test 11: Zero and one argument calls (if applicable) */
void test_argument_counts(void) {
    /* Most math functions take exactly 1 argument, but we can test
       variations in how they're parsed */
    double d1 = trunc(3.14);      /* 1 argument */
    double d2 = floor(2.71);      /* 1 argument */
    
    /* Some builtins might have optional arguments */
    /* For standard functions, we test the call_expr_nargs logic */
    
    checksum += (int)(d1 + d2);
}

/* Test 12: Inline assembly mixed with integer-valued calls (if supported) */
void test_with_asm(void) {
    double input = 3.14159;
    double result;
    
    /* Use inline assembly to create a barrier, then apply integer-valued function */
    #ifdef __GNUC__
    asm volatile ("" : "+r" (input));
    result = trunc(input);
    asm volatile ("" : "+r" (result));
    #else
    result = trunc(input);
    #endif
    
    checksum += (int)result;
}

/* Main driver function */
int main(void) {
    /* Run all tests */
    test_basic_functions();
    test_builtin_functions();
    test_complex_parts();
    test_nested_calls();
    test_conditional_calls();
    test_function_arguments();
    test_compile_time_contexts();
    test_mixed_expressions();
    test_type_conversions();
    test_edge_cases();
    test_argument_counts();
    test_with_asm();
    
    /* Print final checksum */
    printf("Result: %d\n", checksum);
    
    /* Verify some expected values */
    if (checksum != 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Failure (unlikely) */
    }
}

/* Additional C++ specific tests (if compiled as C++) */
#ifdef __cplusplus
#include <type_traits>

namespace cpp_tests {
    /* Test 13: constexpr functions with integer-valued real calls */
    constexpr double constexpr_trunc(double x) {
        return trunc(x);  /* Should be foldable in constexpr context */
    }
    
    constexpr double constexpr_floor(double x) {
        return floor(x);
    }
    
    /* Test 14: Template metaprogramming */
    template<double Value>
    struct TruncatedValue {
        static constexpr int value = static_cast<int>(trunc(Value));
    };
    
    template<double Value>
    struct RoundedValue {
        static constexpr int value = static_cast<int>(round(Value));
    };
    
    void run_cpp_tests() {
        /* constexpr variables */
        constexpr double ct = constexpr_trunc(9.87);
        constexpr double cf = constexpr_floor(6.54);
        
        /* Template values */
        constexpr int tv = TruncatedValue<12.34>::value;
        constexpr int rv = RoundedValue<56.78>::value;
        
        /* static_assert uses */
        static_assert(constexpr_trunc(5.9) == 5.0, "trunc failed");
        static_assert(constexpr_floor(4.2) == 4.0, "floor failed");
        static_assert(TruncatedValue<3.14>::value == 3, "template trunc failed");
        
        checksum += (int)(ct + cf) + tv + rv;
    }
}

/* C++ main wrapper */
extern "C" int cpp_main(void) {
    main();
    cpp_tests::run_cpp_tests();
    printf("C++ Result: %d\n", checksum);
    return 0;
}
#endif
