/* test-double-int.c - Designed to trigger GCC's double_int comparison logic */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_UINT64    0xFFFFFFFFFFFFFFFFULL
#define HIGH_WORD_DIFF 0x1000000000000000ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)MAX_UINT64) < ((__int128)HIGH_BIT_64 << 64),
               "Comparison with high word difference");

/* Test function that exercises __int128 range analysis */
static __int128 process_range(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with __int128 induction variable near 64-bit boundaries */
    for (__int128 i = start; i < end; i += (HIGH_BIT_64 >> 61)) {
        /* Force VRP to analyze __int128 ranges */
        if (i > (HIGH_BIT_64 << 32)) {
            sum += i * 2;
        } else if (i < -((__int128)HIGH_BIT_64 << 32)) {
            sum -= i;
        } else {
            sum += i;
        }
        
        /* Overflow checks that require wide comparisons */
        __int128 overflow_test;
        if (__builtin_add_overflow(sum, i, &overflow_test)) {
            sum = i;
        }
    }
    return sum;
}

/* Function comparing high words specifically */
static int compare_high_words(__int128 a, __int128 b) {
    /* These comparisons should trigger the uncovered lines */
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

/* Mixed-precision operations */
static __int128 mixed_precision_comparisons(unsigned long long ull_val, 
                                           __int128 i128_val) {
    __int128 result = 0;
    
    /* Compare __int128 with narrower types */
    if (i128_val > (__int128)ull_val) {
        result += i128_val;
    }
    
    /* Ternary with mixed types */
    result = (ull_val > 1000) ? (__int128)ull_val * 2 : i128_val / 2;
    
    return result;
}

/* Bitwise operations crossing 64-bit boundary */
static __int128 cross_boundary_bitops(__int128 val) {
    /* Shift operations that move bits across the 64-bit boundary */
    __int128 shifted = val << 37;
    
    /* Mask operations affecting both high and low words */
    __int128 masked = shifted & (((__int128)MAX_UINT64 << 64) | MAX_UINT64);
    
    /* Bitwise OR that sets high word bits */
    __int128 high_set = masked | ((__int128)HIGH_BIT_64 << 32);
    
    return high_set;
}

/* Switch statement with __int128 case labels (compile-time constants) */
static int switch_with_i128(__int128 key) {
    switch ((unsigned __int128)key) {  /* Cast to unsigned for switch */
        case ((unsigned __int128)0x1ULL << 64):
            return 1;
        case ((unsigned __int128)0x2ULL << 64):
            return 2;
        case ((unsigned __int128)0x3ULL << 64) | 0xFFFFFFFFULL:
            return 3;
        case ((unsigned __int128)MAX_UINT64 << 64):
            return 4;
        default:
            return 0;
    }
}

/* Array operations with __int128 */
static __int128 process_array(__int128 arr[], int size) {
    __int128 checksum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Comparisons that exercise high/low word logic */
        if (i > 0) {
            if (arr[i] > arr[i-1]) {
                checksum += arr[i];
            } else if (arr[i] < arr[i-1]) {
                checksum -= arr[i];
            }
        }
        
        /* Boundary checks */
        if (arr[i] > ((__int128)HIGH_BIT_64 << 64)) {
            checksum |= ((__int128)1 << 120);
        }
        
        if (arr[i] < -((__int128)HIGH_BIT_64 << 63)) {
            checksum &= ~((__int128)1 << 120);
        }
    }
    
    return checksum;
}

/* Use builtins with __int128 */
static int count_leading_zeros_i128(__int128 val) {
    /* Count leading zeros by examining high word first */
    unsigned long long high = (unsigned long long)((unsigned __int128)val >> 64);
    unsigned long long low = (unsigned long long)val;
    
    if (high != 0) {
        return __builtin_clzll(high);
    } else {
        return 64 + __builtin_clzll(low);
    }
}

int main(void) {
    __int128 checksum = 0;
    
    /* Test 1: Values where only high words differ */
    __int128 test1_a = ((__int128)HIGH_BIT_64 << 64);
    __int128 test1_b = ((__int128)(HIGH_BIT_64 | 0x1ULL) << 64);
    checksum += compare_high_words(test1_a, test1_b);
    checksum += compare_high_words(test1_b, test1_a);
    checksum += compare_high_words(test1_a, test1_a);
    
    /* Test 2: High words equal, low words differ */
    __int128 test2_a = ((__int128)HIGH_BIT_64 << 64) | 0x1ULL;
    __int128 test2_b = ((__int128)HIGH_BIT_64 << 64) | 0x2ULL;
    checksum += compare_high_words(test2_a, test2_b) * 2;
    
    /* Test 3: Boundary values */
    __int128 max_pos = ((__int128)MAX_UINT64 << 64) | MAX_UINT64;
    __int128 min_neg = ((__int128)HIGH_BIT_64 << 64);
    
    checksum += compare_high_words(max_pos, min_neg) * 3;
    checksum += compare_high_words(min_neg, max_pos) * 4;
    
    /* Test 4: Range analysis with loops */
    checksum += process_range(-((__int128)HIGH_BIT_64 << 32), 
                              ((__int128)HIGH_BIT_64 << 32));
    
    /* Test 5: Mixed precision */
    checksum += mixed_precision_comparisons(MAX_UINT64, max_pos);
    checksum += mixed_precision_comparisons(0, min_neg);
    
    /* Test 6: Bitwise operations */
    __int128 bitwise_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 
                           0xFEDCBA9876543210ULL;
    checksum += cross_boundary_bitops(bitwise_val);
    
    /* Test 7: Array operations (8 elements as requested) */
    __int128 arr[8] = {
        0,
        ((__int128)1 << 64),
        ((__int128)HIGH_BIT_64 << 63),
        ((__int128)MAX_UINT64 << 64),
        -((__int128)1 << 66),
        ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL,
        ((__int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x5555555555555555ULL,
        ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64
    };
    
    checksum += process_array(arr, 8);
    
    /* Test 8: Switch statement */
    for (int i = 0; i < 8; i++) {
        checksum += switch_with_i128(arr[i]);
    }
    
    /* Test 9: Builtin functions */
    checksum += count_leading_zeros_i128(max_pos);
    checksum += count_leading_zeros_i128(min_neg);
    checksum += count_leading_zeros_i128(1);
    
    /* Test 10: Overflow operations */
    __int128 of_test1 = ((__int128)MAX_UINT64 << 63);
    __int128 of_test2 = ((__int128)MAX_UINT64 << 63);
    __int128 of_result;
    
    if (__builtin_add_overflow(of_test1, of_test2, &of_result)) {
        checksum |= ((__int128)1 << 64);
    }
    
    if (__builtin_mul_overflow(of_test1, 2, &of_result)) {
        checksum |= ((__int128)1 << 65);
    }
    
    /* Test 11: __builtin_expect with __int128 comparisons */
    __int128 likely_val = 1000;
    __int128 unlikely_val = ((__int128)HIGH_BIT_64 << 64);
    
    if (__builtin_expect(likely_val < unlikely_val, 1)) {
        checksum += 1000;
    }
    
    /* Print checksum to prevent dead code elimination */
    /* Split 128-bit checksum into two 64-bit parts for printing */
    unsigned long long high_part = (unsigned long long)((unsigned __int128)checksum >> 64);
    unsigned long long low_part = (unsigned long long)checksum;
    
    printf("Checksum high: 0x%016llx\n", high_part);
    printf("Checksum low:  0x%016llx\n", low_part);
    printf("Total checksum: 0x%016llx%016llx\n", high_part, low_part);
    
    return 0;
}
