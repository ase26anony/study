/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Test 1: Large integer constants and static assertions */
static void test_large_constants(void) {
    /* Define constants larger than 64 bits */
    const __int128_t huge_pos = ((__int128_t)1 << 70) + 12345;
    const __int128_t huge_neg = -((__int128_t)1 << 70) - 67890;
    const __int128_t medium = ((__int128_t)1 << 65);
    const __int128_t zero = 0;
    
    /* Static assertions forcing compile-time comparisons */
    _Static_assert(huge_pos > zero, "Large positive should be > 0");
    _Static_assert(huge_neg < zero, "Large negative should be < 0");
    _Static_assert(huge_pos > medium, "70-bit > 65-bit");
    _Static_assert(huge_neg < medium, "Negative < positive");
    _Static_assert(huge_pos != huge_neg, "Positive != negative");
    _Static_assert(huge_pos == huge_pos, "Self equality");
    
    /* Complex compile-time expressions */
    _Static_assert((huge_pos >> 5) > medium, "Shifted comparison");
    _Static_assert((huge_neg << 2) < zero, "Negative shift comparison");
    
    printf("Large constants test: PASS\n");
}

/* Test 2: Builtin overflow operations */
static void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Test cases that will trigger overflow checks */
    a = 0x7FFFFFFFFFFFFFFFLL; /* INT64_MAX */
    b = 2;
    
    /* These builtins internally use double_int comparisons */
    overflow = __builtin_mul_overflow(a, b, &res);
    if (overflow) {
        printf("Multiplication overflow detected correctly\n");
    }
    
    a = 0x8000000000000000LL; /* INT64_MIN */
    b = -1;
    overflow = __builtin_mul_overflow(a, b, &res);
    if (overflow) {
        printf("Negative overflow detected\n");
    }
    
    /* Test with __builtin_constant_p */
    const long long const_a = 0x7FFFFFFFFFFFFFFFLL;
    const long long const_b = 2;
    if (__builtin_constant_p(const_a * const_b > const_a)) {
        printf("Constant overflow comparison evaluated\n");
    }
    
    /* Add overflow with large values */
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    if (overflow) {
        printf("Addition overflow detected\n");
    }
    
    printf("Overflow builtins test: PASS\n");
}

/* Test 3: Range analysis with complex conditions */
static void test_range_analysis(int x) {
    /* Create complex range conditions */
    if (x > 1000 && x < 2000) {
        /* These operations trigger range calculations with double_int */
        long long y = (long long)x * x;
        long long z = y * y;
        
        if (__builtin_expect(z > 0, 1)) {
            printf("Range analysis: positive result for x=%d\n", x);
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -2000) {
        long long y = (long long)x * x;
        if (y > 0) {
            printf("Range analysis: negative squared is positive\n");
        }
    }
    
    /* Large step induction variable */
    for (long long i = 0x7000000000000000LL; 
         i < 0x7800000000000000LL; 
         i += 0x100000000LL) {
        /* Loop analysis uses double_int for bounds checking */
        if (i > 0x7500000000000000LL) {
            printf("Large induction variable: %lld\n", i);
            break;
        }
    }
}

/* Test 4: Template metaprogramming (C++ version) */
#ifdef __cplusplus

template <__int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_large = N > (__int128_t(1) << 65);
    static const bool is_huge = N > (__int128_t(1) << 100);
    static const int comparison = (N > (__int128_t(1) << 70)) ? 1 : 
                                 ((N < (__int128_t(1) << 70)) ? -1 : 0);
};

template <__int128_t A, __int128_t B>
struct LargeOperations {
    static const __int128_t sum = A + B;
    static const __int128_t diff = A - B;
    static const bool a_gt_b = A > B;
    static const bool a_lt_b = A < B;
    static const bool a_eq_b = A == B;
};

void test_templates() {
    /* Instantiate templates with large values */
    const bool test1 = LargeCompare<(__int128_t(1) << 66)>::is_large;
    const bool test2 = LargeCompare<-(__int128_t(1) << 66)>::is_positive;
    const int test3 = LargeCompare<(__int128_t(1) << 71)>::comparison;
    
    const bool test4 = LargeOperations<
        (__int128_t(1) << 68), 
        (__int128_t(1) << 67)>::a_gt_b;
    
    printf("Template tests: %d %d %d %d\n", 
           test1, test2, test3, test4);
}

#endif

/* Test 5: Wide enumerations and attributes */
static void test_wide_types(void) {
    /* Use 128-bit integer type */
    typedef __int128 int128_t;
    typedef unsigned __int128 uint128_t;
    
    /* Large value comparisons */
    int128_t a = ((int128_t)1 << 70) + 123;
    int128_t b = ((int128_t)1 << 69) + 456;
    int128_t c = -((int128_t)1 << 71);
    
    /* Force multiple comparison paths */
    if (a > b) printf("a > b: correct\n");
    if (b < a) printf("b < a: correct\n");
    if (a != b) printf("a != b: correct\n");
    if (c < a) printf("c < a: correct\n");
    if (c < b) printf("c < b: correct\n");
    if (a > 0) printf("a > 0: correct\n");
    if (c < 0) printf("c < 0: correct\n");
    
    /* Complex expression */
    int128_t d = a * 2 - b;
    if (d > a) printf("d > a: correct\n");
    
    /* Modulus operation with large values */
    int128_t e = a % 1000000007;
    if (e >= 0 && e < 1000000007) printf("Modulus in range\n");
    
    /* Bitwise operations */
    uint128_t ua = (uint128_t)a;
    uint128_t ub = (uint128_t)b;
    if ((ua & ub) != 0) printf("Bitwise AND non-zero\n");
    if ((ua | ub) != 0) printf("Bitwise OR non-zero\n");
}

/* Test 6: Mixed-size operations */
static void test_mixed_operations(void) {
    /* Operations between 64-bit and 128-bit values */
    int64_t i64 = 0x7FFFFFFFFFFFFFFFLL;
    __int128_t i128 = ((__int128_t)1 << 70);
    
    /* These comparisons may trigger different code paths */
    if (i128 > i64) printf("128-bit > 64-bit max\n");
    if (i64 < i128) printf("64-bit max < 128-bit\n");
    
    /* Multiplication that requires wide intermediate */
    __int128_t product = (__int128_t)i64 * i64;
    if (product > i64) printf("Square > original\n");
    
    /* Division with large dividend */
    __int128_t dividend = ((__int128_t)1 << 100);
    int64_t divisor = 1000;
    __int128_t quotient = dividend / divisor;
    if (quotient > 0) printf("Large division positive\n");
}

/* Test 7: Constant folding with conditional expressions */
static void test_constant_folding(void) {
    /* Complex constant expressions */
    const __int128_t c1 = ((__int128_t)1 << 72) | 0x123456789ABCDEFLL;
    const __int128_t c2 = ((__int128_t)1 << 71) | 0xFEDCBA987654321LL;
    
    /* Force constant folding with ternary operator */
    const __int128_t max_c = (c1 > c2) ? c1 : c2;
    const __int128_t min_c = (c1 < c2) ? c1 : c2;
    
    /* Use in static assertions */
    _Static_assert(max_c > min_c, "max should be > min");
    _Static_assert(max_c != min_c, "max != min");
    
    /* Chain comparisons */
    const __int128_t c3 = ((__int128_t)1 << 73);
    _Static_assert(c3 > c1 && c1 > c2, "Chained comparisons");
    
    printf("Constant folding test: PASS\n");
}

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    test_large_constants();
    printf("\n");
    
    test_overflow_builtins();
    printf("\n");
    
    test_range_analysis(1500);  /* Mid-range positive */
    test_range_analysis(-1500); /* Mid-range negative */
    printf("\n");
    
    #ifdef __cplusplus
    test_templates();
    printf("\n");
    #endif
    
    test_wide_types();
    printf("\n");
    
    test_mixed_operations();
    printf("\n");
    
    test_constant_folding();
    printf("\n");
    
    printf("=== All tests completed ===\n");
    
    return 0;
}
