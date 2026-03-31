/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_64        0xFFFFFFFFFFFFFFFFULL
#define HIGH_MASK     0xFFFFFFFF00000000ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)MAX_64) < ((__int128)MAX_64 << 64),
               "Shifted max should be larger");

/* Function to create value ranges that span both words */
static __int128 create_range_boundary(int selector) {
    switch(selector) {
        case 0: return (__int128)0;
        case 1: return (__int128)1;
        case 2: return (__int128)-1;
        case 3: return (__int128)HIGH_BIT_64;  /* Crosses into high word */
        case 4: return ((__int128)HIGH_BIT_64 << 64);  /* Only high word set */
        case 5: return ((__int128)MAX_64 << 64) | MAX_64;  /* All bits set */
        case 6: return ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64;
        default: return (__int128)selector;
    }
}

/* Test high-word comparisons (both positive and negative) */
static int test_high_word_comparisons(void) {
    int checksum = 0;
    
    /* Values where only high words differ (positive case) */
    __int128 a1 = ((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL;
    __int128 b1 = ((__int128)0x2ULL << 64) | 0x123456789ABCDEF0ULL;
    
    if (a1 < b1) checksum += 1;  /* Should be true */
    if (b1 > a1) checksum += 2;  /* Should be true */
    
    /* Values where only high words differ (negative case) */
    __int128 a2 = ((__int128)(-2LL) << 64) | 0x123456789ABCDEF0ULL;
    __int128 b2 = ((__int128)(-1LL) << 64) | 0x123456789ABCDEF0ULL;
    
    if (a2 < b2) checksum += 4;  /* Should be true (negative numbers) */
    if (b2 > a2) checksum += 8;  /* Should be true */
    
    /* High words equal, low words differ */
    __int128 a3 = ((__int128)0x5ULL << 64) | 0x1ULL;
    __int128 b3 = ((__int128)0x5ULL << 64) | 0x2ULL;
    
    if (a3 < b3) checksum += 16;
    if (b3 > a3) checksum += 32;
    
    return checksum;
}

/* Test boundary values and overflow detection */
static int test_boundary_comparisons(void) {
    int checksum = 0;
    
    /* Near INT128_MAX */
    __int128 near_max = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 max_minus_one = near_max - 1;
    
    if (max_minus_one < near_max) checksum += 1;
    if (near_max > max_minus_one) checksum += 2;
    
    /* Near INT128_MIN */
    __int128 min_plus_one = ((__int128)(-0x7FFFFFFFFFFFFFFFLL - 1) << 64);
    __int128 min_val = min_plus_one - 1;
    
    if (min_val < min_plus_one) checksum += 4;
    if (min_plus_one > min_val) checksum += 8;
    
    /* Test unsigned comparisons */
    unsigned __int128 ua = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    unsigned __int128 ub = ((unsigned __int128)MAX_64 << 64) | (MAX_64 - 1);
    
    if (ua > ub) checksum += 16;
    if (ub < ua) checksum += 32;
    
    return checksum;
}

/* Test with bitwise operations crossing 64-bit boundary */
static int test_bitwise_operations(void) {
    int checksum = 0;
    
    __int128 mask = ((__int128)0xF0F0F0F0F0F0F0F0ULL << 64) | 0x0F0F0F0F0F0F0F0FULL;
    __int128 value = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    __int128 and_result = value & mask;
    __int128 or_result = value | mask;
    __int128 shift_left = value << 65;  /* Crosses word boundary */
    __int128 shift_right = value >> 65;
    
    /* Comparisons after bitwise operations */
    if (and_result < or_result) checksum += 1;
    if (shift_left > shift_right) checksum += 2;
    
    /* Test with arithmetic shift for negative numbers */
    __int128 neg_value = -value;
    __int128 arith_shift = neg_value >> 65;
    
    if (arith_shift < 0) checksum += 4;
    
    return checksum;
}

/* Test range analysis with loops */
static int test_range_analysis(void) {
    int checksum = 0;
    
    /* Loop with __int128 induction variable */
    for (__int128 i = ((__int128)0x7FFFFFFFFFFFFF00ULL << 64); 
         i < ((__int128)0x7FFFFFFFFFFFFF10ULL << 64); 
         i += ((__int128)1 << 64)) {
        checksum += (i > 0) ? 1 : 0;
    }
    
    /* Overflow detection with builtins */
    __int128 x = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64);
    __int128 y = 0x1000000000000000ULL;
    __int128 overflow_result;
    
    if (__builtin_add_overflow(x, y, &overflow_result)) {
        checksum += 2;  /* Should overflow */
    }
    
    /* Multiplication that requires wide comparison */
    __int128 a = 0x1000000000000000ULL;
    __int128 b = 0x1000000000000000ULL;
    __int128 mul_result;
    
    if (__builtin_mul_overflow(a, b, &mul_result)) {
        checksum += 4;
    } else {
        if (mul_result > a) checksum += 8;
    }
    
    return checksum;
}

/* Test mixed-precision operations */
static int test_mixed_precision(void) {
    int checksum = 0;
    
    __int128 wide_val = ((__int128)0x123456789ABCDEF0ULL << 64);
    long long narrow_val = 0x123456789ABCDEF0LL;
    unsigned long long ul_narrow = 0xFEDCBA9876543210ULL;
    size_t size_val = (size_t)0xFFFFFFFFFFFFFFFFULL;
    
    /* Mixed comparisons */
    if (wide_val > narrow_val) checksum += 1;
    if ((unsigned __int128)wide_val < ul_narrow) checksum += 2;
    if (wide_val != size_val) checksum += 4;
    
    /* Ternary operator with mixed types */
    __int128 ternary_result = (checksum > 2) ? wide_val : narrow_val;
    if (ternary_result == wide_val) checksum += 8;
    
    /* Variadic function argument (triggers conversions) */
    printf("Mixed precision check: %lld\n", (long long)(wide_val >> 64));
    
    return checksum;
}

/* Test with compiler builtins */
static int test_builtin_functions(void) {
    int checksum = 0;
    
    unsigned __int128 uval = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) |
                             0xFEDCBA9876543210ULL;
    
    /* Count leading zeros in high word */
    int clz_high = __builtin_clzll((unsigned long long)(uval >> 64));
    /* Count trailing zeros in low word */
    int ctz_low = __builtin_ctzll((unsigned long long)uval);
    
    checksum += clz_high + ctz_low;
    
    /* Population count of entire 128-bit value */
    unsigned long long high_pop = __builtin_popcountll((unsigned long long)(uval >> 64));
    unsigned long long low_pop = __builtin_popcountll((unsigned long long)uval);
    checksum += high_pop + low_pop;
    
    /* Branch prediction with wide comparison */
    __int128 a = ((__int128)0x1ULL << 64);
    __int128 b = ((__int128)0x2ULL << 64);
    
    if (__builtin_expect(a < b, 1)) {
        checksum += 16;
    }
    
    return checksum;
}

/* Main test driver with array operations */
int main(void) {
    int total_checksum = 0;
    
    printf("Testing double_int comparison logic...\n");
    
    /* Run all test suites */
    total_checksum += test_high_word_comparisons();
    total_checksum += test_boundary_comparisons();
    total_checksum += test_bitwise_operations();
    total_checksum += test_range_analysis();
    total_checksum += test_mixed_precision();
    total_checksum += test_builtin_functions();
    
    /* Array operations to give optimizer substantial work */
    __int128 array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = create_range_boundary(i);
    }
    
    /* Perform comparisons across array elements */
    for (int i = 0; i < 7; i++) {
        if (array[i] < array[i + 1]) {
            total_checksum += i;
        }
        if (array[i] > array[i + 1]) {
            total_checksum -= i;
        }
    }
    
    /* Complex expression forcing constant folding */
    __int128 complex_expr = 
        ((__int128)array[3] << 2) + 
        (array[4] >> 1) - 
        (array[5] & array[6]) | 
        array[7];
    
    if (complex_expr != 0) {
        total_checksum += (int)(complex_expr & 0xFF);
    }
    
    printf("Final checksum: %d\n", total_checksum);
    printf("Test completed. Compile with:\n");
    printf("  gcc -O3 -fstrict-overflow -Wstrict-overflow=5 -c test.c\n");
    printf("  gcc -O2 -fdump-tree-all -fdump-rtl-all -c test.c\n");
    
    return total_checksum != 0 ? 0 : 1;
}
