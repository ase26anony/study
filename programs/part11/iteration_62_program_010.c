/* test-double-int-comparison.c */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Use GCC's __int128 type which maps to double_int internally */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Constants that exercise different comparison paths */
#define HIGH_DIFF_LOW_EQUAL_A ((uint128_t)0x10000000000000000ULL) /* high:1, low:0 */
#define HIGH_DIFF_LOW_EQUAL_B ((uint128_t)0x20000000000000000ULL) /* high:2, low:0 */

#define HIGH_EQUAL_LOW_DIFF_A ((uint128_t)0x10000000000000001ULL) /* high:1, low:1 */
#define HIGH_EQUAL_LOW_DIFF_B ((uint128_t)0x10000000000000002ULL) /* high:1, low:2 */

#define BOTH_DIFF_A ((uint128_t)0x10000000000000001ULL) /* high:1, low:1 */
#define BOTH_DIFF_B ((uint128_t)0x20000000000000002ULL) /* high:2, low:2 */

#define SIGNED_NEG_ONE ((int128_t)-1) /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO ((int128_t)0)

/* Compile-time comparisons using static assertions */
_Static_assert(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
               "High diff, low equal: A should be less than B");
_Static_assert(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B,
               "High equal, low diff: A should be less than B");
_Static_assert(BOTH_DIFF_A < BOTH_DIFF_B,
               "Both diff: A should be less than B");
_Static_assert(SIGNED_NEG_ONE < SIGNED_ZERO,
               "Signed -1 should be less than 0");

/* Constant expressions that force compile-time evaluation */
const int cmp_high_diff = (HIGH_DIFF_LOW_EQUAL_A > HIGH_DIFF_LOW_EQUAL_B) ? 0 : 1;
const int cmp_low_diff = (HIGH_EQUAL_LOW_DIFF_A <= HIGH_EQUAL_LOW_DIFF_B) ? 1 : 0;
const int cmp_both_diff = (BOTH_DIFF_A != BOTH_DIFF_B) ? 1 : 0;
const int cmp_signed = (SIGNED_NEG_ONE >= SIGNED_ZERO) ? 0 : 1;

/* Array size depending on comparison result */
char arr_high[(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) ? 10 : 20];
char arr_low[(HIGH_EQUAL_LOW_DIFF_A > HIGH_EQUAL_LOW_DIFF_B) ? 5 : 15];

/* Runtime comparisons with volatile to prevent optimization */
static int runtime_comparisons(void) {
    int checksum = 0;
    
    /* Use volatile to force runtime evaluation */
    volatile uint128_t v1 = HIGH_DIFF_LOW_EQUAL_A;
    volatile uint128_t v2 = HIGH_DIFF_LOW_EQUAL_B;
    volatile uint128_t v3 = HIGH_EQUAL_LOW_DIFF_A;
    volatile uint128_t v4 = HIGH_EQUAL_LOW_DIFF_B;
    volatile int128_t v5 = SIGNED_NEG_ONE;
    volatile int128_t v6 = SIGNED_ZERO;
    
    /* Exercise all comparison operators */
    if (v1 < v2) checksum += 1;    /* high less */
    if (v2 > v1) checksum += 2;    /* high greater */
    if (v3 < v4) checksum += 4;    /* low less */
    if (v4 > v3) checksum += 8;    /* low greater */
    if (v5 < v6) checksum += 16;   /* signed with negative high part */
    if (v6 > v5) checksum += 32;   /* signed with zero high part */
    
    /* Equality comparisons */
    if (v1 == v1) checksum += 64;
    if (v2 != v1) checksum += 128;
    
    /* Mixed signed/unsigned comparisons */
    uint128_t uval = (uint128_t)1 << 127;  /* MSB set, but unsigned */
    int128_t sval = (int128_t)1 << 126;    /* Positive signed */
    
    if (uval > sval) checksum += 256;
    if (sval < uval) checksum += 512;
    
    return checksum;
}

/* Use GCC built-ins that may trigger double_int comparisons */
static int builtin_comparisons(void) {
    int checksum = 0;
    
    /* __builtin_mul_overflow with 128-bit values */
    int128_t a = (int128_t)0x7FFFFFFFFFFFFFFFLL;
    int128_t b = (int128_t)2;
    int128_t result;
    
    if (__builtin_mul_overflow(a, b, &result)) {
        checksum += 1024;  /* Overflow occurred */
    }
    
    /* __builtin_add_overflow_p for constant folding */
    uint128_t x = (uint128_t)0xFFFFFFFFFFFFFFFFULL;
    uint128_t y = (uint128_t)1;
    
    if (__builtin_add_overflow_p(x, y, (uint128_t)0)) {
        checksum += 2048;
    }
    
    return checksum;
}

/* Additional edge cases */
static int edge_case_comparisons(void) {
    int checksum = 0;
    
    /* Compare values where high part has MSB set (negative in signed) */
    int128_t s1 = (int128_t)0x8000000000000000ULL << 64;  /* Most negative */
    int128_t s2 = (int128_t)0x7FFFFFFFFFFFFFFFULL << 64;  /* Large positive */
    
    if (s1 < s2) checksum += 4096;  /* Should be true despite MSB difference */
    
    /* Compare with zero */
    uint128_t max_uint = ~(uint128_t)0;
    if (max_uint > 0) checksum += 8192;
    
    /* Chain comparisons */
    uint128_t low = 100;
    uint128_t mid = 200;
    uint128_t high = 300;
    
    if (low < mid && mid < high) checksum += 16384;
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    /* Add compile-time comparison results */
    total_checksum += cmp_high_diff;
    total_checksum += cmp_low_diff;
    total_checksum += cmp_both_diff;
    total_checksum += cmp_signed;
    
    /* Runtime comparisons */
    total_checksum += runtime_comparisons();
    
    /* Built-in function comparisons */
    total_checksum += builtin_comparisons();
    
    /* Edge cases */
    total_checksum += edge_case_comparisons();
    
    /* Prevent dead code elimination */
    printf("Comparison checksum: %d\n", total_checksum);
    printf("Array sizes: %zu, %zu\n", sizeof(arr_high), sizeof(arr_low));
    
    /* Additional assertions at runtime */
    assert(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B);
    assert(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B);
    assert(BOTH_DIFF_A < BOTH_DIFF_B);
    assert(SIGNED_NEG_ONE < SIGNED_ZERO);
    
    return 0;
}
