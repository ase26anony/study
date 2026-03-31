/* test_double_int_cmp.c - Comprehensive test to trigger double_int::cmp logic */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Use 128-bit constants that require double_int representation */
#define VERY_LARGE_CONST ((__int128_t)1 << 70)
#define VERY_LARGE_NEGATIVE (-((__int128_t)1 << 70))
#define LARGE_PRODUCT ((__int128_t)0x7FFFFFFFFFFFFFFF * 2)

/* Static assertions force compile-time comparison */
static_assert(VERY_LARGE_CONST > 0, "Large positive constant should be > 0");
static_assert(VERY_LARGE_NEGATIVE < 0, "Large negative constant should be < 0");
static_assert(VERY_LARGE_CONST > VERY_LARGE_NEGATIVE, 
              "Positive should be greater than negative");
static_assert(LARGE_PRODUCT > INT64_MAX, 
              "Product should exceed 64-bit maximum");

/* Template-like macro for compile-time comparisons */
#define COMPILE_TIME_CMP(a, b) \
    sizeof(char[(a) > (b) ? 1 : -1])  /* Will fail if comparison is false */

/* Force evaluation with __builtin_constant_p */
static int check_constant_comparisons(void) {
    int result = 0;
    
    /* These force constant folding */
    if (__builtin_constant_p(VERY_LARGE_CONST > 1000)) {
        result |= 1;
    }
    if (__builtin_constant_p(VERY_LARGE_NEGATIVE < -1000)) {
        result |= 2;
    }
    
    /* Compare two large constants */
    if (__builtin_constant_p(((__int128_t)1 << 65) > ((__int128_t)1 << 64))) {
        result |= 4;
    }
    
    return result;
}

/* ========== 2. GCC Builtins That Return or Manipulate double_int ========== */

/* Test overflow builtins with large values */
static void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Test cases that will trigger overflow checks */
    a = LLONG_MAX;
    b = 2;
    
    /* __builtin_mul_overflow uses double_int internally */
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MAX, 2) = %d, res = %lld\n", overflow, res);
    
    /* Test with constants that force compile-time evaluation */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, (long long)0))) {
        printf("Constant overflow check passed\n");
    }
    
    /* Test addition overflow */
    a = LLONG_MAX;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("add_overflow(LLONG_MAX, 1) = %d, res = %lld\n", overflow, res);
    
    /* Test subtraction with large negative */
    a = LLONG_MIN;
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    printf("sub_overflow(LLONG_MIN, 1) = %d, res = %lld\n", overflow, res);
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Complex range analysis that uses double_int comparisons */
static int test_range_analysis(int x) {
    int result = 0;
    
    /* Create known bounds for x */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        long long y = (long long)x * x;
        
        /* Further comparisons on the result */
        if (y > 1000000 && y < 4000000) {
            result = 1;
        }
    }
    
    /* Test with very large ranges */
    if (x > -1000000000 && x < 1000000000) {
        /* Multiplication that could overflow 32-bit */
        long long z = (long long)x * 1000000;
        if (z > -1000000000000LL && z < 1000000000000LL) {
            result |= 2;
        }
    }
    
    return result;
}

/* Loop with induction variable that may wrap */
static void test_loop_wrap_analysis(void) {
    unsigned long long i;
    unsigned long long sum = 0;
    
    /* Loop that might trigger wrap-around analysis */
    for (i = ULLONG_MAX - 10; i < ULLONG_MAX; i++) {
        sum += i;
    }
    printf("Loop sum: %llu\n", sum);
    
    /* Another loop with large step */
    for (i = 0; i < ULLONG_MAX; i += (ULLONG_MAX / 100)) {
        if (i > ULLONG_MAX / 2) {
            break;
        }
    }
}

/* ========== 4. Template Metaprogramming with Large Values (C++ style in C) ========== */

/* Simulate template-like behavior with macros and inline functions */
typedef struct {
    __int128_t value;
} LargeInt;

static inline int compare_large(LargeInt a, LargeInt b) {
    /* This should trigger the double_int comparison */
    if (a.value < b.value) return -1;
    if (a.value > b.value) return 1;
    return 0;
}

/* Function that uses large integer comparisons */
static int test_large_comparisons(void) {
    LargeInt a = { .value = ((__int128_t)1 << 65) };
    LargeInt b = { .value = ((__int128_t)1 << 64) };
    LargeInt c = { .value = -((__int128_t)1 << 65) };
    
    int cmp1 = compare_large(a, b);  /* Should return 1 */
    int cmp2 = compare_large(b, a);  /* Should return -1 */
    int cmp3 = compare_large(c, a);  /* Should return -1 */
    int cmp4 = compare_large(a, a);  /* Should return 0 */
    
    return (cmp1 == 1 && cmp2 == -1 && cmp3 == -1 && cmp4 == 0) ? 1 : 0;
}

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Use 128-bit types with attributes */
typedef __int128_t __attribute__((mode(TI))) int128_t;
typedef unsigned __int128_t __attribute__((mode(TI))) uint128_t;

/* Operations on 128-bit types that require comparisons */
static void test_128bit_operations(void) {
    int128_t a = ((int128_t)1 << 70);
    int128_t b = ((int128_t)1 << 69);
    int128_t c = -((int128_t)1 << 70);
    
    /* These operations require magnitude comparisons */
    int128_t sum = a + b;
    int128_t diff = a - b;
    int128_t neg = -a;
    
    /* Comparisons that should trigger double_int::cmp */
    int gt = (a > b);
    int lt = (c < a);
    int eq = (a == a);
    int ne = (a != b);
    
    printf("128-bit comparisons: gt=%d, lt=%d, eq=%d, ne=%d\n", gt, lt, eq, ne);
    
    /* Division/modulus requires comparing magnitudes */
    if (b != 0) {
        int128_t div = a / b;
        int128_t mod = a % b;
        printf("128-bit division: %lld / %lld = %lld\n", 
               (long long)(a >> 64), (long long)(b >> 64), (long long)(div >> 64));
    }
}

/* ========== Main Test Harness ========== */

int main(void) {
    int success = 1;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Test constant folding */
    printf("1. Testing constant folding:\n");
    int const_result = check_constant_comparisons();
    printf("   Constant comparison result: %d (expected 7)\n", const_result);
    success &= (const_result == 7);
    
    /* 2. Test overflow builtins */
    printf("\n2. Testing overflow builtins:\n");
    test_overflow_builtins();
    
    /* 3. Test range analysis */
    printf("\n3. Testing range analysis:\n");
    int range_result = test_range_analysis(1500);
    printf("   Range analysis result: %d (expected 1)\n", range_result);
    success &= (range_result == 1);
    
    test_loop_wrap_analysis();
    
    /* 4. Test large comparisons */
    printf("\n4. Testing large integer comparisons:\n");
    int cmp_result = test_large_comparisons();
    printf("   Large comparison result: %d (expected 1)\n", cmp_result);
    success &= (cmp_result == 1);
    
    /* 5. Test 128-bit operations */
    printf("\n5. Testing 128-bit operations:\n");
    test_128bit_operations();
    
    /* Additional edge cases */
    printf("\n6. Testing edge cases:\n");
    
    /* Compare values with same high part but different low parts */
    __int128_t x1 = ((__int128_t)0x12345678 << 64) | 0x1;
    __int128_t x2 = ((__int128_t)0x12345678 << 64) | 0x2;
    
    int edge1 = (x1 < x2);  /* Should be 1 */
    int edge2 = (x2 > x1);  /* Should be 1 */
    printf("   Edge case 1 (same high, diff low): %s\n", 
           (edge1 && edge2) ? "PASS" : "FAIL");
    success &= (edge1 && edge2);
    
    /* Test with maximum values */
    __int128_t max_pos = ((__int128_t)0x7FFFFFFFFFFFFFFF << 64) | 0xFFFFFFFFFFFFFFFF;
    __int128_t max_neg = ((__int128_t)0x8000000000000000 << 64);
    
    int edge3 = (max_pos > max_neg);  /* Should be 1 */
    printf("   Edge case 2 (max pos vs max neg): %s\n", edge3 ? "PASS" : "FAIL");
    success &= edge3;
    
    printf("\n=== Overall Result: %s ===\n", success ? "PASS" : "FAIL");
    
    return success ? 0 : 1;
}
