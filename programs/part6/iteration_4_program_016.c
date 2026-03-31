/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Force compiler to use wide integer representations */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Static assertions with large constants - forces compile-time comparison */
#define STATIC_ASSERT(cond) _Static_assert(cond, #cond)

/* Large constants that require double_int representation */
static const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
static const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
static const int128_t HUGE_VAL = ((int128_t)0x7FFFFFFFFFFFFFFF << 64) | 0xFFFFFFFFFFFFFFFF;
static const uint128_t HUGE_UINT = ((uint128_t)1 << 127) | ((uint128_t)1 << 64);

/* Test 1: Basic compile-time comparisons with static assertions */
void test_static_comparisons(void) {
    /* These will force double_int::cmp during constant folding */
    STATIC_ASSERT(VERY_LARGE_POS > 0);
    STATIC_ASSERT(VERY_LARGE_NEG < 0);
    STATIC_ASSERT(VERY_LARGE_POS > VERY_LARGE_NEG);
    STATIC_ASSERT(HUGE_VAL > VERY_LARGE_POS);
    STATIC_ASSERT(HUGE_UINT > (uint128_t)VERY_LARGE_POS);
    
    /* Compare high parts specifically */
    STATIC_ASSERT(((int128_t)1 << 64) < ((int128_t)1 << 65));
    STATIC_ASSERT(((int128_t)1 << 127) > ((int128_t)1 << 126));
}

/* Test 2: Builtin overflow operations that use double_int internally */
void test_overflow_builtins(void) {
    long long a, b;
    long long result;
    int overflow;
    
    /* Test cases designed to trigger overflow checks with comparison */
    a = 0x7FFFFFFFFFFFFFFFLL; /* LLONG_MAX */
    b = 2;
    
    /* __builtin_mul_overflow uses double_int for overflow detection */
    overflow = __builtin_mul_overflow(a, b, &result);
    printf("mul_overflow test: %lld * 2 overflow? %s\n", 
           a, overflow ? "YES" : "NO");
    
    /* Force constant evaluation of overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(0x7FFFFFFFFFFFFFFFLL, 
                                                       2, 
                                                       0x7FFFFFFFFFFFFFFFLL))) {
        printf("Constant overflow check evaluated\n");
    }
    
    /* Test with different combinations to hit all comparison paths */
    int64_t vals[] = {1, -1, 0x7FFFFFFFFFFFFFFFLL, 0x8000000000000000LL};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            long long res;
            if (__builtin_add_overflow(vals[i], vals[j], &res)) {
                printf("Overflow in %lld + %lld\n", vals[i], vals[j]);
            }
        }
    }
}

/* Test 3: Range analysis with conditions that require double_int comparisons */
void test_range_analysis(void) {
    int x;
    
    /* Complex range analysis that requires comparing wide integers */
    if (x > 1000 && x < 2000) {
        /* Multiplication creates range that needs double_int for comparison */
        int64_t y = (int64_t)x * x;
        
        /* Nested conditions to force VRP to compare ranges */
        if (y > 1000000 && y < 4000000) {
            printf("Range analysis path 1\n");
        }
    }
    
    /* Test with large ranges */
    unsigned long long ull;
    if (ull > 0xFFFFFFFFFFFFFFFFULL / 2) {
        /* This comparison in VRP uses double_int */
        unsigned long long shifted = ull << 2;
        if (shifted > 0xFFFFFFFFFFFFFFFFULL) {
            printf("Large range overflow path\n");
        }
    }
}

/* Test 4: Template metaprogramming (C++ version) */
#ifdef __cplusplus

template <int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_large = N > ((int128_t)1 << 65);
    static const bool is_huge = N > ((int128_t)1 << 100);
    
    /* Force comparisons at compile time */
    static const int compare_to_mid = (N > ((int128_t)1 << 63)) ? 1 : 
                                      (N < ((int128_t)1 << 63)) ? -1 : 0;
};

/* Instantiate templates with various large values */
template struct LargeCompare<((int128_t)1 << 70)>;
template struct LargeCompare<-((int128_t)1 << 70)>;
template struct LargeCompare<((int128_t)1 << 127) - 1>;

#endif

/* Test 5: Direct operations on 128-bit integers */
void test_128bit_operations(void) {
    int128_t a = ((int128_t)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210;
    int128_t b = ((int128_t)0x123456789ABCDEF0 << 64) | 0xFEDCBA987654320F;
    int128_t c = ((int128_t)0x123456789ABCDEEF << 64) | 0xFEDCBA9876543210;
    
    /* These comparisons should use double_int::cmp */
    printf("128-bit comparisons:\n");
    printf("a > b: %s (expected: true)\n", a > b ? "true" : "false");
    printf("a < c: %s (expected: false)\n", a < c ? "true" : "false");
    printf("b < c: %s (expected: true)\n", b < c ? "true" : "false");
    
    /* Arithmetic that might trigger internal double_int operations */
    int128_t sum = a + b;
    int128_t diff = a - b;
    int128_t prod = a * 2;
    
    printf("sum > a: %s\n", sum > a ? "true" : "false");
    printf("diff > 0: %s\n", diff > 0 ? "true" : "false");
}

/* Test 6: Enumeration with large values */
enum big_enum : int128_t {
    BIG_ENUM_A = ((int128_t)1 << 70),
    BIG_ENUM_B = ((int128_t)1 << 71),
    BIG_ENUM_C = ((int128_t)1 << 72)
};

void test_enum_comparisons(void) {
    /* Comparisons between enum values use double_int */
    if (BIG_ENUM_B > BIG_ENUM_A) {
        printf("Enum comparison correct: B > A\n");
    }
    
    if (BIG_ENUM_C > BIG_ENUM_B) {
        printf("Enum comparison correct: C > B\n");
    }
}

/* Test 7: Shift operations beyond word size */
void test_large_shifts(void) {
    uint64_t base = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Shifts that create values requiring double_int representation */
    uint128_t shifted1 = (uint128_t)base << 32;
    uint128_t shifted2 = (uint128_t)base << 64;
    uint128_t shifted3 = (uint128_t)base << 96;
    
    /* Comparisons of these shifted values */
    printf("Shift comparisons:\n");
    printf("shifted2 > shifted1: %s\n", shifted2 > shifted1 ? "true" : "false");
    printf("shifted3 > shifted2: %s\n", shifted3 > shifted2 ? "true" : "false");
    
    /* Mixed signed/unsigned comparisons */
    int128_t signed_shifted = (int128_t)base << 32;
    printf("signed_shifted > 0: %s\n", signed_shifted > 0 ? "true" : "false");
}

/* Test 8: Complex constant expressions */
void test_complex_constants(void) {
    /* Multi-step constant expressions that require double_int comparisons */
    const int128_t complex1 = ((int128_t)1 << 70) + ((int128_t)1 << 60);
    const int128_t complex2 = ((int128_t)1 << 70) - ((int128_t)1 << 60);
    const int128_t complex3 = ((int128_t)1 << 70) * 3;
    
    /* Force compiler to evaluate these at compile time */
    if (__builtin_constant_p(complex1 > complex2)) {
        printf("Complex constant comparison evaluated at compile time\n");
    }
    
    /* Use in switch to force comparison */
    int128_t val = complex1;
    switch (val > complex2 ? 1 : 0) {
        case 1: printf("complex1 > complex2\n"); break;
        default: printf("unexpected\n");
    }
}

/* Main test driver */
int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Test 1: Static comparisons */
    printf("Test 1: Static comparisons\n");
    test_static_comparisons();
    printf("Static assertions passed\n\n");
    
    /* Test 2: Overflow builtins */
    printf("Test 2: Overflow builtins\n");
    test_overflow_builtins();
    printf("\n");
    
    /* Test 3: Range analysis */
    printf("Test 3: Range analysis (compile-time evaluation)\n");
    /* Note: This test's runtime behavior depends on uninitialized variables,
       but the important part is the compile-time analysis */
    printf("Range analysis structures compiled\n\n");
    
    /* Test 5: 128-bit operations */
    printf("Test 5: 128-bit operations\n");
    test_128bit_operations();
    printf("\n");
    
    /* Test 6: Enum comparisons */
    printf("Test 6: Enum comparisons\n");
    test_enum_comparisons();
    printf("\n");
    
    /* Test 7: Large shifts */
    printf("Test 7: Large shifts\n");
    test_large_shifts();
    printf("\n");
    
    /* Test 8: Complex constants */
    printf("Test 8: Complex constants\n");
    test_complex_constants();
    printf("\n");
    
    printf("=== All tests completed ===\n");
    
    return 0;
}
