/* test-double-int-comparison.c */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Define large 128-bit constants that exercise different comparison paths */

/* High part differs, low part equal */
#define LARGE_A_HIGH_DIFF_LOW_EQ ((unsigned __int128)0x10000000000000000ULL)  /* high: 1, low: 0 */
#define LARGE_B_HIGH_DIFF_LOW_EQ ((unsigned __int128)0x20000000000000000ULL)  /* high: 2, low: 0 */

/* High part equal, low part differs */
#define LARGE_A_HIGH_EQ_LOW_DIFF ((__int128)0x10000000000000001ULL)  /* high: 1, low: 1 */
#define LARGE_B_HIGH_EQ_LOW_DIFF ((__int128)0x10000000000000002ULL)  /* high: 1, low: 2 */

/* Both parts differ */
#define LARGE_A_BOTH_DIFF ((unsigned __int128)0x10000000000000001ULL)  /* high: 1, low: 1 */
#define LARGE_B_BOTH_DIFF ((unsigned __int128)0x20000000000000002ULL)  /* high: 2, low: 2 */

/* Edge cases with sign bits for signed comparisons */
#define LARGE_NEGATIVE ((__int128)-1)  /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define LARGE_ZERO ((__int128)0)

/* Mixed high/low differences */
#define LARGE_MIXED_A ((__int128)0x10000000000000003ULL)  /* high: 1, low: 3 */
#define LARGE_MIXED_B ((__int128)0x10000000000000001ULL)  /* high: 1, low: 1 */

/* Compile-time comparisons using static assertions */
_Static_assert(LARGE_A_HIGH_DIFF_LOW_EQ < LARGE_B_HIGH_DIFF_LOW_EQ, 
               "High part less comparison should be true");
_Static_assert(LARGE_B_HIGH_DIFF_LOW_EQ > LARGE_A_HIGH_DIFF_LOW_EQ, 
               "High part greater comparison should be true");
_Static_assert(LARGE_A_HIGH_EQ_LOW_DIFF < LARGE_B_HIGH_EQ_LOW_DIFF, 
               "Low part less comparison should be true");
_Static_assert(LARGE_B_HIGH_EQ_LOW_DIFF > LARGE_A_HIGH_EQ_LOW_DIFF, 
               "Low part greater comparison should be true");
_Static_assert(LARGE_A_BOTH_DIFF < LARGE_B_BOTH_DIFF, 
               "Both parts less comparison should be true");
_Static_assert(LARGE_B_BOTH_DIFF > LARGE_A_BOTH_DIFF, 
               "Both parts greater comparison should be true");
_Static_assert(LARGE_NEGATIVE < LARGE_ZERO, 
               "Negative < Zero should be true (tests unsigned high part comparison)");
_Static_assert(LARGE_ZERO > LARGE_NEGATIVE, 
               "Zero > Negative should be true");

/* Compile-time constant expressions */
const int cmp_high_less = (LARGE_A_HIGH_DIFF_LOW_EQ < LARGE_B_HIGH_DIFF_LOW_EQ) ? 1 : 0;
const int cmp_high_greater = (LARGE_B_HIGH_DIFF_LOW_EQ > LARGE_A_HIGH_DIFF_LOW_EQ) ? 1 : 0;
const int cmp_low_less = (LARGE_A_HIGH_EQ_LOW_DIFF < LARGE_B_HIGH_EQ_LOW_DIFF) ? 1 : 0;
const int cmp_low_greater = (LARGE_B_HIGH_EQ_LOW_DIFF > LARGE_A_HIGH_EQ_LOW_DIFF) ? 1 : 0;
const int cmp_both_less = (LARGE_A_BOTH_DIFF < LARGE_B_BOTH_DIFF) ? 1 : 0;
const int cmp_both_greater = (LARGE_B_BOTH_DIFF > LARGE_A_BOTH_DIFF) ? 1 : 0;

/* Array size depending on comparison result */
char array_high_less[(LARGE_A_HIGH_DIFF_LOW_EQ < LARGE_B_HIGH_DIFF_LOW_EQ) ? 10 : 20];
char array_low_greater[(LARGE_B_HIGH_EQ_LOW_DIFF > LARGE_A_HIGH_EQ_LOW_DIFF) ? 15 : 25];

/* Test GCC built-in overflow functions with 128-bit operands */
int test_builtin_overflow(void) {
    __int128 a = LARGE_MIXED_A;
    __int128 b = LARGE_MIXED_B;
    __int128 result;
    int overflow;
    
    /* These built-ins may perform internal comparisons */
    overflow = __builtin_mul_overflow(a, b, &result);
    overflow |= __builtin_add_overflow_p(a, b, (__int128)0);
    overflow |= __builtin_sub_overflow_p(a, b, (__int128)0);
    
    return overflow;
}

int main(void) {
    int checksum = 0;
    
    /* Runtime comparisons with volatile variables to prevent optimization */
    volatile __int128 volatile_a = LARGE_A_HIGH_EQ_LOW_DIFF;
    volatile __int128 volatile_b = LARGE_B_HIGH_EQ_LOW_DIFF;
    volatile unsigned __int128 volatile_ua = LARGE_A_BOTH_DIFF;
    volatile unsigned __int128 volatile_ub = LARGE_B_BOTH_DIFF;
    
    /* Test 1: High part differs, low part equal (unsigned) */
    if (volatile_ua < volatile_ub) checksum += 1;  /* Should take high-less path */
    if (volatile_ub > volatile_ua) checksum += 2;  /* Should take high-greater path */
    
    /* Test 2: High part equal, low part differs (signed) */
    if (volatile_a < volatile_b) checksum += 4;    /* Should take low-less path */
    if (volatile_b > volatile_a) checksum += 8;    /* Should take low-greater path */
    
    /* Test 3: Both parts differ */
    volatile_ua = LARGE_A_BOTH_DIFF;
    volatile_ub = LARGE_B_BOTH_DIFF;
    if (volatile_ua < volatile_ub) checksum += 16;  /* Should take high-less path */
    if (volatile_ub > volatile_ua) checksum += 32;  /* Should take high-greater path */
    
    /* Test 4: Signed comparisons with negative values */
    volatile __int128 volatile_neg = LARGE_NEGATIVE;
    volatile __int128 volatile_zero = LARGE_ZERO;
    if (volatile_neg < volatile_zero) checksum += 64;   /* Tests unsigned high comparison */
    if (volatile_zero > volatile_neg) checksum += 128;  /* Tests unsigned high comparison */
    
    /* Test 5: Equality comparisons (should not take the uncovered paths) */
    if (volatile_a == volatile_a) checksum += 256;
    if (volatile_ua == volatile_ua) checksum += 512;
    
    /* Test 6: Less-than-or-equal and greater-than-or-equal */
    if (volatile_a <= volatile_b) checksum += 1024;    /* Should be true */
    if (volatile_b >= volatile_a) checksum += 2048;    /* Should be true */
    if (volatile_ua <= volatile_ub) checksum += 4096;  /* Should be true */
    if (volatile_ub >= volatile_ua) checksum += 8192;  /* Should be true */
    
    /* Test 7: Mixed signed/unsigned comparisons */
    /* Note: These may trigger different comparison operators */
    __int128 signed_val = LARGE_MIXED_A;
    unsigned __int128 unsigned_val = LARGE_A_BOTH_DIFF;
    if (signed_val < (__int128)unsigned_val) checksum += 16384;
    if ((unsigned __int128)signed_val > unsigned_val) checksum += 32768;
    
    /* Test 8: Use built-in overflow functions */
    checksum += test_builtin_overflow() * 65536;
    
    /* Test 9: Additional comparisons with constants */
    const __int128 const_a = LARGE_MIXED_A;
    const __int128 const_b = LARGE_MIXED_B;
    if (const_a > const_b) checksum += 131072;
    if (const_b < const_a) checksum += 262144;
    
    /* Test 10: Chain comparisons */
    if (volatile_a < volatile_b && volatile_b < volatile_a + 10) checksum += 524288;
    
    /* Print checksum to prevent dead code elimination */
    printf("Comparison checksum: %d\n", checksum);
    printf("Array sizes: %zu, %zu\n", 
           sizeof(array_high_less), sizeof(array_low_greater));
    printf("Compile-time constants: %d,%d,%d,%d,%d,%d\n",
           cmp_high_less, cmp_high_greater, cmp_low_less, 
           cmp_low_greater, cmp_both_less, cmp_both_greater);
    
    return 0;
}
