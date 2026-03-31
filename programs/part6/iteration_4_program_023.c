/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

/* ==================== SECTION 1: Large Integer Constants ==================== */

/* Test 1: Static assertions with very large 128-bit constants */
#define LARGE_CONST_1 (((__int128_t)1) << 70)      /* 2^70 */
#define LARGE_CONST_2 (((__int128_t)1) << 80)      /* 2^80 */
#define LARGE_CONST_3 (((__int128_t)1) << 90)      /* 2^90 */
#define LARGE_CONST_4 (((__int128_t)0x123456789ABCDEFULL) << 64 | 0xFEDCBA9876543210ULL)

/* Force compile-time comparisons through static assertions */
_Static_assert(LARGE_CONST_1 > 0, "Large constant 1 must be positive");
_Static_assert(LARGE_CONST_2 > LARGE_CONST_1, "2^80 > 2^70");
_Static_assert(LARGE_CONST_3 > LARGE_CONST_2, "2^90 > 2^80");
_Static_assert(LARGE_CONST_4 == LARGE_CONST_4, "Large constant equality");
_Static_assert(LARGE_CONST_3 != LARGE_CONST_2, "Large constant inequality");

/* Test 2: Template-like macro comparisons */
#define COMPARE_LARGE(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

/* Force evaluation at compile time */
static const int cmp_result_1 = COMPARE_LARGE(LARGE_CONST_2, LARGE_CONST_1);
static const int cmp_result_2 = COMPARE_LARGE(LARGE_CONST_1, LARGE_CONST_2);
static const int cmp_result_3 = COMPARE_LARGE(LARGE_CONST_4, LARGE_CONST_4);

/* ==================== SECTION 2: Builtin Overflow Operations ==================== */

/* Test 3: Overflow builtins that trigger double_int comparisons */
void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Case 1: Multiplication that would overflow 64-bit */
    a = LLONG_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MAX, 2): overflow=%d, res=%lld\n", overflow, res);
    
    /* Case 2: Addition that would overflow */
    a = LLONG_MAX;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("add_overflow(LLONG_MAX, 1): overflow=%d, res=%lld\n", overflow, res);
    
    /* Case 3: Subtraction that would underflow */
    a = LLONG_MIN;
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    printf("sub_overflow(LLONG_MIN, 1): overflow=%d, res=%lld\n", overflow, res);
    
    /* Case 4: Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, (long long)0))) {
        printf("Constant overflow check passed\n");
    }
}

/* Test 4: Complex overflow expressions */
void test_complex_overflow(void) {
    int64_t x = 1000000000;
    int64_t y = 1000000000;
    int64_t z;
    
    /* Chain of operations that require overflow analysis */
    if (!__builtin_mul_overflow(x, y, &z)) {
        /* If no overflow, check if result exceeds threshold */
        int64_t threshold = ((int64_t)1 << 62);
        if (z > threshold) {
            printf("Product %lld exceeds threshold %lld\n", (long long)z, (long long)threshold);
        }
    }
    
    /* Nested overflow checks */
    int64_t a = 0x7FFFFFFFFFFFFFFFLL;
    int64_t b = 2;
    int64_t c;
    if (__builtin_mul_overflow(a, b, &c)) {
        printf("Definite overflow detected\n");
    }
}

/* ==================== SECTION 3: Range Analysis Tests ==================== */

/* Test 5: Complex range calculations */
void test_range_analysis(int input) {
    /* Create known bounds */
    int x = input;
    
    /* First range restriction */
    if (x > 1000 && x < 10000) {
        /* Second range restriction */
        if (x % 2 == 0) {
            /* Compute something that requires range comparison */
            long long y = (long long)x * (long long)x;
            long long upper_bound = 50000000LL;
            
            /* This comparison may use double_int in VRP */
            if (y > upper_bound) {
                printf("y=%lld exceeds upper bound %lld\n", y, upper_bound);
            } else {
                printf("y=%lld within bounds\n", y);
            }
            
            /* Chain comparisons */
            if (y > 10000000LL && y < 40000000LL) {
                printf("y in middle range\n");
            }
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -10000) {
        long long y = (long long)x * (long long)x;
        if (y > 1000000LL) {
            printf("Negative x squared gives large positive: %lld\n", y);
        }
    }
}

/* Test 6: Loop induction variable analysis */
void test_loop_induction(void) {
    /* Loop with large step that may trigger wrap-around analysis */
    for (int64_t i = 0; i < LLONG_MAX - 1000; i += 0x7FFFFFFFFFFFFFLL) {
        /* The loop condition comparison may use double_int */
        if (i > LLONG_MAX / 2) {
            printf("i exceeded half of LLONG_MAX: %lld\n", (long long)i);
            break;
        }
    }
    
    /* Another loop with potential overflow in calculation */
    for (int j = 0; j < 100; j++) {
        int64_t val = j * 1000000000LL;
        if (val > INT_MAX) {
            printf("val %lld exceeds INT_MAX at j=%d\n", (long long)val, j);
        }
    }
}

/* ==================== SECTION 4: 128-bit Integer Operations ==================== */

/* Test 7: Direct 128-bit arithmetic and comparisons */
void test_128bit_operations(void) {
    /* Use __int128 type which directly maps to double_int */
    __int128 big1 = ((__int128_t)1) << 70;
    __int128 big2 = ((__int128_t)1) << 80;
    __int128 big3 = big1 * 1024;  /* Should equal 2^80 */
    
    /* These comparisons should trigger double_int::cmp */
    if (big1 < big2) {
        printf("Correct: 2^70 < 2^80\n");
    }
    
    if (big2 > big1) {
        printf("Correct: 2^80 > 2^70\n");
    }
    
    if (big2 == big3) {
        printf("Correct: 2^80 == 2^70 * 1024\n");
    }
    
    /* Test with negative 128-bit values */
    __int128 neg_big = -big1;
    if (neg_big < 0) {
        printf("Correct: -2^70 < 0\n");
    }
    
    if (neg_big < big1) {
        printf("Correct: -2^70 < 2^70\n");
    }
    
    /* Mixed comparisons */
    if (big1 > 0 && big2 > big1 && neg_big < 0) {
        printf("All 128-bit comparisons correct\n");
    }
}

/* Test 8: 128-bit constant expressions */
void test_128bit_constexpr(void) {
    /* Force constant folding with 128-bit values */
    const __int128 a = ((__int128_t)0x123456789ABCDEFULL) << 64;
    const __int128 b = ((__int128_t)0xFEDCBA987654321ULL) << 64;
    const __int128 c = a + b;
    
    /* These should be evaluated at compile time */
    if (__builtin_constant_p(a > b)) {
        printf("128-bit constant comparison folded at compile time\n");
    }
    
    /* Use in switch to force comparison */
    __int128 val = a;
    switch (val == a ? 1 : 0) {
        case 1: printf("128-bit equality comparison worked\n"); break;
        default: printf("Unexpected\n"); break;
    }
}

/* ==================== SECTION 5: Template Metaprogramming (C++ only) ==================== */

#ifdef __cplusplus
#include <type_traits>

/* Test 9: Template with large integer values */
template <__int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_large = N > ((__int128_t)1 << 65);
    static const bool is_very_large = N > ((__int128_t)1 << 100);
    
    static const int compare_to_2_70 = (N > ((__int128_t)1 << 70)) ? 1 : 
                                      ((N < ((__int128_t)1 << 70)) ? -1 : 0);
};

/* Instantiate templates with various large values */
template struct LargeCompare<((__int128_t)1) << 60>;   /* 2^60 */
template struct LargeCompare<((__int128_t)1) << 70>;   /* 2^70 */
template struct LargeCompare<((__int128_t)1) << 80>;   /* 2^80 */
template struct LargeCompare<((__int128_t)-1) << 70>;  /* -2^70 */

/* Test 10: Compile-time computation with large integers */
template <__int128_t A, __int128_t B>
struct CompareValues {
    static const int result = (A > B) ? 1 : ((A < B) ? -1 : 0);
    static const bool equal = (A == B);
    static const bool not_equal = (A != B);
    static const bool less = (A < B);
    static const bool greater = (A > B);
};

/* Force instantiation and use */
static const int cmp1 = CompareValues<((__int128_t)1) << 75, 
                                      ((__int128_t)1) << 70>::result;
static const bool cmp2 = CompareValues<((__int128_t)1) << 80,
                                      ((__int128_t)1) << 80>::equal;

#endif /* __cplusplus */

/* ==================== SECTION 6: Enum and Bit-field Tests ==================== */

/* Test 11: Large enumeration values */
enum big_enum : unsigned long long {
    BIG_VAL1 = 0xFFFFFFFFFFFFFFFFULL,
    BIG_VAL2 = 0xFFFFFFFFFFFFFFFEULL
};

/* Test comparisons between large enum values */
void test_enum_comparisons(void) {
    enum big_enum e1 = BIG_VAL1;
    enum big_enum e2 = BIG_VAL2;
    
    if (e1 > e2) {
        printf("Enum comparison: BIG_VAL1 > BIG_VAL2\n");
    }
    
    /* Force constant folding with enum */
    if (BIG_VAL1 > BIG_VAL2) {
        printf("Constant enum comparison folded\n");
    }
}

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    printf("=== Starting double_int::cmp coverage tests ===\n\n");
    
    /* Verify compile-time comparisons worked */
    printf("Compile-time comparison results:\n");
    printf("  cmp_result_1 (2^80 > 2^70): %d (expected: 1)\n", cmp_result_1);
    printf("  cmp_result_2 (2^70 < 2^80): %d (expected: -1)\n", cmp_result_2);
    printf("  cmp_result_3 (equality): %d (expected: 0)\n", cmp_result_3);
    
    assert(cmp_result_1 == 1);
    assert(cmp_result_2 == -1);
    assert(cmp_result_3 == 0);
    
    printf("\n=== Testing overflow builtins ===\n");
    test_overflow_builtins();
    test_complex_overflow();
    
    printf("\n=== Testing range analysis ===\n");
    test_range_analysis(2000);
    test_range_analysis(-5000);
    
    printf("\n=== Testing loop induction ===\n");
    test_loop_induction();
    
    printf("\n=== Testing 128-bit operations ===\n");
    test_128bit_operations();
    test_128bit_constexpr();
    
    printf("\n=== Testing enum comparisons ===\n");
    test_enum_comparisons();
    
#ifdef __cplusplus
    printf("\n=== Testing template metaprogramming ===\n");
    printf("Template comparisons instantiated at compile time\n");
#endif
    
    printf("\n=== All tests completed successfully ===\n");
    
    return 0;
}
