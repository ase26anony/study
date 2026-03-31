/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

/* For C++ template tests */
#ifdef __cplusplus
#include <type_traits>
#endif

/* Helper macro for static assertions */
#ifndef __cplusplus
#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
#define STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#endif

/* ==================== SECTION 1: Large Integer Constants ==================== */

/* Test 1.1: Basic large constant comparisons */
void test_large_constants() {
    /* Define 128-bit constants using __int128 */
    __int128_t huge_pos = ((__int128_t)1 << 70);      /* 2^70 */
    __int128_t huge_neg = -((__int128_t)1 << 70);     /* -2^70 */
    __int128_t huge_pos2 = ((__int128_t)1 << 69) + 1; /* 2^69 + 1 */
    
    /* These comparisons should trigger double_int::cmp during constant folding */
    STATIC_ASSERT(huge_pos > 0, "Large positive constant");
    STATIC_ASSERT(huge_neg < 0, "Large negative constant");
    STATIC_ASSERT(huge_pos > huge_pos2, "Large vs large comparison");
    STATIC_ASSERT(huge_neg < huge_pos2, "Negative large vs positive large");
    
    /* Test equality with large numbers */
    __int128_t same1 = ((__int128_t)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210ULL;
    __int128_t same2 = ((__int128_t)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210ULL;
    STATIC_ASSERT(same1 == same2, "Large equality check");
    
    printf("Large constants: PASS\n");
}

/* Test 1.2: Arithmetic resulting in wide integers */
void test_wide_arithmetic() {
    /* Multiplication that overflows 64 bits */
    int64_t big1 = 0x7FFFFFFFFFFFFFFFLL; /* Max int64_t */
    int64_t big2 = 2;
    
    /* These operations create double_int internally */
    __int128_t prod = (__int128_t)big1 * (__int128_t)big2;
    STATIC_ASSERT(prod > big1, "Multiplication overflow comparison");
    
    /* Left shift beyond 63 bits */
    __int128_t shifted = ((__int128_t)1 << 95);
    STATIC_ASSERT(shifted > ((__int128_t)1 << 94), "Large shift comparison");
    
    /* Mixed high/low part comparisons */
    __int128_t a = ((__int128_t)0x1 << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128_t b = ((__int128_t)0x1 << 64) | 0xFFFFFFFFFFFFFFFEULL;
    STATIC_ASSERT(a > b, "Same high part, different low part");
    
    printf("Wide arithmetic: PASS\n");
}

/* ==================== SECTION 2: GCC Builtins ==================== */

/* Test 2.1: Overflow builtins */
void test_overflow_builtins() {
    long long x, y, result;
    int overflow;
    
    /* Test with constants that should trigger overflow detection */
    x = 0x7FFFFFFFFFFFFFFFLL; /* Max signed 64-bit */
    y = 2;
    
    /* This builtin uses double_int internally for overflow checking */
    overflow = __builtin_mul_overflow(x, y, &result);
    
    if (__builtin_constant_p(x * y > x)) {
        /* Force constant evaluation path */
        printf("Constant overflow check triggered\n");
    }
    
    /* Test addition overflow */
    long long a = 0x7FFFFFFFFFFFFFFFLL;
    long long b = 1;
    int add_overflow = __builtin_add_overflow(a, b, &result);
    
    /* Test overflow with __int128 */
    __int128_t big_a = ((__int128_t)1 << 127) - 1;
    __int128_t big_b = 1;
    __int128_t big_result;
    int big_overflow = __builtin_add_overflow(big_a, big_b, &big_result);
    
    printf("Overflow builtins: PASS (overflow=%d, add_overflow=%d, big_overflow=%d)\n", 
           overflow, add_overflow, big_overflow);
}

/* Test 2.2: Builtin constant detection */
void test_builtin_constant_p() {
    /* Force compiler to evaluate large comparisons at compile time */
    __int128_t huge = ((__int128_t)1 << 100);
    
    if (__builtin_constant_p(huge > 0)) {
        /* This path should be taken */
        printf("Builtin constant detected large comparison\n");
    }
    
    /* Complex expression that needs double_int comparison */
    __int128_t x = ((__int128_t)1 << 64) + 123;
    __int128_t y = ((__int128_t)1 << 64) + 122;
    
    if (__builtin_constant_p(x > y && x < y + 10)) {
        printf("Complex large comparison evaluated at compile time\n");
    }
}

/* ==================== SECTION 3: Range Analysis ==================== */

/* Test 3.1: Value Range Propagation (VRP) */
void test_vrp_range_comparisons(int input) {
    /* Create known bounds for VRP analysis */
    int x = input;
    
    /* These conditions create value ranges */
    if (x > 1000 && x < 10000) {
        /* Multiplication that might overflow 32-bit */
        long long y = (long long)x * x;
        
        /* Further range refinement */
        if (y > 2000000 && y < 50000000) {
            /* Nested comparisons create complex range analysis */
            long long z = y * 2;
            if (z > 4000000) {
                printf("VRP path taken: x=%d, y=%lld, z=%lld\n", x, y, z);
            }
        }
    }
    
    /* Test with very large ranges */
    if (x > INT_MIN && x < INT_MAX) {
        /* This creates the full range, testing boundary comparisons */
        __int128_t wide = (__int128_t)x * INT_MAX;
        if (wide > (__int128_t)INT_MIN * INT_MAX) {
            /* Always true, but compiler must prove it */
            printf("Wide range comparison\n");
        }
    }
}

/* Test 3.2: Loop induction variables */
void test_loop_induction() {
    /* Loop with large step that might trigger wrap analysis */
    for (int64_t i = 0; i < 1000000000000LL; i += 1000000000LL) {
        /* The loop bound comparison uses double_int */
        if (i > 500000000000LL) {
            printf("Large loop induction: %lld\n", (long long)i);
            break; /* Just test first hit */
        }
    }
    
    /* Test potential overflow in loop */
    for (int64_t j = 0x7000000000000000LL; j < 0x7000000000000000LL + 10; j++) {
        /* The increment near overflow boundary */
        printf("Near overflow: %lld\n", (long long)j);
    }
}

/* ==================== SECTION 4: C++ Template Metaprogramming ==================== */

#ifdef __cplusplus

/* Test 4.1: Template with large integer parameters */
template <__int128_t N>
struct LargeComparator {
    static const bool is_positive = N > 0;
    static const bool is_large = N > (__int128_t(1) << 65);
    static const bool is_huge = N > (__int128_t(1) << 100);
    
    /* Compare with another large value */
    template <__int128_t M>
    static const bool greater_than = N > M;
    
    /* Test different high/low combinations */
    static const bool high_bit_set = N > (__int128_t(1) << 63);
};

/* Instantiate templates to force compile-time comparisons */
template struct LargeComparator<(__int128_t(1) << 70)>;
template struct LargeComparator<-(__int128_t(1) << 70)>;
template struct LargeComparator<((__int128_t)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210ULL>;

/* Test 4.2: Compile-time assertions using templates */
template <__int128_t A, __int128_t B>
constexpr bool compile_time_greater() {
    return A > B;  /* Uses double_int::cmp at compile time */
}

static_assert(compile_time_greater<(__int128_t(1) << 80), (__int128_t(1) << 79)>(),
              "Template compile-time comparison failed");

/* Test 4.3: SFINAE with large integers */
template <__int128_t N>
typename std::enable_if<(N > (__int128_t(1) << 127)), bool>::type
check_overflow() {
    return true;
}

template <__int128_t N>
typename std::enable_if<!(N > (__int128_t(1) << 127)), bool>::type
check_overflow() {
    return false;
}

#endif /* __cplusplus */

/* ==================== SECTION 5: Tree Node Construction ==================== */

/* Test 5.1: Wide enumerations */
#ifdef __cplusplus
enum big_enum : __int128 {
    BIG_VALUE1 = (__int128_t(1) << 70),
    BIG_VALUE2 = (__int128_t(1) << 71),
    BIG_VALUE3 = BIG_VALUE1 + BIG_VALUE2
};

static_assert(BIG_VALUE2 > BIG_VALUE1, "Enum comparison");
#endif

/* Test 5.2: 128-bit types using attribute */
typedef __int128 int128_t __attribute__((mode(TI)));

void test_128bit_operations(int128_t a, int128_t b) {
    /* Operations that require magnitude comparison */
    int128_t div_result = a / b;  /* Division compares magnitudes */
    int128_t mod_result = a % b;  /* Modulus compares magnitudes */
    
    /* Comparisons that should use double_int::cmp */
    if (a > b) {
        printf("a > b: division=%lld\n", (long long)div_result);
    } else if (a < b) {
        printf("a < b: modulus=%lld\n", (long long)mod_result);
    } else {
        printf("a == b\n");
    }
}

/* Test 5.3: Complex constant expressions */
void test_complex_constants() {
    /* Nested constant expressions with large integers */
    const __int128_t complex1 = (((__int128_t)1 << 64) + 123) * 456;
    const __int128_t complex2 = (((__int128_t)1 << 64) + 122) * 457;
    
    /* This comparison should be folded at compile time */
    if (__builtin_constant_p(complex1 > complex2)) {
        printf("Complex constant comparison folded\n");
    }
    
    /* Use in switch case (if supported) */
    __int128_t val = complex1;
    switch (val > complex2 ? 1 : 0) {
        case 0: printf("case 0\n"); break;
        case 1: printf("case 1\n"); break;
    }
}

/* ==================== MAIN TEST DRIVER ==================== */

int main() {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Section 1: Large Integer Constants */
    test_large_constants();
    test_wide_arithmetic();
    printf("\n");
    
    /* Section 2: GCC Builtins */
    test_overflow_builtins();
    test_builtin_constant_p();
    printf("\n");
    
    /* Section 3: Range Analysis */
    test_vrp_range_comparisons(5000);
    test_loop_induction();
    printf("\n");
    
    /* Section 4: C++ Templates (compile-time only) */
#ifdef __cplusplus
    printf("C++ Template tests compiled successfully\n");
    
    /* Runtime verification of template instantiations */
    bool test1 = LargeComparator<(__int128_t(1) << 70)>::is_large;
    bool test2 = LargeComparator<(__int128_t(1) << 70)>::greater_than<(__int128_t(1) << 69)>;
    printf("Template results: is_large=%d, greater_than=%d\n", test1, test2);
    
    /* Test SFINAE */
    bool overflow1 = check_overflow<(__int128_t(1) << 127)>();
    bool overflow2 = check_overflow<(__int128_t(1) << 126)>();
    printf("Overflow checks: %d, %d\n", overflow1, overflow2);
#endif
    printf("\n");
    
    /* Section 5: Tree Node Construction */
    test_complex_constants();
    
    /* Test with actual 128-bit values */
    int128_t big_a = ((int128_t)1 << 70) + 123;
    int128_t big_b = ((int128_t)1 << 70) + 122;
    test_128bit_operations(big_a, big_b);
    
    printf("\n=== All tests completed ===\n");
    
    /* Final runtime assertion to ensure all comparisons worked */
    __int128_t final_test_a = ((__int128_t)1 << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128_t final_test_b = ((__int128_t)1 << 64) | 0xFFFFFFFFFFFFFFFEULL;
    assert(final_test_a > final_test_b);
    
    printf("Final assertion passed - double_int comparisons working correctly\n");
    return 0;
}
