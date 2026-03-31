/* test_double_int_cmp.c - Comprehensive test to trigger double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ==================== SECTION 1: Large Constants and Static Assertions ==================== */

/* Use __int128 for 128-bit integers */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Large constants that exceed 64 bits */
const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
const int128_t LARGE_MULT_RESULT = ((int128_t)0x7FFFFFFFFFFFFFFFLL * 4);
const uint128_t HUGE_UNSIGNED = ((uint128_t)1 << 100);

/* Static assertions force compile-time comparisons */
#define STATIC_ASSERT(cond) typedef char static_assert_##__LINE__[(cond) ? 1 : -1]

STATIC_ASSERT(VERY_LARGE_POS > 0);
STATIC_ASSERT(VERY_LARGE_NEG < 0);
STATIC_ASSERT(VERY_LARGE_POS > VERY_LARGE_NEG);
STATIC_ASSERT(HUGE_UNSIGNED > UINT64_MAX);
STATIC_ASSERT(LARGE_MULT_RESULT > INT64_MAX);

/* Template-like macro for compile-time comparisons */
#define COMPILE_TIME_CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

const int cmp1 = COMPILE_TIME_CMP(VERY_LARGE_POS, 0);
const int cmp2 = COMPILE_TIME_CMP(VERY_LARGE_NEG, VERY_LARGE_POS);
const int cmp3 = COMPILE_TIME_CMP(HUGE_UNSIGNED, (uint128_t)UINT64_MAX + 1);

/* ==================== SECTION 2: Builtin Overflow Operations ==================== */

/* Test overflow builtins with large values */
void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Cases that should trigger overflow detection */
    a = LLONG_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MAX, 2) = %d, res = %lld\n", overflow, res);
    
    a = 1LL << 62;
    b = 1LL << 62;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(2^62, 2^62) = %d, res = %lld\n", overflow, res);
    
    /* Test with __builtin_constant_p */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, LLONG_MAX))) {
        printf("Constant overflow check performed\n");
    }
    
    /* Add overflow with large values */
    long long sum;
    a = LLONG_MAX;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &sum);
    printf("add_overflow(LLONG_MAX, 1) = %d, sum = %lld\n", overflow, sum);
    
    /* Sub overflow with large negative */
    long long diff;
    a = LLONG_MIN;
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &diff);
    printf("sub_overflow(LLONG_MIN, 1) = %d, diff = %lld\n", overflow, diff);
}

/* ==================== SECTION 3: Range Analysis and VRP ==================== */

/* Complex range calculations */
void test_range_analysis(int x) {
    /* Create known bounds */
    if (x > 1000 && x < 2000) {
        /* Multiplication that creates large range */
        long long y = (long long)x * x;
        
        /* Nested conditions for range refinement */
        if (y > 1500000 && y < 3000000) {
            long long z = y * 2;
            printf("Range test: x=%d, y=%lld, z=%lld\n", x, y, z);
        }
    }
    
    /* Test with very large ranges */
    if (x > -1000000 && x < 1000000) {
        /* This multiplication could overflow 64-bit */
        long long big = (long long)x * 1000000000LL;
        
        /* Conditional that depends on comparison of potentially large values */
        if (big > 500000000000LL) {
            printf("Large range positive: %lld\n", big);
        } else if (big < -500000000000LL) {
            printf("Large range negative: %lld\n", big);
        }
    }
}

/* Loop with induction variable that could wrap */
void test_loop_wrap_analysis(void) {
    for (int64_t i = LLONG_MAX - 10; i < LLONG_MAX; i++) {
        /* Loop analysis may use double_int for wrap-around */
        if (i > LLONG_MAX - 5) {
            printf("Near overflow: %lld\n", (long long)i);
        }
    }
}

/* ==================== SECTION 4: 128-bit Arithmetic and Comparisons ==================== */

/* Direct 128-bit operations */
void test_128bit_operations(void) {
    int128_t a = ((int128_t)1 << 70) + 123;
    int128_t b = ((int128_t)1 << 69) + 456;
    int128_t c = -((int128_t)1 << 71);
    
    /* Runtime comparisons of 128-bit values */
    if (a > b) {
        printf("128-bit: a > b\n");
    }
    if (b < a) {
        printf("128-bit: b < a\n");
    }
    if (c < a && c < b) {
        printf("128-bit: c is smallest\n");
    }
    
    /* Arithmetic that might be constant folded */
    int128_t sum = a + b;
    int128_t diff = a - b;
    int128_t prod = (a >> 60) * (b >> 60); /* Avoid overflow */
    
    if (sum > a && sum > b) {
        printf("128-bit sum is larger than operands\n");
    }
    
    /* Compare with mixed sizes */
    if (a > INT64_MAX) {
        printf("a exceeds 64-bit signed maximum\n");
    }
}

/* ==================== SECTION 5: Compile-time Template-like Comparisons ==================== */

/* Use macros to simulate template instantiation with different large values */
#define GENERATE_COMPARISON(id, val1, val2) \
    do { \
        const int128_t v1 = (val1); \
        const int128_t v2 = (val2); \
        const char* result; \
        if (v1 < v2) result = "LESS"; \
        else if (v1 > v2) result = "GREATER"; \
        else result = "EQUAL"; \
        printf("Comparison %d: %s\n", id, result); \
    } while(0)

/* Force evaluation of __builtin_constant_p with large values */
void test_constant_folding(void) {
    /* These comparisons should be evaluated at compile-time */
    if (__builtin_constant_p(VERY_LARGE_POS > 0)) {
        printf("Constant folding triggered for VERY_LARGE_POS > 0\n");
    }
    
    if (__builtin_constant_p(HUGE_UNSIGNED < ((uint128_t)1 << 120))) {
        printf("Constant folding triggered for HUGE_UNSIGNED < 2^120\n");
    }
    
    /* Generate multiple comparisons with different large values */
    GENERATE_COMPARISON(1, ((int128_t)1 << 65), ((int128_t)1 << 64));
    GENERATE_COMPARISON(2, -((int128_t)1 << 66), -((int128_t)1 << 65));
    GENERATE_COMPARISON(3, ((int128_t)1 << 70) + 1, ((int128_t)1 << 70));
    GENERATE_COMPARISON(4, ((int128_t)0x123456789ABCDEF0LL << 32), 
                           ((int128_t)0xFEDCBA9876543210LL >> 16));
}

/* ==================== SECTION 6: Enum and Bitfield Tests ==================== */

/* Enum with large values (GCC extension) */
enum big_enum {
    BIG_VAL1 = ((int128_t)1 << 70),
    BIG_VAL2 = ((int128_t)1 << 71),
    BIG_VAL3 = BIG_VAL1 + BIG_VAL2
};

void test_enum_comparisons(void) {
    /* Compare enum values */
    if (BIG_VAL2 > BIG_VAL1) {
        printf("Enum comparison: BIG_VAL2 > BIG_VAL1\n");
    }
    
    if (BIG_VAL3 > BIG_VAL2 && BIG_VAL3 > BIG_VAL1) {
        printf("Enum comparison: BIG_VAL3 is largest\n");
    }
}

/* ==================== MAIN FUNCTION ==================== */

int main(int argc, char *argv[]) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Test 1: Overflow builtins */
    printf("--- Test 1: Overflow Builtins ---\n");
    test_overflow_builtins();
    printf("\n");
    
    /* Test 2: Range analysis */
    printf("--- Test 2: Range Analysis ---\n");
    test_range_analysis(1500);
    test_range_analysis(-500000);
    printf("\n");
    
    /* Test 3: 128-bit operations */
    printf("--- Test 3: 128-bit Operations ---\n");
    test_128bit_operations();
    printf("\n");
    
    /* Test 4: Constant folding */
    printf("--- Test 4: Constant Folding ---\n");
    test_constant_folding();
    printf("\n");
    
    /* Test 5: Enum comparisons */
    printf("--- Test 5: Enum Comparisons ---\n");
    test_enum_comparisons();
    printf("\n");
    
    /* Test 6: Loop wrap analysis */
    printf("--- Test 6: Loop Wrap Analysis ---\n");
    test_loop_wrap_analysis();
    printf("\n");
    
    /* Additional runtime validation */
    printf("--- Runtime Validation ---\n");
    printf("cmp1 (VERY_LARGE_POS > 0): %d (expected: 1)\n", cmp1);
    printf("cmp2 (VERY_LARGE_NEG < VERY_LARGE_POS): %d (expected: -1)\n", cmp2);
    printf("cmp3 (HUGE_UNSIGNED > UINT64_MAX+1): %d (expected: 1)\n", cmp3);
    
    /* Final check using all comparison operators */
    int all_pass = 1;
    all_pass &= (VERY_LARGE_POS > 0);
    all_pass &= (VERY_LARGE_NEG < 0);
    all_pass &= (VERY_LARGE_POS != VERY_LARGE_NEG);
    all_pass &= (HUGE_UNSIGNED >= ((uint128_t)UINT64_MAX + 1));
    all_pass &= (LARGE_MULT_RESULT <= ((int128_t)1 << 62));
    
    printf("\n=== %s ===\n", all_pass ? "ALL TESTS PASS" : "SOME TESTS FAILED");
    
    return all_pass ? 0 : 1;
}
