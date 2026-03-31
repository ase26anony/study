/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64    0x8000000000000000ULL
#define MAX_64         0xFFFFFFFFFFFFFFFFULL
#define MID_64         0x7FFFFFFFFFFFFFFFULL

/* Signed 128-bit constants */
static const __int128 INT128_MAX = ((__int128)MAX_64 << 64) | MAX_64;
static const __int128 INT128_MIN = ((__int128)HIGH_BIT_64 << 64);
static const __int128 MID_POS_128 = ((__int128)MID_64 << 64) | MID_64;
static const __int128 MID_NEG_128 = ((__int128)HIGH_BIT_64 << 64) | MID_64;

/* Unsigned 128-bit constants */
static const unsigned __int128 UINT128_MAX = 
    ((unsigned __int128)MAX_64 << 64) | MAX_64;
static const unsigned __int128 HIGH_ONLY = 
    ((unsigned __int128)MAX_64 << 64);

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) < 0, 
               "Negative 128-bit constant");
_Static_assert(((__int128)MID_64 << 64) > 0, 
               "Positive 128-bit constant");
_Static_assert(((unsigned __int128)MAX_64 << 64) > MAX_64, 
               "High word comparison in unsigned");

/* Function to create value ranges that span both words */
static __int128 create_range_based_value(int selector) {
    __int128 result = 0;
    
    /* These branches create different value ranges for VRP */
    if (selector & 1) {
        result = ((__int128)HIGH_BIT_64 << 64) | selector;
    } else {
        result = ((__int128)MID_64 << 64) | selector;
    }
    
    if (selector & 2) {
        result = -result;
    }
    
    return result;
}

/* Force high-word comparisons */
static int test_high_word_comparisons(void) {
    int checksum = 0;
    
    /* Comparisons where high words differ (both positive) */
    __int128 a1 = ((__int128)0x1ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 b1 = ((__int128)0x2ULL << 64) | 0x0ULL;
    checksum += (a1 < b1) ? 1 : 0;  /* Should be true */
    checksum += (b1 > a1) ? 2 : 0;  /* Should be true */
    
    /* Comparisons where high words differ (negative vs positive) */
    __int128 neg_high = ((__int128)HIGH_BIT_64 << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 pos_high = ((__int128)0x1ULL << 64) | 0x0ULL;
    checksum += (neg_high < pos_high) ? 4 : 0;  /* Should be true */
    checksum += (pos_high > neg_high) ? 8 : 0;  /* Should be true */
    
    /* Comparisons where high words are equal but low words differ */
    __int128 c1 = ((__int128)0x1ULL << 64) | 0x1ULL;
    __int128 c2 = ((__int128)0x1ULL << 64) | 0x2ULL;
    checksum += (c1 < c2) ? 16 : 0;  /* Should be true */
    checksum += (c2 > c1) ? 32 : 0;  /* Should be true */
    
    return checksum;
}

/* Test boundary values */
static int test_boundary_comparisons(void) {
    int checksum = 0;
    
    /* INT128_MIN < INT128_MAX */
    checksum += (INT128_MIN < INT128_MAX) ? 1 : 0;
    
    /* INT128_MAX > 0 */
    checksum += (INT128_MAX > 0) ? 2 : 0;
    
    /* INT128_MIN < 0 */
    checksum += (INT128_MIN < 0) ? 4 : 0;
    
    /* UINT128_MAX comparisons */
    unsigned __int128 u1 = UINT128_MAX;
    unsigned __int128 u2 = UINT128_MAX - 1;
    checksum += (u1 > u2) ? 8 : 0;
    checksum += (u2 < u1) ? 16 : 0;
    
    /* Cross-type comparisons */
    long long ll_max = LLONG_MAX;
    __int128 i128_from_ll = ll_max;
    checksum += (i128_from_ll > ll_max - 1) ? 32 : 0;
    
    return checksum;
}

/* Test overflow operations that require wide comparisons */
static int test_overflow_comparisons(void) {
    int checksum = 0;
    
    __int128 x = ((__int128)MID_64 << 64) | MID_64;
    __int128 y = ((__int128)0x1ULL << 64) | 0x1ULL;
    
    /* These may trigger overflow checking logic */
    __int128 sum = x + y;
    __int128 diff = x - y;
    __int128 prod = x * 2;
    
    checksum += (sum > x) ? 1 : 0;
    checksum += (diff < x) ? 2 : 0;
    checksum += (prod > x) ? 4 : 0;
    
    /* Use builtin overflow checks */
    __int128 of_result;
    int overflow = __builtin_add_overflow(x, y, &of_result);
    checksum += overflow ? 8 : 0;
    
    overflow = __builtin_mul_overflow(x, 3, &of_result);
    checksum += overflow ? 16 : 0;
    
    return checksum;
}

/* Test bitwise operations crossing 64-bit boundaries */
static int test_bitwise_comparisons(void) {
    int checksum = 0;
    
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    __int128 mask_high = ((__int128)0xFFFFFFFFULL << 64);
    __int128 mask_all = mask_low | mask_high;
    
    __int128 val1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 
                    0xFEDCBA9876543210ULL;
    __int128 val2 = ((__int128)0x0FEDCBA987654321ULL << 64) | 
                    0x123456789ABCDEF0ULL;
    
    __int128 and_result = val1 & val2;
    __int128 or_result = val1 | val2;
    __int128 xor_result = val1 ^ val2;
    __int128 shift_left = val1 << 32;
    __int128 shift_right = val1 >> 32;
    
    checksum += (and_result < val1) ? 1 : 0;
    checksum += (or_result > val1) ? 2 : 0;
    checksum += (xor_result != 0) ? 4 : 0;
    checksum += (shift_left > val1) ? 8 : 0;
    checksum += (shift_right < val1) ? 16 : 0;
    
    return checksum;
}

/* Test with arrays to give optimizer substantial work */
static int test_array_comparisons(void) {
    __int128 arr[8] = {
        INT128_MIN,
        ((__int128)HIGH_BIT_64 << 64) | 0x1ULL,
        -100,
        0,
        100,
        ((__int128)0x1ULL << 64) | 0x0ULL,
        MID_POS_128,
        INT128_MAX
    };
    
    int checksum = 0;
    
    /* Compare all pairs to exercise comparison logic */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i != j) {
                checksum += (arr[i] < arr[j]) ? (1 << (i & 3)) : 0;
                checksum += (arr[i] > arr[j]) ? (1 << ((j + 1) & 3)) : 0;
                checksum += (arr[i] == arr[j]) ? 0 : (1 << ((i + j) & 3));
            }
        }
    }
    
    return checksum;
}

/* Test switch statement with __int128 case labels */
static int test_switch_comparisons(__int128 value) {
    int result = 0;
    
    /* Force compiler to generate comparison trees for switch */
    switch ((unsigned __int128)value & 0xFF) {
        case 0x00:
            result = 1;
            break;
        case ((unsigned __int128)0x80ULL << 56):
            result = 2;
            break;
        case ((unsigned __int128)0xFFULL << 64) & 0xFF:
            result = 3;
            break;
        default:
            result = 4;
            break;
    }
    
    return result;
}

/* Test mixed-precision operations */
static int test_mixed_precision(void) {
    int checksum = 0;
    
    /* Ternary with different types */
    __int128 large_val = ((__int128)0x1ULL << 64);
    long long small_val = 100;
    
    __int128 ternary_result = (small_val > 50) ? large_val : small_val;
    checksum += (ternary_result == large_val) ? 1 : 0;
    
    /* Comparison with size_t */
    size_t size_val = SIZE_MAX;
    __int128 i128_from_size = size_val;
    checksum += (i128_from_size > size_val - 1) ? 2 : 0;
    
    /* Variadic function argument (triggers conversions) */
    __int128 print_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 
                         0xFEDCBA9876543210ULL;
    /* Note: printf with %lld for __int128 is implementation-defined */
    checksum += 4;  // Placeholder for conversion effect
    
    return checksum;
}

/* Test builtin functions */
static int test_builtin_functions(void) {
    int checksum = 0;
    
    unsigned __int128 uval = ((unsigned __int128)0x1ULL << 64) | 0x8000000000000000ULL;
    
    /* Count leading zeros in high word */
    unsigned long long high_part = (unsigned long long)(uval >> 64);
    int clz_high = __builtin_clzll(high_part);
    checksum += clz_high;
    
    /* Count trailing zeros in low word */
    unsigned long long low_part = (unsigned long long)uval;
    int ctz_low = __builtin_ctzll(low_part);
    checksum += ctz_low;
    
    /* Population count */
    int popcount = __builtin_popcountll(high_part) + __builtin_popcountll(low_part);
    checksum += popcount;
    
    /* __builtin_expect with wide comparison */
    __int128 a = ((__int128)0x1ULL << 64);
    __int128 b = ((__int128)0x2ULL << 64);
    if (__builtin_expect(a < b, 1)) {
        checksum += 64;
    }
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Execute all tests to trigger various comparison paths */
    total_checksum += test_high_word_comparisons();
    total_checksum += test_boundary_comparisons();
    total_checksum += test_overflow_comparisons();
    total_checksum += test_bitwise_comparisons();
    total_checksum += test_array_comparisons();
    
    __int128 switch_val = ((__int128)0x80ULL << 56);
    total_checksum += test_switch_comparisons(switch_val);
    
    total_checksum += test_mixed_precision();
    total_checksum += test_builtin_functions();
    
    /* Create value ranges for VRP */
    for (int i = 0; i < 10; i++) {
        __int128 ranged_val = create_range_based_value(i);
        total_checksum += (ranged_val > 0) ? i : -i;
    }
    
    printf("Total checksum: %d\n", total_checksum);
    printf("All tests completed.\n");
    
    return total_checksum != 0 ? 0 : 1;
}
