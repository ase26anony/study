/* test-double-int-comparison.c */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Define large 128-bit constants that exercise different comparison paths */

/* High part differs, low part equal */
#define LARGE_A_HIGH_DIFF_LOW_EQ ((unsigned __int128)0x10000000000000000ULL)  /* high=1, low=0 */
#define LARGE_B_HIGH_DIFF_LOW_EQ ((unsigned __int128)0x20000000000000000ULL)  /* high=2, low=0 */

/* High part equal, low part differs */
#define LARGE_A_HIGH_EQ_LOW_DIFF ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define LARGE_B_HIGH_EQ_LOW_DIFF ((unsigned __int128)0x10000000000000002ULL)  /* high=1, low=2 */

/* Both parts differ */
#define LARGE_A_BOTH_DIFF ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define LARGE_B_BOTH_DIFF ((unsigned __int128)0x20000000000000002ULL)  /* high=2, low=2 */

/* Edge cases with sign bits for signed comparisons */
#define SIGNED_NEG_ONE ((__int128)-1)  /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO ((__int128)0)
#define SIGNED_LARGE_POS ((__int128)0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFULL)
#define SIGNED_LARGE_NEG ((__int128)0x80000000000000000000000000000000ULL)

/* Compile-time comparisons using static assertions */
_Static_assert(LARGE_A_HIGH_DIFF_LOW_EQ < LARGE_B_HIGH_DIFF_LOW_EQ, 
               "High part less comparison failed");
_Static_assert(LARGE_A_HIGH_EQ_LOW_DIFF < LARGE_B_HIGH_EQ_LOW_DIFF, 
               "Low part less comparison failed");
_Static_assert(LARGE_B_HIGH_DIFF_LOW_EQ > LARGE_A_HIGH_DIFF_LOW_EQ, 
               "High part greater comparison failed");
_Static_assert(LARGE_B_HIGH_EQ_LOW_DIFF > LARGE_A_HIGH_EQ_LOW_DIFF, 
               "Low part greater comparison failed");

/* Constant expressions that force compile-time evaluation */
const int cmp_high_less = (LARGE_A_HIGH_DIFF_LOW_EQ < LARGE_B_HIGH_DIFF_LOW_EQ) ? 1 : 0;
const int cmp_low_less = (LARGE_A_HIGH_EQ_LOW_DIFF < LARGE_B_HIGH_EQ_LOW_DIFF) ? 1 : 0;
const int cmp_high_greater = (LARGE_B_HIGH_DIFF_LOW_EQ > LARGE_A_HIGH_DIFF_LOW_EQ) ? 1 : 0;
const int cmp_low_greater = (LARGE_B_HIGH_EQ_LOW_DIFF > LARGE_A_HIGH_EQ_LOW_DIFF) ? 1 : 0;

/* Array size depending on comparison result */
char arr_high_less[(LARGE_A_HIGH_DIFF_LOW_EQ < LARGE_B_HIGH_DIFF_LOW_EQ) ? 10 : 20];
char arr_low_less[(LARGE_A_HIGH_EQ_LOW_DIFF < LARGE_B_HIGH_EQ_LOW_DIFF) ? 15 : 25];

/* Runtime comparisons with volatile to prevent optimization */
static int runtime_comparisons(void) {
    int checksum = 0;
    
    /* Use volatile to force runtime evaluation */
    volatile unsigned __int128 v1 = LARGE_A_HIGH_DIFF_LOW_EQ;
    volatile unsigned __int128 v2 = LARGE_B_HIGH_DIFF_LOW_EQ;
    volatile unsigned __int128 v3 = LARGE_A_HIGH_EQ_LOW_DIFF;
    volatile unsigned __int128 v4 = LARGE_B_HIGH_EQ_LOW_DIFF;
    volatile unsigned __int128 v5 = LARGE_A_BOTH_DIFF;
    volatile unsigned __int128 v6 = LARGE_B_BOTH_DIFF;
    
    volatile __int128 sv1 = SIGNED_NEG_ONE;
    volatile __int128 sv2 = SIGNED_ZERO;
    volatile __int128 sv3 = SIGNED_LARGE_POS;
    volatile __int128 sv4 = SIGNED_LARGE_NEG;
    
    /* High part differs, low part equal */
    if (v1 < v2) checksum += 1;    /* Should take: high less */
    if (v2 > v1) checksum += 2;    /* Should take: high greater */
    
    /* High part equal, low part differs */
    if (v3 < v4) checksum += 4;    /* Should take: low less */
    if (v4 > v3) checksum += 8;    /* Should take: low greater */
    
    /* Both parts differ */
    if (v5 < v6) checksum += 16;   /* Should take: high less */
    if (v6 > v5) checksum += 32;   /* Should take: high greater */
    
    /* Signed comparisons with sign extension handling */
    if (sv1 < sv2) checksum += 64;  /* -1 < 0: high part unsigned comparison of all-ones vs 0 */
    if (sv2 > sv1) checksum += 128; /* 0 > -1: high part unsigned comparison of 0 vs all-ones */
    
    /* Large signed values */
    if (sv4 < sv3) checksum += 256; /* Most negative < most positive */
    if (sv3 > sv4) checksum += 512; /* Most positive > most negative */
    
    /* Equality comparisons */
    if (v1 == v1) checksum += 1024;
    if (v2 != v1) checksum += 2048;
    
    return checksum;
}

/* Use GCC built-ins that may trigger internal comparisons */
static int builtin_comparisons(void) {
    int checksum = 0;
    __int128 a = 0x7FFFFFFFFFFFFFFFULL;
    __int128 b = 0x7FFFFFFFFFFFFFFFULL;
    __int128 result;
    int overflow;
    
    /* Multiplication overflow check */
    overflow = __builtin_mul_overflow(a, b, &result);
    if (overflow) checksum += 1;
    
    /* Addition overflow check */
    __int128 c = 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFULL;
    __int128 d = 1;
    overflow = __builtin_add_overflow(c, d, &result);
    if (overflow) checksum += 2;
    
    /* Overflow check with predicate */
    if (__builtin_add_overflow_p(c, d, (__int128)0)) {
        checksum += 4;
    }
    
    return checksum;
}

/* Mixed signed/unsigned comparisons */
static int mixed_comparisons(void) {
    int checksum = 0;
    
    /* Compare unsigned __int128 with signed __int128 */
    unsigned __int128 u1 = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFULL;
    __int128 s1 = -1;
    
    /* These should be equal in value but different in type */
    if (u1 == (unsigned __int128)s1) checksum += 1;
    if ((__int128)u1 == s1) checksum += 2;
    
    /* Comparisons that require type conversions */
    unsigned __int128 u2 = 0x80000000000000000000000000000000ULL;
    __int128 s2 = (__int128)0x80000000000000000000000000000000ULL;
    
    if (u2 > (unsigned __int128)s2) checksum += 4;
    if (s2 < (__int128)u2) checksum += 8;
    
    return checksum;
}

/* Complex expressions with comparisons */
static int complex_expression_comparisons(void) {
    unsigned __int128 x = LARGE_A_HIGH_DIFF_LOW_EQ;
    unsigned __int128 y = LARGE_B_HIGH_DIFF_LOW_EQ;
    unsigned __int128 z = LARGE_A_HIGH_EQ_LOW_DIFF;
    
    int result = 0;
    
    /* Chained comparisons */
    if (x < y && y > z) result += 1;
    
    /* Ternary operator with comparisons */
    result += (x < y) ? 2 : 0;
    result += (y > z) ? 4 : 0;
    result += (z < x) ? 8 : 0;
    
    /* Arithmetic with comparison */
    unsigned __int128 sum = x + y;
    if (sum > x && sum > y) result += 16;
    
    return result;
}

int main(void) {
    int total_checksum = 0;
    
    /* Add compile-time comparison results */
    total_checksum += cmp_high_less;
    total_checksum += cmp_low_less;
    total_checksum += cmp_high_greater;
    total_checksum += cmp_low_greater;
    
    /* Runtime comparisons */
    total_checksum += runtime_comparisons();
    
    /* Built-in function comparisons */
    total_checksum += builtin_comparisons();
    
    /* Mixed signed/unsigned comparisons */
    total_checksum += mixed_comparisons();
    
    /* Complex expression comparisons */
    total_checksum += complex_expression_comparisons();
    
    /* Verify array sizes were correctly determined */
    assert(sizeof(arr_high_less) == 10);
    assert(sizeof(arr_low_less) == 15);
    
    printf("Total checksum: %d\n", total_checksum);
    printf("All comparisons executed successfully.\n");
    
    return 0;
}
