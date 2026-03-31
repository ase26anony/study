/* test-double-int-comparison.c */
#include <stdio.h>
#include <stdint.h>

/* Define large 128-bit constants that exercise different comparison paths */
#define HIGH_PART_DIFFERS_LOW_EQUAL_A ((__int128)0x1ULL << 64)      /* 0x10000000000000000 */
#define HIGH_PART_DIFFERS_LOW_EQUAL_B ((__int128)0x2ULL << 64)      /* 0x20000000000000000 */

#define HIGH_EQUAL_LOW_DIFFERS_A (((__int128)0x1ULL << 64) | 0x1ULL) /* 0x10000000000000001 */
#define HIGH_EQUAL_LOW_DIFFERS_B (((__int128)0x1ULL << 64) | 0x2ULL) /* 0x10000000000000002 */

#define BOTH_PARTS_DIFFER_A (((__int128)0x1ULL << 64) | 0x1ULL)     /* 0x10000000000000001 */
#define BOTH_PARTS_DIFFER_B (((__int128)0x2ULL << 64) | 0x2ULL)     /* 0x20000000000000002 */

#define NEGATIVE_ONE ((__int128)-1)                                 /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define ZERO ((__int128)0)

/* Unsigned versions */
#define UHIGH_PART_DIFFERS_LOW_EQUAL_A ((unsigned __int128)0x1ULL << 64)
#define UHIGH_PART_DIFFERS_LOW_EQUAL_B ((unsigned __int128)0x2ULL << 64)

/* Compile-time comparisons using static assertions */
_Static_assert(HIGH_PART_DIFFERS_LOW_EQUAL_A < HIGH_PART_DIFFERS_LOW_EQUAL_B, 
               "High part differs, low equal: A < B should be true");
_Static_assert(HIGH_PART_DIFFERS_LOW_EQUAL_B > HIGH_PART_DIFFERS_LOW_EQUAL_A,
               "High part differs, low equal: B > A should be true");

_Static_assert(HIGH_EQUAL_LOW_DIFFERS_A < HIGH_EQUAL_LOW_DIFFERS_B,
               "High equal, low differs: A < B should be true");
_Static_assert(HIGH_EQUAL_LOW_DIFFERS_B > HIGH_EQUAL_LOW_DIFFERS_A,
               "High equal, low differs: B > A should be true");

_Static_assert(BOTH_PARTS_DIFFER_A < BOTH_PARTS_DIFFER_B,
               "Both parts differ: A < B should be true");
_Static_assert(BOTH_PARTS_DIFFER_B > BOTH_PARTS_DIFFER_A,
               "Both parts differ: B > A should be true");

/* Edge case with sign bits - negative vs zero */
_Static_assert(NEGATIVE_ONE < ZERO,
               "Negative one < zero should be true for signed comparison");
_Static_assert(ZERO > NEGATIVE_ONE,
               "Zero > negative one should be true");

/* Unsigned comparisons at compile time */
_Static_assert(UHIGH_PART_DIFFERS_LOW_EQUAL_A < UHIGH_PART_DIFFERS_LOW_EQUAL_B,
               "Unsigned: High part differs, low equal");

/* Compile-time constant expressions */
const int cmp_high_diff = (HIGH_PART_DIFFERS_LOW_EQUAL_A < HIGH_PART_DIFFERS_LOW_EQUAL_B) ? 1 : 0;
const int cmp_low_diff = (HIGH_EQUAL_LOW_DIFFERS_A < HIGH_EQUAL_LOW_DIFFERS_B) ? 1 : 0;
const int cmp_both_diff = (BOTH_PARTS_DIFFER_A < BOTH_PARTS_DIFFER_B) ? 1 : 0;
const int cmp_signed_neg = (NEGATIVE_ONE < ZERO) ? 1 : 0;

/* Array sizes based on comparisons */
char arr_high_diff[(HIGH_PART_DIFFERS_LOW_EQUAL_A < HIGH_PART_DIFFERS_LOW_EQUAL_B) ? 10 : 20];
char arr_low_diff[(HIGH_EQUAL_LOW_DIFFERS_A < HIGH_EQUAL_LOW_DIFFERS_B) ? 15 : 25];
char arr_both_diff[(BOTH_PARTS_DIFFER_A < BOTH_PARTS_DIFFER_B) ? 20 : 30];

/* Runtime comparisons with volatile to prevent constant folding */
volatile __int128 runtime_a = ((__int128)0x3ULL << 64) | 0x1ULL;  /* 0x30000000000000001 */
volatile __int128 runtime_b = ((__int128)0x4ULL << 64) | 0x2ULL;  /* 0x40000000000000002 */
volatile __int128 runtime_c = ((__int128)0x3ULL << 64) | 0x3ULL;  /* 0x30000000000000003 */
volatile __int128 runtime_d = ((__int128)0x3ULL << 64) | 0x1ULL;  /* Same as runtime_a */

volatile unsigned __int128 runtime_u_a = ((unsigned __int128)0x5ULL << 64);
volatile unsigned __int128 runtime_u_b = ((unsigned __int128)0x6ULL << 64);

/* Test GCC built-in overflow functions with 128-bit values */
int test_builtin_overflow(void) {
    __int128 x = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 y = 1;
    __int128 result;
    int overflow = __builtin_add_overflow(x, y, &result);
    return overflow;
}

int main(void) {
    int checksum = 0;
    
    /* Add compile-time comparison results */
    checksum += cmp_high_diff;
    checksum += cmp_low_diff;
    checksum += cmp_both_diff;
    checksum += cmp_signed_neg;
    
    /* Runtime signed comparisons */
    if (runtime_a < runtime_b) checksum += 1;  /* High part differs: 3 < 4 */
    if (runtime_a > runtime_b) checksum += 0;  /* Should not execute */
    
    if (runtime_a < runtime_c) checksum += 1;  /* High equal, low differs: 1 < 3 */
    if (runtime_c > runtime_a) checksum += 1;  /* High equal, low differs: 3 > 1 */
    
    if (runtime_a == runtime_d) checksum += 1; /* Equality test */
    if (runtime_a != runtime_b) checksum += 1; /* Inequality test */
    
    if (runtime_a <= runtime_b) checksum += 1; /* Less or equal */
    if (runtime_b >= runtime_a) checksum += 1; /* Greater or equal */
    
    /* Test all comparison operators with signed values */
    checksum += (runtime_a < runtime_b) ? 1 : 0;
    checksum += (runtime_a > runtime_b) ? 0 : 1;
    checksum += (runtime_a <= runtime_b) ? 1 : 0;
    checksum += (runtime_a >= runtime_b) ? 0 : 1;
    checksum += (runtime_a == runtime_d) ? 1 : 0;
    checksum += (runtime_a != runtime_b) ? 1 : 0;
    
    /* Runtime unsigned comparisons */
    if (runtime_u_a < runtime_u_b) checksum += 1;  /* 5 < 6 */
    if (runtime_u_b > runtime_u_a) checksum += 1;  /* 6 > 5 */
    
    checksum += (runtime_u_a < runtime_u_b) ? 1 : 0;
    checksum += (runtime_u_a > runtime_u_b) ? 0 : 1;
    checksum += (runtime_u_a <= runtime_u_b) ? 1 : 0;
    checksum += (runtime_u_a >= runtime_u_b) ? 0 : 1;
    
    /* Edge cases with negative values */
    volatile __int128 neg_val = -1;
    volatile __int128 pos_val = 0;
    
    if (neg_val < pos_val) checksum += 1;  /* -1 < 0 */
    if (pos_val > neg_val) checksum += 1;  /* 0 > -1 */
    if (neg_val <= pos_val) checksum += 1; /* -1 <= 0 */
    if (pos_val >= neg_val) checksum += 1; /* 0 >= -1 */
    
    /* Test with very large negative number */
    volatile __int128 large_neg = ((__int128)-1) << 120;  /* Very large negative */
    if (large_neg < pos_val) checksum += 1;
    
    /* Test built-in overflow functions */
    checksum += test_builtin_overflow();
    
    /* Mixed signed/unsigned comparisons (these may trigger different code paths) */
    volatile __int128 signed_val = ((__int128)0x1ULL << 63);  /* High bit set in low part */
    volatile unsigned __int128 unsigned_val = ((unsigned __int128)0x1ULL << 63);
    
    /* These comparisons may require special handling */
    checksum += (signed_val < (__int128)unsigned_val) ? 1 : 0;
    checksum += ((unsigned __int128)signed_val < unsigned_val) ? 0 : 1;
    
    printf("Comparison checksum: %d\n", checksum);
    printf("Array sizes: %zu, %zu, %zu\n", 
           sizeof(arr_high_diff), 
           sizeof(arr_low_diff), 
           sizeof(arr_both_diff));
    
    return 0;
}
