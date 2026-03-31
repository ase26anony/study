/* test-double-int-comparison.c */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Use static assertions for compile-time evaluation */
#define STATIC_ASSERT _Static_assert

/* Large 128-bit constants that exercise different comparison paths */
#define HIGH_DIFF_LOW_EQUAL_A ((unsigned __int128)0x10000000000000000ULL)  /* high=1, low=0 */
#define HIGH_DIFF_LOW_EQUAL_B ((unsigned __int128)0x20000000000000000ULL)  /* high=2, low=0 */

#define HIGH_EQUAL_LOW_DIFF_A ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define HIGH_EQUAL_LOW_DIFF_B ((unsigned __int128)0x10000000000000002ULL)  /* high=1, low=2 */

#define BOTH_DIFF_A ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define BOTH_DIFF_B ((unsigned __int128)0x20000000000000002ULL)  /* high=2, low=2 */

/* Signed constants with sign bit implications */
#define SIGNED_NEG_ONE ((__int128)-1)  /* All bits set: 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO ((__int128)0)
#define SIGNED_LARGE_POS ((__int128)0x7FFFFFFFFFFFFFFFFFFFFFFFULL)  /* Large positive */
#define SIGNED_LARGE_NEG ((__int128)0x800000000000000000000000ULL)  /* Large negative */

/* Compile-time comparisons using static assertions */
STATIC_ASSERT(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
              "High part less comparison failed");
STATIC_ASSERT(HIGH_DIFF_LOW_EQUAL_B > HIGH_DIFF_LOW_EQUAL_A,
              "High part greater comparison failed");
STATIC_ASSERT(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B,
              "Low part less comparison failed");
STATIC_ASSERT(HIGH_EQUAL_LOW_DIFF_B > HIGH_EQUAL_LOW_DIFF_A,
              "Low part greater comparison failed");
STATIC_ASSERT(BOTH_DIFF_A < BOTH_DIFF_B,
              "Both parts differ comparison failed");
STATIC_ASSERT(SIGNED_NEG_ONE < SIGNED_ZERO,
              "Signed negative comparison failed");
STATIC_ASSERT(SIGNED_LARGE_POS > SIGNED_LARGE_NEG,
              "Signed large value comparison failed");

/* Compile-time comparisons in constant expressions */
const int cmp_high_less = (HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) ? 1 : 0;
const int cmp_high_greater = (HIGH_DIFF_LOW_EQUAL_B > HIGH_DIFF_LOW_EQUAL_A) ? 1 : 0;
const int cmp_low_less = (HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B) ? 1 : 0;
const int cmp_low_greater = (HIGH_EQUAL_LOW_DIFF_B > HIGH_EQUAL_LOW_DIFF_A) ? 1 : 0;
const int cmp_both_diff = (BOTH_DIFF_A < BOTH_DIFF_B) ? 1 : 0;
const int cmp_signed_neg = (SIGNED_NEG_ONE < SIGNED_ZERO) ? 1 : 0;

/* Array size depending on comparison result */
char array_high[(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) ? 10 : 20];
char array_low[(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B) ? 15 : 25];

/* Runtime comparisons with volatile to prevent optimization */
static int runtime_comparisons(void) {
    int checksum = 0;
    
    /* Volatile variables force runtime evaluation */
    volatile unsigned __int128 v1 = HIGH_DIFF_LOW_EQUAL_A;
    volatile unsigned __int128 v2 = HIGH_DIFF_LOW_EQUAL_B;
    volatile unsigned __int128 v3 = HIGH_EQUAL_LOW_DIFF_A;
    volatile unsigned __int128 v4 = HIGH_EQUAL_LOW_DIFF_B;
    volatile unsigned __int128 v5 = BOTH_DIFF_A;
    volatile unsigned __int128 v6 = BOTH_DIFF_B;
    volatile __int128 v7 = SIGNED_NEG_ONE;
    volatile __int128 v8 = SIGNED_ZERO;
    volatile __int128 v9 = SIGNED_LARGE_POS;
    volatile __int128 v10 = SIGNED_LARGE_NEG;
    
    /* High part differs, low part equal */
    if (v1 < v2) checksum += 1;    /* Should take: high less */
    if (v2 > v1) checksum += 2;    /* Should take: high greater */
    if (v1 <= v2) checksum += 4;   /* Should take: high less or equal */
    if (v2 >= v1) checksum += 8;   /* Should take: high greater or equal */
    
    /* High part equal, low part differs */
    if (v3 < v4) checksum += 16;   /* Should take: low less */
    if (v4 > v3) checksum += 32;   /* Should take: low greater */
    if (v3 <= v4) checksum += 64;  /* Should take: low less or equal */
    if (v4 >= v3) checksum += 128; /* Should take: low greater or equal */
    
    /* Both parts differ */
    if (v5 < v6) checksum += 256;  /* Should take: both less */
    if (v6 > v5) checksum += 512;  /* Should take: both greater */
    
    /* Signed comparisons with sign implications */
    if (v7 < v8) checksum += 1024; /* Should take: signed negative less */
    if (v8 > v7) checksum += 2048; /* Should take: zero greater than negative */
    if (v9 > v10) checksum += 4096; /* Should take: large positive > large negative */
    
    /* Equality comparisons */
    if (v1 == v1) checksum += 8192;  /* Should take: equal */
    if (v1 != v2) checksum += 16384; /* Should take: not equal */
    
    return checksum;
}

/* Use GCC builtins that may trigger internal comparisons */
static int builtin_comparisons(void) {
    int checksum = 0;
    unsigned __int128 a = HIGH_EQUAL_LOW_DIFF_A;
    unsigned __int128 b = HIGH_EQUAL_LOW_DIFF_B;
    __int128 sa = SIGNED_NEG_ONE;
    __int128 sb = SIGNED_ZERO;
    
    /* __builtin_add_overflow_p may perform comparisons internally */
    if (__builtin_add_overflow_p(a, b, (unsigned __int128)0)) {
        checksum += 1;
    }
    
    /* __builtin_mul_overflow may perform comparisons */
    unsigned __int128 prod;
    if (__builtin_mul_overflow(a, (unsigned __int128)2, &prod)) {
        checksum += 2;
    }
    
    /* Signed overflow checks */
    __int128 sum;
    if (__builtin_add_overflow(sa, sb, &sum)) {
        checksum += 4;
    }
    
    return checksum;
}

/* Mixed signed/unsigned comparisons */
static int mixed_comparisons(void) {
    int checksum = 0;
    
    /* Compare unsigned __int128 with __int128 */
    unsigned __int128 uval = 0xFFFFFFFFFFFFFFFFULL;  /* 2^64 - 1 */
    __int128 sval = -1;
    
    /* These should trigger unsigned comparison of high parts */
    if (uval > (unsigned __int128)sval) checksum += 1;
    if ((__int128)uval < sval) checksum += 2;
    
    /* Large values that differ in high part */
    unsigned __int128 large_u1 = ((unsigned __int128)0x1ULL << 64) | 0x1ULL;
    unsigned __int128 large_u2 = ((unsigned __int128)0x2ULL << 64) | 0x1ULL;
    __int128 large_s1 = (__int128)large_u1;
    __int128 large_s2 = (__int128)large_u2;
    
    if (large_u1 < large_u2) checksum += 4;
    if (large_s1 < large_s2) checksum += 8;
    if (large_u1 <= large_s2) checksum += 16;
    
    return checksum;
}

/* Function with constant expressions that compare 128-bit values */
static constexpr int constexpr_comparisons(void) {
    /* Using C++ constexpr if compiling as C++ */
    const unsigned __int128 c1 = HIGH_DIFF_LOW_EQUAL_A;
    const unsigned __int128 c2 = HIGH_DIFF_LOW_EQUAL_B;
    const unsigned __int128 c3 = HIGH_EQUAL_LOW_DIFF_A;
    const unsigned __int128 c4 = HIGH_EQUAL_LOW_DIFF_B;
    
    int result = 0;
    result += (c1 < c2) ? 1 : 0;
    result += (c2 > c1) ? 2 : 0;
    result += (c3 < c4) ? 4 : 0;
    result += (c4 > c3) ? 8 : 0;
    result += (c1 == c1) ? 16 : 0;
    result += (c1 != c2) ? 32 : 0;
    
    return result;
}

int main(void) {
    int total_checksum = 0;
    
    /* Add compile-time comparison results */
    total_checksum += cmp_high_less;
    total_checksum += cmp_high_greater;
    total_checksum += cmp_low_less;
    total_checksum += cmp_low_greater;
    total_checksum += cmp_both_diff;
    total_checksum += cmp_signed_neg;
    
    /* Runtime comparisons */
    total_checksum += runtime_comparisons();
    
    /* Built-in function comparisons */
    total_checksum += builtin_comparisons();
    
    /* Mixed signed/unsigned comparisons */
    total_checksum += mixed_comparisons();
    
    /* Constexpr comparisons (compile-time in C++) */
    total_checksum += constexpr_comparisons();
    
    /* Use the array sizes to prevent optimization */
    array_high[0] = 0;
    array_low[0] = 0;
    
    printf("Total checksum: %d\n", total_checksum);
    printf("Array sizes: %zu, %zu\n", sizeof(array_high), sizeof(array_low));
    
    /* Verify expected values */
    assert(cmp_high_less == 1);
    assert(cmp_high_greater == 1);
    assert(cmp_low_less == 1);
    assert(cmp_low_greater == 1);
    assert(cmp_both_diff == 1);
    assert(cmp_signed_neg == 1);
    
    return 0;
}
