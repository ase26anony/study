/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. CONSTANT FOLDING WITH LARGE INTEGERS ========== */

/* Test 1a: Static assertions with 128-bit constants */
#ifdef __SIZEOF_INT128__
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Large constants that require double_int representation */
const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
const int128_t HUGE_PRODUCT = ((int128_t)0x7FFFFFFFFFFFFFFFLL * 0x7FFFFFFFFFFFFFFFLL);

/* Static assertions force compile-time comparison */
_Static_assert(VERY_LARGE_POS > 0, "Large positive constant");
_Static_assert(VERY_LARGE_NEG < 0, "Large negative constant");
_Static_assert(HUGE_PRODUCT > VERY_LARGE_POS, "Product comparison");
_Static_assert(VERY_LARGE_POS != VERY_LARGE_NEG, "Inequality check");

/* Test 1b: Builtin constant evaluation */
static int test_constant_folding(void) {
    int result = 0;
    
    /* Force compiler to evaluate comparisons at compile time */
    if (__builtin_constant_p(VERY_LARGE_POS > 1000)) {
        result |= 1;
    }
    
    if (__builtin_constant_p(VERY_LARGE_NEG < -1000)) {
        result |= 2;
    }
    
    /* Complex expression requiring double_int comparison */
    int128_t shifted = ((int128_t)1 << 65);
    if (__builtin_constant_p(shifted > INT64_MAX)) {
        result |= 4;
    }
    
    return result;
}
#endif

/* ========== 2. GCC BUILTINS WITH OVERFLOW DETECTION ========== */

/* Test 2a: Multiplication overflow checks */
static void test_mul_overflow(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* These multiplications will overflow on 64-bit systems */
    a = 0x7FFFFFFFFFFFFFFFLL; /* LLONG_MAX */
    b = 2;
    
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Mul overflow test 1: %d (expected 1)\n", overflow);
    
    /* Compare overflow results - internal double_int comparison */
    a = 0x3FFFFFFFFFFFFFFFLL;
    b = 0x3FFFFFFFFFFFFFFFLL;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Mul overflow test 2: %d (expected 1)\n", overflow);
    
    /* Non-overflowing case for comparison */
    a = 1000;
    b = 1000;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Mul overflow test 3: %d (expected 0)\n", overflow);
}

/* Test 2b: Addition overflow checks */
static void test_add_overflow(void) {
    long long a, b;
    long long res;
    
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 1;
    
    int overflow1 = __builtin_add_overflow(a, b, &res);
    printf("Add overflow test: %d (expected 1)\n", overflow1);
    
    /* Force comparison of overflow results */
    a = -0x7FFFFFFFFFFFFFFFLL - 1;
    b = -1;
    int overflow2 = __builtin_add_overflow(a, b, &res);
    printf("Add underflow test: %d (expected 1)\n", overflow2);
}

/* ========== 3. RANGE CALCULATIONS AND VRP ========== */

/* Test 3a: Complex range analysis */
static void test_range_analysis(int x) {
    /* Create known bounds for VRP */
    if (x > 1000 && x < 2000) {
        /* These operations create ranges that need double_int comparison */
        long long y = (long long)x * x;
        long long z = y * 2;
        
        /* Comparisons that trigger range analysis */
        if (y > 1000000) {
            printf("Range test 1 passed: y = %lld\n", y);
        }
        
        if (z < 8000000) {
            printf("Range test 2 passed: z = %lld\n", z);
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -2000) {
        long long y = (long long)x * x;
        if (y > 1000000) {
            printf("Negative range test passed: y = %lld\n", y);
        }
    }
}

/* Test 3b: Loop induction variable analysis */
static void test_loop_induction(void) {
    /* Loop with large step that may trigger wrap analysis */
    for (long long i = 0; i < 0x7FFFFFFFFFFFFFFFLL; i += 0x1000000000000000LL) {
        /* This comparison in loop analysis may use double_int */
        if (i > 0x4000000000000000LL) {
            printf("Loop induction: i = %lld\n", i);
            break;
        }
    }
}

/* ========== 4. TEMPLATE METAPROGRAMMING (C++ VERSION) ========== */
#ifdef __cplusplus

template <int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_large = N > (int128_t(1) << 65);
    static const bool is_huge = N > (int128_t(1) << 100);
    
    /* Force instantiation with different comparisons */
    static const int compare_to_mid = (N > (int128_t(1) << 63)) ? 1 : 
                                      (N < (int128_t(1) << 63)) ? -1 : 0;
};

/* Instantiate templates to force compile-time comparisons */
template struct LargeCompare<(int128_t)1 << 70>;
template struct LargeCompare<-(int128_t)1 << 70>;
template struct LargeCompare<(int128_t)0x7FFFFFFFFFFFFFFFLL * 2>;

#endif

/* ========== 5. TREE NODE CONSTRUCTION ========== */

/* Test 5a: Wide enumerations */
#ifdef __SIZEOF_INT128__
enum big_enum : int128_t {
    BIG_ENUM_A = (int128_t)1 << 65,
    BIG_ENUM_B = (int128_t)1 << 66,
    BIG_ENUM_C = BIG_ENUM_A * 2
};

/* Test 5b: Operations on wide constants */
static int128_t operate_on_big_constants(void) {
    int128_t a = (int128_t)0xFFFFFFFFFFFFFFFFULL;
    int128_t b = a << 10;
    int128_t c = b / 1024;
    
    /* These comparisons create INTEGER_CST nodes */
    if (b > a) {
        printf("Shift comparison passed\n");
    }
    
    if (c == a) {
        printf("Division comparison passed\n");
    }
    
    return c;
}
#endif

/* ========== MAIN TEST DRIVER ========== */

int main(void) {
    int total_score = 0;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
#ifdef __SIZEOF_INT128__
    printf("1. Testing constant folding...\n");
    int cf_result = test_constant_folding();
    printf("   Constant folding result: %d (expected 7)\n", cf_result);
    if (cf_result == 7) total_score++;
    
    printf("\n2. Testing overflow builtins...\n");
    test_mul_overflow();
    test_add_overflow();
    total_score++;
#endif
    
    printf("\n3. Testing range analysis...\n");
    test_range_analysis(1500);
    test_range_analysis(-1500);
    total_score++;
    
    printf("\n4. Testing loop induction analysis...\n");
    test_loop_induction();
    total_score++;
    
#ifdef __SIZEOF_INT128__
    printf("\n5. Testing wide constant operations...\n");
    operate_on_big_constants();
    total_score++;
#endif
    
    printf("\n=== Summary ===\n");
    printf("Tests completed: %d\n", total_score);
    
    /* Final validation using large constant comparison */
#ifdef __SIZEOF_INT128__
    const int128_t FINAL_TEST_A = ((int128_t)1 << 72) + 1;
    const int128_t FINAL_TEST_B = ((int128_t)1 << 72);
    
    /* This should trigger double_int::cmp during compilation */
    if (FINAL_TEST_A > FINAL_TEST_B) {
        printf("FINAL VALIDATION: PASS\n");
        return 0;
    } else {
        printf("FINAL VALIDATION: FAIL\n");
        return 1;
    }
#else
    printf("128-bit support not available - partial test only\n");
    return 0;
#endif
}
