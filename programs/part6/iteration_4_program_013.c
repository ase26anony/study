/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

/* Enable 128-bit integer support */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* ===== 1. Trigger Constant Folding with Large Integers ===== */

/* Large constants that require double_int representation */
static const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
static const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
static const int128_t HUGE_PRODUCT = ((int128_t)INT64_MAX) * ((int128_t)INT64_MAX);
static const uint128_t LARGE_UNSIGNED = ((uint128_t)1 << 80);

/* Static assertions that force compile-time comparisons */
_Static_assert(VERY_LARGE_POS > 0, "Large positive constant comparison");
_Static_assert(VERY_LARGE_NEG < 0, "Large negative constant comparison");
_Static_assert(HUGE_PRODUCT > ((int128_t)1 << 100), "Huge product comparison");
_Static_assert(LARGE_UNSIGNED > UINT64_MAX, "Large unsigned comparison");

/* Compile-time function using __builtin_constant_p */
static int check_constant_comparisons(void) {
    if (__builtin_constant_p(VERY_LARGE_POS > VERY_LARGE_NEG)) {
        return 1;
    }
    if (__builtin_constant_p(LARGE_UNSIGNED < ((uint128_t)1 << 90))) {
        return 2;
    }
    return 0;
}

/* ===== 2. GCC Builtins That Return or Manipulate double_int ===== */

/* Test overflow builtins with large values */
static void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Case 1: Multiplication that overflows 64-bit */
    a = LLONG_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MAX, 2): overflow=%d\n", overflow);
    
    /* Case 2: Addition with potential overflow */
    a = LLONG_MAX;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("add_overflow(LLONG_MAX, 1): overflow=%d\n", overflow);
    
    /* Case 3: Subtraction with underflow */
    a = LLONG_MIN;
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    printf("sub_overflow(LLONG_MIN, 1): overflow=%d\n", overflow);
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, (long long)0))) {
        printf("Constant overflow check passed\n");
    }
}

/* ===== 3. Range Calculations That Compare Bounds ===== */

/* Function with complex range analysis */
static void test_range_calculations(int x) {
    /* Create known bounds for x */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        long long y = (long long)x * x;
        
        /* Further range checks */
        if (y > 1000000 && y < 4000000) {
            printf("Range check passed: y=%lld\n", y);
        }
        
        /* Test with larger values that might require double_int */
        long long z = y * 1000LL;
        if (z > 1000000000LL) {
            printf("Large range: z=%lld\n", z);
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -2000) {
        long long y = (long long)x * x;
        if (y > 1000000 && y < 4000000) {
            printf("Negative range check passed: y=%lld\n", y);
        }
    }
}

/* Loop with induction variable analysis */
static void test_induction_variables(void) {
    for (int64_t i = 0; i < 10000; i += 1000) {
        /* Large step might trigger wrap-around analysis */
        int64_t j = i * i;
        if (j > 10000000) {
            printf("Induction: i=%ld, j=%ld\n", (long)i, (long)j);
        }
    }
    
    /* Test with potential overflow in loop bounds */
    for (int64_t k = INT64_MAX - 1000; k < INT64_MAX; k += 100) {
        /* This might trigger range analysis with double_int comparisons */
        if (k > INT64_MAX - 500) {
            printf("Near overflow: k=%ld\n", (long)k);
            break;
        }
    }
}

/* ===== 4. Template Metaprogramming (C++ version) ===== */
#ifdef __cplusplus

template <int128_t N>
struct LargeCompare {
    static const bool greater_than_zero = N > 0;
    static const bool less_than_max = N < ((int128_t)1 << 65);
    static const bool equal_self = N == N;
    static const bool not_equal = N != (N + 1);
};

template <uint128_t N>
struct UnsignedLargeCompare {
    static const bool above_threshold = N > ((uint128_t)1 << 70);
    static const bool below_threshold = N < ((uint128_t)1 << 75);
};

/* Instantiate templates with various large values */
template struct LargeCompare<((int128_t)1 << 64)>;
template struct LargeCompare<-((int128_t)1 << 64)>;
template struct UnsignedLargeCompare<((uint128_t)1 << 72)>;

#endif

/* ===== 5. Force Tree Node Construction for Wide Constants ===== */

/* Use 128-bit types with attributes */
typedef int128_t __attribute__((mode(TI))) ti_int;
typedef uint128_t __attribute__((mode(TI))) tu_int;

/* Operations on wide constants that require comparison */
static void test_wide_constant_operations(void) {
    ti_int a = ((ti_int)1 << 70);
    ti_int b = ((ti_int)1 << 69);
    
    /* These operations require magnitude comparisons */
    ti_int sum = a + b;
    ti_int diff = a - b;
    ti_int prod = a / 2;
    
    /* Comparisons that should trigger double_int::cmp */
    if (a > b) printf("a > b (wide comparison)\n");
    if (a != b) printf("a != b (wide inequality)\n");
    if (sum > a) printf("sum > a\n");
    if (diff > 0) printf("diff > 0\n");
    
    /* Modulus operation with large values */
    ti_int mod_result = a % b;
    if (mod_result == 0) printf("Modulus result is zero\n");
}

/* Enumeration with large values */
enum big_enum : int128_t {
    BIG_VALUE = ((int128_t)1 << 65),
    BIGGER_VALUE = ((int128_t)1 << 66),
    BIGGEST_VALUE = ((int128_t)1 << 67)
};

/* ===== Main Test Function ===== */

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Constant folding tests */
    printf("1. Constant Folding Tests:\n");
    printf("   VERY_LARGE_POS = %016llx%016llx\n", 
           (unsigned long long)(VERY_LARGE_POS >> 64),
           (unsigned long long)VERY_LARGE_POS);
    printf("   VERY_LARGE_NEG = %016llx%016llx\n", 
           (unsigned long long)(VERY_LARGE_NEG >> 64),
           (unsigned long long)VERY_LARGE_NEG);
    printf("   Constant comparison check: %d\n\n", check_constant_comparisons());
    
    /* 2. Overflow builtin tests */
    printf("2. Overflow Builtin Tests:\n");
    test_overflow_builtins();
    printf("\n");
    
    /* 3. Range calculation tests */
    printf("3. Range Calculation Tests:\n");
    test_range_calculations(1500);
    test_range_calculations(-1500);
    test_induction_variables();
    printf("\n");
    
    /* 4. Wide constant operations */
    printf("4. Wide Constant Operation Tests:\n");
    test_wide_constant_operations();
    printf("\n");
    
    /* 5. Runtime validation of compile-time comparisons */
    printf("5. Runtime Validation:\n");
    
    /* Verify that compile-time comparisons were correct */
    if (VERY_LARGE_POS > 0) {
        printf("   ✓ VERY_LARGE_POS > 0 (runtime check)\n");
    }
    
    if (VERY_LARGE_NEG < 0) {
        printf("   ✓ VERY_LARGE_NEG < 0 (runtime check)\n");
    }
    
    if (LARGE_UNSIGNED > UINT64_MAX) {
        printf("   ✓ LARGE_UNSIGNED > UINT64_MAX (runtime check)\n");
    }
    
    /* Test edge cases for comparisons */
    int128_t zero = 0;
    int128_t one = 1;
    int128_t minus_one = -1;
    
    if (zero == 0) printf("   ✓ zero == 0\n");
    if (one > zero) printf("   ✓ one > zero\n");
    if (minus_one < zero) printf("   ✓ minus_one < zero\n");
    if (VERY_LARGE_POS > VERY_LARGE_NEG) printf("   ✓ VERY_LARGE_POS > VERY_LARGE_NEG\n");
    
    /* Test equality comparisons */
    int128_t copy = VERY_LARGE_POS;
    if (VERY_LARGE_POS == copy) printf("   ✓ Equality comparison works\n");
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
