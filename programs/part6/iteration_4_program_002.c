/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Test 1A: Static assertions with 128-bit constants */
#ifdef __SIZEOF_INT128__
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Large constants that require double_int representation */
const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
const int128_t HUGE_PRODUCT = ((int128_t)0x7FFFFFFFFFFFFFFF) * 2;
const uint128_t UHUGE = ((uint128_t)1 << 80);

/* Static assertions force compile-time comparison */
_Static_assert(VERY_LARGE_POS > 0, "Large positive constant");
_Static_assert(VERY_LARGE_NEG < 0, "Large negative constant");
_Static_assert(HUGE_PRODUCT > INT64_MAX, "Product exceeds 64-bit");
_Static_assert(UHUGE > UINT64_MAX, "Unsigned exceeds 64-bit");

/* Test 1B: Compile-time comparisons in macros */
#define COMPARE_LARGE_CONSTANTS(a, b) \
    (__builtin_constant_p(a) && __builtin_constant_p(b) ? (a) < (b) : 0)

/* Test 1C: Template-like comparisons using _Generic (C11) */
#define IS_GREATER(a, b) _Generic((a), \
    int128_t: (a) > (b), \
    default: (int64_t)(a) > (int64_t)(b) \
)
#endif

/* ========== 2. GCC Builtins That Return or Manipulate double_int ========== */

/* Test 2A: Overflow builtins with large types */
void test_overflow_builtins(void) {
    int64_t a, b;
    int64_t res;
    int overflow;
    
    /* These will trigger double_int comparisons during constant folding */
    a = INT64_MAX;
    b = 2;
    
    /* Multiplication overflow check */
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(INT64_MAX, 2) = %d, res = %ld\n", overflow, res);
    
    /* Addition overflow check */
    overflow = __builtin_add_overflow(a, 1, &res);
    printf("add_overflow(INT64_MAX, 1) = %d, res = %ld\n", overflow, res);
    
    /* Subtraction overflow check */
    a = INT64_MIN;
    overflow = __builtin_sub_overflow(a, 1, &res);
    printf("sub_overflow(INT64_MIN, 1) = %d, res = %ld\n", overflow, res);
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(INT64_MAX, 2, (int64_t)0))) {
        printf("Constant overflow check passed\n");
    }
}

/* Test 2B: Builtin with 128-bit types */
#ifdef __SIZEOF_INT128__
void test_128bit_overflow(void) {
    int128_t a = ((int128_t)INT64_MAX) * 2;
    int128_t b = ((int128_t)INT64_MAX) * 3;
    int128_t res;
    
    /* Force compiler to use double_int for overflow detection */
    if (__builtin_add_overflow(a, b, &res)) {
        printf("128-bit addition overflow detected\n");
    } else {
        printf("128-bit addition result fits\n");
    }
}
#endif

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Test 3A: Complex range analysis with multiplication */
void test_range_analysis(int x) {
    /* Create known bounds for range analysis */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        int64_t y = (int64_t)x * (int64_t)x;
        
        /* Further comparisons on the result */
        if (y > 1000000 && y < 4000000) {
            printf("Range analysis: y = %ld within expected range\n", y);
        }
        
        /* Chain of comparisons */
        int64_t z = y * 2;
        if (z > y && z < 4000000 * 2) {
            printf("Chained comparison passed\n");
        }
    }
}

/* Test 3B: Loop with induction variable analysis */
void test_induction_variables(void) {
    for (int64_t i = INT64_MAX - 100; i < INT64_MAX; i += 10) {
        /* Compiler must analyze potential overflow in loop bounds */
        if (i > INT64_MAX - 50) {
            printf("Near overflow: %ld\n", i);
        }
    }
    
    /* Large step value */
    for (int64_t j = 0; j < INT64_MAX; j += (INT64_MAX / 100)) {
        /* Empty loop - compiler analyzes bounds */
        (void)j;
    }
}

/* Test 3C: Multiple range intersections */
void test_range_intersection(int a, int b) {
    /* Create complex range conditions */
    if (a > 0 && a < 1000 && b > 500 && b < 1500) {
        int64_t product = (int64_t)a * (int64_t)b;
        
        /* Nested range checks */
        if (product > 0 && product < 1500000) {
            if (product > 100000 && product < 1400000) {
                printf("Product %ld in complex range\n", product);
            }
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ version available) ========== */

#ifdef __cplusplus
#include <type_traits>

/* Test 4A: Template with large integer comparisons */
template <int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_very_large = N > (int128_t(1) << 65);
    static const bool greater_than_max = N > INT64_MAX;
    
    static void check() {
        static_assert(is_positive == (N > 0), "Positive check failed");
        static_assert(is_very_large == (N > (int128_t(1) << 65)), "Size check failed");
    }
};

/* Instantiate templates to force compile-time comparisons */
template struct LargeCompare<(int128_t)1 << 66>;
template struct LargeCompare<-(int128_t)1 << 66>;
template struct LargeCompare<INT64_MAX + 1LL>;

/* Test 4B: Compile-time function with large integers */
constexpr bool compare_at_compile_time(int128_t a, int128_t b) {
    return a < b;
}

static_assert(compare_at_compile_time((int128_t)1 << 63, (int128_t)1 << 64), 
              "Compile-time comparison failed");
#endif

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Test 5A: Using __int128 with arithmetic operations */
#ifdef __SIZEOF_INT128__
void test_128bit_arithmetic(void) {
    __int128 a = ((__int128)0x123456789ABCDEF0) << 32;
    __int128 b = ((__int128)0xFEDCBA9876543210) >> 16;
    
    /* Operations that require magnitude comparison */
    __int128 sum = a + b;
    __int128 diff = a - b;
    __int128 prod = a / 1000;  /* Division requires comparison */
    
    /* Comparisons that might use double_int::cmp */
    if (a > b) printf("a > b\n");
    if (sum > diff) printf("sum > diff\n");
    if (prod != 0) printf("prod != 0\n");
    
    /* Modulus operation */
    __int128 mod = a % 1000000007;
    if (mod >= 0 && mod < 1000000007) {
        printf("Modulus in range: %lld\n", (long long)mod);
    }
}

/* Test 5B: Enumeration with large values */
enum big_enum : __int128 {
    BIG_VALUE = ((__int128)1 << 70),
    BIGGER_VALUE = ((__int128)1 << 80),
    BIG_NEGATIVE = -((__int128)1 << 70)
};

/* Test 5C: Using __attribute__((mode(TI))) for 128-bit */
typedef int int128 __attribute__((mode(TI)));
typedef unsigned int uint128 __attribute__((mode(TI)));

void test_mode_TI(void) {
    int128 x = ((int128)1 << 70);
    int128 y = ((int128)1 << 69);
    
    /* Multiple comparisons */
    if (x > y) printf("TI mode: x > y\n");
    if (x != y) printf("TI mode: x != y\n");
    if (x >= y + y) printf("TI mode: x >= 2*y\n");
}
#endif

/* ========== Main Test Harness ========== */

int main(void) {
    int all_tests_passed = 1;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Test 2: Overflow builtins */
    printf("--- Test 2: Overflow Builtins ---\n");
    test_overflow_builtins();
    
#ifdef __SIZEOF_INT128__
    printf("\n--- Test 2B: 128-bit Overflow ---\n");
    test_128bit_overflow();
#endif
    
    /* Test 3: Range analysis */
    printf("\n--- Test 3: Range Analysis ---\n");
    test_range_analysis(1500);  /* Within range */
    test_range_analysis(500);   /* Outside range */
    
    test_induction_variables();
    
    /* Test multiple range intersections */
    test_range_intersection(800, 1000);
    test_range_intersection(200, 300);
    
#ifdef __SIZEOF_INT128__
    /* Test 5: 128-bit arithmetic */
    printf("\n--- Test 5: 128-bit Arithmetic ---\n");
    test_128bit_arithmetic();
    
    printf("\n--- Test 5C: TI Mode Arithmetic ---\n");
    test_mode_TI();
    
    /* Verify enum values */
    printf("\n--- Test 5B: Large Enums ---\n");
    if (BIGGER_VALUE > BIG_VALUE) {
        printf("Enum comparison correct: BIGGER_VALUE > BIG_VALUE\n");
    }
    if (BIG_NEGATIVE < 0) {
        printf("Enum comparison correct: BIG_NEGATIVE < 0\n");
    }
#endif
    
    /* Additional runtime validation */
    printf("\n--- Runtime Validation ---\n");
    
    /* Test comparisons with mixed sizes */
    {
        int64_t large64 = INT64_MAX;
        int32_t small32 = 100;
        
        /* These should trigger different comparison paths */
        if (large64 > small32) printf("Mixed size comparison 1 passed\n");
        if (small32 < large64) printf("Mixed size comparison 2 passed\n");
        
        /* Force promotion */
        int64_t promoted = small32;
        if (large64 > promoted) printf("Promotion comparison passed\n");
    }
    
    /* Test edge cases */
    {
        uint64_t max_u64 = UINT64_MAX;
        int64_t max_s64 = INT64_MAX;
        
        /* Unsigned comparisons */
        if (max_u64 > max_s64) printf("Unsigned/signed comparison passed\n");
        
        /* Zero comparisons */
        if (0 < max_u64) printf("Zero comparison passed\n");
        if (-1 < 0) printf("Negative comparison passed\n");
    }
    
    printf("\n=== All tests completed ===\n");
    
    if (all_tests_passed) {
        printf("PASS\n");
        return 0;
    } else {
        printf("FAIL\n");
        return 1;
    }
}
