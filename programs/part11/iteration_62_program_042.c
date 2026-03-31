/* test_double_int_comparisons.c
 * 
 * This program exercises the double_int comparison logic in GCC,
 * specifically targeting lines 1285-1293 of double-int.cc.
 * It creates various 128-bit integer comparisons that require
 * both high and low part evaluations with unsigned semantics.
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large constants that exercise different comparison paths */
#define HIGH_DIFF_LOW_EQUAL_A   (((__int128)0x1ULL) << 64)      /* 0x10000000000000000 */
#define HIGH_DIFF_LOW_EQUAL_B   (((__int128)0x2ULL) << 64)      /* 0x20000000000000000 */

#define HIGH_EQUAL_LOW_DIFF_A   (((__int128)0x1ULL << 64) | 0x1ULL)  /* 0x10000000000000001 */
#define HIGH_EQUAL_LOW_DIFF_B   (((__int128)0x1ULL << 64) | 0x2ULL)  /* 0x10000000000000002 */

#define BOTH_PARTS_DIFF_A       (((__int128)0x1ULL << 64) | 0x1ULL)  /* 0x10000000000000001 */
#define BOTH_PARTS_DIFF_B       (((__int128)0x2ULL << 64) | 0x2ULL)  /* 0x20000000000000002 */

#define SIGNED_NEGATIVE         ((__int128)-1)                  /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO             ((__int128)0)

/* Unsigned versions */
#define UHIGH_DIFF_LOW_EQUAL_A  ((unsigned __int128)0x1ULL << 64)
#define UHIGH_DIFF_LOW_EQUAL_B  ((unsigned __int128)0x2ULL << 64)

/* Compile-time comparisons using static assertions */
_Static_assert(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
               "High part less comparison should be true");
_Static_assert(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B,
               "Low part less comparison should be true");
_Static_assert(BOTH_PARTS_DIFF_A < BOTH_PARTS_DIFF_B,
               "Both parts less comparison should be true");
_Static_assert(SIGNED_NEGATIVE < SIGNED_ZERO,
               "Signed negative < zero should be true (unsigned high part comparison)");

/* Constant expressions that force compile-time evaluation */
const int cmp_high_less = (HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) ? 1 : 0;
const int cmp_low_less = (HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B) ? 1 : 0;
const int cmp_both_less = (BOTH_PARTS_DIFF_A < BOTH_PARTS_DIFF_B) ? 1 : 0;
const int cmp_signed_neg = (SIGNED_NEGATIVE < SIGNED_ZERO) ? 1 : 0;

/* Array size depending on comparison result */
char array_high_greater[(UHIGH_DIFF_LOW_EQUAL_B > UHIGH_DIFF_LOW_EQUAL_A) ? 10 : 20];
char array_low_greater[(HIGH_EQUAL_LOW_DIFF_B > HIGH_EQUAL_LOW_DIFF_A) ? 15 : 25];

/* Runtime comparison function */
static int runtime_comparisons(void) {
    int checksum = 0;
    
    /* Use volatile to force runtime evaluation */
    volatile __int128 v1 = HIGH_DIFF_LOW_EQUAL_A;
    volatile __int128 v2 = HIGH_DIFF_LOW_EQUAL_B;
    volatile __int128 v3 = HIGH_EQUAL_LOW_DIFF_A;
    volatile __int128 v4 = HIGH_EQUAL_LOW_DIFF_B;
    volatile __int128 v5 = BOTH_PARTS_DIFF_A;
    volatile __int128 v6 = BOTH_PARTS_DIFF_B;
    volatile __int128 v7 = SIGNED_NEGATIVE;
    volatile __int128 v8 = SIGNED_ZERO;
    
    volatile unsigned __int128 uv1 = UHIGH_DIFF_LOW_EQUAL_A;
    volatile unsigned __int128 uv2 = UHIGH_DIFF_LOW_EQUAL_B;
    
    /* Test 1: High part differs, low part equal (unsigned comparison) */
    if ((unsigned __int128)v1 < (unsigned __int128)v2) checksum += 1;  /* high less */
    if ((unsigned __int128)v2 > (unsigned __int128)v1) checksum += 2;  /* high greater */
    
    /* Test 2: High part equal, low part differs */
    if (v3 < v4) checksum += 4;  /* low less */
    if (v4 > v3) checksum += 8;  /* low greater */
    
    /* Test 3: Both parts differ */
    if (v5 < v6) checksum += 16; /* both less */
    if (v6 > v5) checksum += 32; /* both greater */
    
    /* Test 4: Signed comparisons with negative values */
    if (v7 < v8) checksum += 64; /* signed negative < zero */
    if (v8 > v7) checksum += 128; /* zero > signed negative */
    
    /* Test 5: Equality comparisons */
    if (v1 == v1) checksum += 256;
    if (v2 != v1) checksum += 512;
    
    /* Test 6: Mixed signed/unsigned comparisons */
    if ((unsigned __int128)v7 > (unsigned __int128)v8) checksum += 1024;
    
    /* Test 7: Unsigned high part comparisons */
    if (uv1 < uv2) checksum += 2048;
    if (uv2 > uv1) checksum += 4096;
    
    return checksum;
}

/* Use GCC built-ins that may trigger double_int comparisons */
static int builtin_comparisons(void) {
    int result = 0;
    __int128 a = HIGH_EQUAL_LOW_DIFF_A;
    __int128 b = HIGH_EQUAL_LOW_DIFF_B;
    __int128 sum;
    
    /* __builtin_add_overflow may internally compare */
    if (__builtin_add_overflow(a, b, &sum)) {
        result += 1;
    }
    
    /* __builtin_mul_overflow with large values */
    __int128 prod;
    __int128 c = ((__int128)0x7FFFFFFFFFFFFFFFULL) << 32;
    if (__builtin_mul_overflow(c, c, &prod)) {
        result += 2;
    }
    
    return result;
}

/* C++ version with constexpr (compile as C++ with g++) */
#ifdef __cplusplus
#include <type_traits>

constexpr bool constexpr_compare_high_less() {
    const __int128 x = HIGH_DIFF_LOW_EQUAL_A;
    const __int128 y = HIGH_DIFF_LOW_EQUAL_B;
    return x < y;
}

constexpr bool constexpr_compare_low_less() {
    const __int128 x = HIGH_EQUAL_LOW_DIFF_A;
    const __int128 y = HIGH_EQUAL_LOW_DIFF_B;
    return x < y;
}

static_assert(constexpr_compare_high_less(), 
              "C++ constexpr high less comparison failed");
static_assert(constexpr_compare_low_less(),
              "C++ constexpr low less comparison failed");
#endif

int main(void) {
    int total_checksum = 0;
    
    /* Add compile-time comparison results */
    total_checksum += cmp_high_less;
    total_checksum += cmp_low_less;
    total_checksum += cmp_both_less;
    total_checksum += cmp_signed_neg;
    
    /* Runtime comparisons */
    total_checksum += runtime_comparisons();
    
    /* Built-in function comparisons */
    total_checksum += builtin_comparisons();
    
    /* Verify array sizes were computed correctly */
    assert(sizeof(array_high_greater) == 10);
    assert(sizeof(array_low_greater) == 15);
    
    printf("Comparison checksum: %d\n", total_checksum);
    printf("All comparisons executed successfully.\n");
    
    return 0;
}
