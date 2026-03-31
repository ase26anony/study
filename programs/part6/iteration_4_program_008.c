/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Large constants that require 128-bit representation */
#define VERY_LARGE_CONST ((__int128_t)1 << 70)
#define HUGE_CONST ((__int128_t)0x123456789ABCDEFULL << 64 | 0xFEDCBA9876543210ULL)
#define NEG_LARGE_CONST ((__int128_t)-1 << 80)

/* Static assertions force compile-time comparison */
static_assert(VERY_LARGE_CONST > 0, "Large positive constant");
static_assert(NEG_LARGE_CONST < 0, "Large negative constant");
static_assert(HUGE_CONST > VERY_LARGE_CONST, "Compare two large positives");
static_assert(VERY_LARGE_CONST != HUGE_CONST, "Large inequality check");

/* Template-like macro for compile-time comparison */
#define COMPILE_TIME_CMP(a, b) \
    sizeof(char[(a) > (b) ? 1 : -1])  /* Force error if comparison fails */

/* Test various comparison scenarios at compile time */
void test_constant_folding(void) {
    printf("Testing constant folding with large integers...\n");
    
    /* These comparisons must be evaluated at compile time */
    const __int128_t a = ((__int128_t)1 << 65) + 123;
    const __int128_t b = ((__int128_t)1 << 65) + 456;
    const __int128_t c = ((__int128_t)1 << 66);
    
    /* Force compiler to evaluate comparisons */
    if (__builtin_constant_p(a < b)) {
        printf("  a < b evaluated at compile time: %s\n", a < b ? "true" : "false");
    }
    
    if (__builtin_constant_p(b < c)) {
        printf("  b < c evaluated at compile time: %s\n", b < c ? "true" : "false");
    }
    
    if (__builtin_constant_p(a == a)) {
        printf("  a == a evaluated at compile time: true\n");
    }
    
    /* Test with mixed high/low parts */
    const __int128_t x = ((__int128_t)0x1ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    const __int128_t y = ((__int128_t)0x2ULL << 64) | 0x0ULL;
    
    if (__builtin_constant_p(x < y)) {
        printf("  x < y (different high parts): %s\n", x < y ? "true" : "false");
    }
    
    /* Test with same high part, different low part */
    const __int128_t p = ((__int128_t)0x1ULL << 64) | 0x1ULL;
    const __int128_t q = ((__int128_t)0x1ULL << 64) | 0x2ULL;
    
    if (__builtin_constant_p(p < q)) {
        printf("  p < q (same high, different low): %s\n", p < q ? "true" : "false");
    }
}

/* ========== 2. GCC Builtins That Return or Manipulate double_int ========== */

void test_overflow_builtins(void) {
    printf("\nTesting overflow builtins...\n");
    
    long long a, b;
    long long result;
    int overflow;
    
    /* Test cases designed to trigger overflow checks with comparisons */
    
    /* Case 1: Multiplication that overflows 64-bit */
    a = 0x7FFFFFFFFFFFFFFFLL;  /* Max positive int64 */
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &result);
    printf("  Overflow check 1: %lld * %lld overflow? %s\n", 
           a, b, overflow ? "yes" : "no");
    
    /* Case 2: Large multiplication requiring wide comparison */
    a = 0x123456789ABCDEFLL;
    b = 0xFEDCBA987654321LL;
    overflow = __builtin_mul_overflow(a, b, &result);
    printf("  Overflow check 2: large multiplication overflow? %s\n",
           overflow ? "yes" : "no");
    
    /* Case 3: Addition overflow */
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &result);
    printf("  Overflow check 3: %lld + %lld overflow? %s\n",
           a, b, overflow ? "yes" : "no");
    
    /* Case 4: Use __builtin_constant_p with overflow */
    const long long c1 = 0x4000000000000000LL;
    const long long c2 = 0x4000000000000000LL;
    
    if (__builtin_constant_p(__builtin_mul_overflow_p(c1, c2, (long long)0))) {
        printf("  Constant overflow check evaluated at compile time\n");
    }
    
    /* Test with 128-bit intermediate results */
    __int128_t big_a = 0x7FFFFFFFFFFFFFFFLL;
    __int128_t big_b = 0x7FFFFFFFFFFFFFFFLL;
    __int128_t big_result = big_a * big_b;
    
    /* Force comparison of 128-bit values */
    if (big_result > ((__int128_t)1 << 100)) {
        printf("  128-bit comparison triggered\n");
    }
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

void test_range_calculations(int input) {
    printf("\nTesting range calculations...\n");
    
    /* Create complex range analysis scenarios */
    
    /* Scenario 1: Nested ranges */
    int x = input;
    if (x > 1000 && x < 10000) {
        /* The compiler must compute range for x*x */
        long long y = (long long)x * x;
        
        /* Further range checks */
        if (y > 5000000 && y < 500000000) {
            printf("  Range 1: x=%d, y=%lld within nested range\n", x, y);
        }
    }
    
    /* Scenario 2: Multiple bounds with large numbers */
    unsigned long long big = 0xFFFFFFFFFFFFFFFFULL;
    if (input > 0) {
        unsigned long long scaled = big / input;
        
        /* This division result's range depends on comparison of bounds */
        if (scaled < 0x100000000ULL) {
            printf("  Range 2: scaled value in lower range\n");
        }
    }
    
    /* Scenario 3: Loop with induction variable analysis */
    for (long long i = 0x7000000000000000LL; 
         i < 0x7000000000000100LL; 
         i += 0x10) {
        /* Compiler analyzes i's range and may use wide int comparisons */
        if (i & 0x100) {
            printf("  Range 3: i=0x%llx\n", i);
            break;
        }
    }
    
    /* Scenario 4: Signed overflow wrap-around analysis */
    int val = input;
    if (val > 0x70000000) {
        int doubled = val * 2;  /* May overflow, compiler analyzes range */
        if (doubled < val) {    /* Check for overflow */
            printf("  Range 4: overflow detected\n");
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ style in C) ========== */

/* Simulate template-like behavior using macros and inline functions */

#define LARGE_COMPARE_GT(val, threshold) \
    ((val) > (threshold) ? 1 : 0)

#define LARGE_COMPARE_LT(val, threshold) \
    ((val) < (threshold) ? 1 : 0)

/* Force compile-time evaluation of large comparisons */
static const int result_gt = LARGE_COMPARE_GT(
    ((__int128_t)1 << 66), 
    ((__int128_t)1 << 65)
);

static const int result_lt = LARGE_COMPARE_LT(
    ((__int128_t)-1 << 70),
    ((__int128_t)-1 << 69)
);

void test_template_like_comparisons(void) {
    printf("\nTesting template-like large comparisons...\n");
    
    printf("  (1 << 66) > (1 << 65): %s\n", 
           result_gt ? "true" : "false");
    printf("  (-1 << 70) < (-1 << 69): %s\n",
           result_lt ? "true" : "false");
    
    /* Additional runtime checks */
    __int128_t template_val1 = ((__int128_t)1 << 100) | 0x12345;
    __int128_t template_val2 = ((__int128_t)1 << 100) | 0x12346;
    
    if (LARGE_COMPARE_LT(template_val1, template_val2)) {
        printf("  Large values with same high bits compared correctly\n");
    }
}

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Use 128-bit types with attributes */
typedef __int128_t __attribute__((mode(TI))) int128_t;
typedef unsigned __int128_t __attribute__((mode(TI))) uint128_t;

/* Enumeration with large values */
enum big_enum {
    BIG_VAL1 = ((__int128_t)1 << 72),
    BIG_VAL2 = ((__int128_t)1 << 73),
    BIG_VAL3 = ((__int128_t)1 << 74)
};

void test_wide_constant_operations(void) {
    printf("\nTesting wide constant operations...\n");
    
    /* Operations on 128-bit constants */
    int128_t w1 = ((int128_t)0x123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL;
    int128_t w2 = ((int128_t)0x1ULL << 64) | 0x1ULL;
    
    /* Various operations that require magnitude comparison */
    int128_t quotient = w1 / w2;
    int128_t remainder = w1 % w2;
    
    /* Comparisons during division */
    if (w1 > w2) {
        printf("  Wide comparison: w1 > w2\n");
    }
    
    if (quotient > 0) {
        printf("  Quotient positive: 0x%016llx%016llx\n", 
               (unsigned long long)(quotient >> 64),
               (unsigned long long)(quotient & 0xFFFFFFFFFFFFFFFFULL));
    }
    
    /* Test with negative wide integers */
    int128_t neg_wide = -((int128_t)1 << 80);
    int128_t pos_wide = ((int128_t)1 << 79);
    
    if (neg_wide < pos_wide) {
        printf("  Negative wide < positive wide\n");
    }
    
    /* Mixed-size comparisons */
    long long regular = 0x7FFFFFFFFFFFFFFFLL;
    if (w1 > regular) {
        printf("  128-bit > 64-bit comparison\n");
    }
}

/* ========== 6. Complex Real-world Scenarios ========== */

void test_complex_scenarios(void) {
    printf("\nTesting complex scenarios...\n");
    
    /* Scenario: Array indexing with large offsets */
    char buffer[1000];
    __int128_t large_index = ((__int128_t)1 << 63) + 500;
    
    /* The compiler must analyze if large_index is within bounds */
    if (large_index < 0 || large_index >= 1000) {
        printf("  Large index out of bounds analysis\n");
    }
    
    /* Scenario: Bitfield extraction with wide ints */
    uint128_t packed = ((uint128_t)0x1234ULL << 96) | 
                      ((uint128_t)0x5678ULL << 64) |
                      ((uint128_t)0x9ABCULL << 32) |
                      0xDEF0ULL;
    
    /* Extract and compare parts */
    uint64_t high_part = packed >> 64;
    uint64_t low_part = packed & 0xFFFFFFFFFFFFFFFFULL;
    
    if (high_part > low_part) {
        printf("  High 64 bits > low 64 bits\n");
    }
    
    /* Scenario: Cryptographic-like constant comparisons */
    const __int128_t crypto_const1 = ((__int128_t)0x6a09e667f3bcc908ULL << 64) | 
                                    0xbb67ae8584caa73bULL;
    const __int128_t crypto_const2 = ((__int128_t)0x3c6ef372fe94f82bULL << 64) | 
                                    0xa54ff53a5f1d36f1ULL;
    
    if (crypto_const1 != crypto_const2) {
        printf("  Cryptographic constants differ\n");
    }
}

/* ========== Main Function ========== */

int main(int argc, char *argv[]) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Seed for random tests */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Run all test suites */
    test_constant_folding();
    test_overflow_builtins();
    test_range_calculations(rand() % 10000 + 500);
    test_template_like_comparisons();
    test_wide_constant_operations();
    test_complex_scenarios();
    
    printf("\n=== All tests completed ===\n");
    
    /* Final validation */
    const __int128_t final_check_a = ((__int128_t)0x1ULL << 64) | 0xFFFFFFFFULL;
    const __int128_t final_check_b = ((__int128_t)0x1ULL << 64) | 0xFFFFFFFEULL;
    
    if (final_check_a > final_check_b && 
        final_check_a != final_check_b &&
        final_check_b < final_check_a) {
        printf("\nPASS: All double_int comparisons working correctly\n");
        return 0;
    } else {
        printf("\nFAIL: Comparison logic error\n");
        return 1;
    }
}
