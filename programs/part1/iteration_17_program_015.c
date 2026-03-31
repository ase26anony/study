/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64 0x8000000000000000ULL
#define MAX_64 0xFFFFFFFFFFFFFFFFULL
#define LARGE_CONSTANT_128 ((__int128)0x123456789ABCDEF0ULL << 64 | 0xFEDCBA9876543210ULL)
#define NEGATIVE_LARGE_CONSTANT ((__int128)(-1) * ((__int128)0x7FFFFFFFFFFFFFFFULL << 64 | 0xFFFFFFFFFFFFFFFFULL))

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > ((__int128)HIGH_BIT_64 >> 1), 
               "High word comparison should be triggered");
_Static_assert((unsigned __int128)MAX_64 < ((unsigned __int128)MAX_64 << 64),
               "Unsigned high word comparison needed");

/* Test function that exercises __int128 range analysis */
static __int128 process_range(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with __int128 induction variable - forces VRP analysis */
    for (__int128 i = start; i < end; i += (end - start) / 100) {
        /* Mixed operations that may overflow */
        __int128 temp = i * i;
        if (temp < 0) {
            sum -= temp;
        } else {
            sum += temp;
        }
        
        /* Bitwise operations crossing 64-bit boundary */
        sum = (sum << 2) | (sum >> 126);
    }
    return sum;
}

/* Function using builtin overflow checks */
static int check_overflow_operations(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* These builtins may trigger double_int comparisons internally */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) return -1;
    
    overflow = __builtin_mul_overflow(a, b, &result);
    if (overflow) return -2;
    
    overflow = __builtin_sub_overflow(a, b, &result);
    if (overflow) return -3;
    
    return 0;
}

/* Switch statement with __int128 cases - forces comparison tree generation */
static const char* classify_int128(__int128 value) {
    switch (value) {
        case ((__int128)0):
            return "zero";
        case ((__int128)1):
            return "one";
        case ((__int128)HIGH_BIT_64 << 64):
            return "high-bit-only";
        case ((__int128)MAX_64):
            return "max-64";
        case ((__int128)MAX_64 << 64):
            return "high-word-max";
        case NEGATIVE_LARGE_CONSTANT:
            return "negative-large";
        default:
            if (value > 0) return "positive";
            else return "negative";
    }
}

/* Mixed precision comparisons */
static int compare_mixed_types(__int128 a, unsigned long long b) {
    /* Force conversions and comparisons */
    if (a == (__int128)b) return 0;
    if (a < (__int128)b) return -1;
    if (a > (__int128)b) return 1;
    
    /* Ternary with mixed types */
    __int128 result = (a > 0) ? a : (__int128)b;
    return (result == a) ? 100 : 200;
}

/* Array operations to give optimizer substantial work */
static __int128 process_array(__int128 arr[], int size) {
    __int128 checksum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Compare array elements - may trigger the target comparison logic */
        if (i > 0) {
            if (arr[i] < arr[i-1]) checksum -= arr[i];
            else if (arr[i] > arr[i-1]) checksum += arr[i];
            else checksum ^= arr[i];
        }
        
        /* Use builtin expect to influence branch prediction */
        if (__builtin_expect(arr[i] > 0, 1)) {
            checksum = (checksum << 1) | (checksum >> 127);
        }
    }
    
    return checksum;
}

/* Bit manipulation functions that cross 64-bit boundary */
static unsigned __int128 reverse_bits(unsigned __int128 x) {
    unsigned __int128 result = 0;
    for (int i = 0; i < 128; i++) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

static int count_leading_zeros(__int128 x) {
    /* This may trigger internal wide integer comparisons */
    unsigned __int128 ux = (unsigned __int128)x;
    if (ux == 0) return 128;
    
    /* Check high word first */
    unsigned long long high = (unsigned long long)(ux >> 64);
    if (high != 0) {
        return __builtin_clzll(high);
    } else {
        unsigned long long low = (unsigned long long)ux;
        return 64 + __builtin_clzll(low);
    }
}

int main(void) {
    __int128 checksum = 0;
    
    /* Test 1: High word comparisons (both positive and negative) */
    __int128 test_values[] = {
        0,
        1,
        -1,
        (__int128)HIGH_BIT_64,  /* 2^63 */
        (__int128)HIGH_BIT_64 << 64,  /* 2^127 */
        (__int128)HIGH_BIT_64 << 1,   /* 2^64 */
        (__int128)MAX_64,             /* 2^64 - 1 */
        (__int128)MAX_64 << 64,       /* High word all ones */
        NEGATIVE_LARGE_CONSTANT,
        LARGE_CONSTANT_128
    };
    
    int array_size = sizeof(test_values) / sizeof(test_values[0]);
    
    /* Process array - forces many comparisons */
    checksum += process_array(test_values, array_size);
    
    /* Test 2: Explicit comparisons that should trigger the uncovered code */
    
    /* Case where high words differ (both positive) */
    __int128 a = (__int128)HIGH_BIT_64 << 64;  /* High word has bit 63 set */
    __int128 b = (__int128)1 << 64;            /* High word is 1 */
    
    if (a < b) checksum += 1;
    if (a > b) checksum += 2;  /* This should trigger high word comparison */
    
    /* Case where high words differ (negative vs positive) */
    __int128 neg = (__int128)-1 * ((__int128)1 << 120);
    __int128 pos = (__int128)1 << 120;
    
    if (neg < pos) checksum += 4;  /* Should trigger signed high word comparison */
    
    /* Case where high words are equal but low words differ */
    __int128 c = (__int128)0x123456789ABCDEF0ULL << 64 | 0x1111111111111111ULL;
    __int128 d = (__int128)0x123456789ABCDEF0ULL << 64 | 0x2222222222222222ULL;
    
    if (c < d) checksum += 8;   /* Should trigger low word comparison */
    if (c > d) checksum += 16;
    
    /* Test 3: Boundary comparisons */
    __int128 max_signed = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 min_signed = ((__int128)0x8000000000000000ULL << 64);
    
    if (min_signed < max_signed) checksum += 32;
    if (min_signed > max_signed) checksum += 64;
    
    unsigned __int128 max_unsigned = ~((unsigned __int128)0);
    unsigned __int128 mid_unsigned = max_unsigned >> 1;
    
    if (mid_unsigned < max_unsigned) checksum += 128;
    
    /* Test 4: Range analysis triggers */
    __int128 range_result = process_range(-1000, 1000);
    checksum += range_result;
    
    /* Test 5: Overflow checks */
    int overflow_result = check_overflow_operations(
        (__int128)0x7FFFFFFFFFFFFFFFULL << 32,
        (__int128)0x7FFFFFFFFFFFFFFFULL << 32
    );
    checksum += overflow_result;
    
    /* Test 6: Mixed precision */
    int mixed_result = compare_mixed_types(
        (__int128)0x123456789ABCDEF0ULL << 64,
        0xFFFFFFFFFFFFFFFFULL
    );
    checksum += mixed_result;
    
    /* Test 7: Bit manipulation */
    unsigned __int128 reversed = reverse_bits(0xF0F0F0F0F0F0F0F0ULL << 64 | 0x0F0F0F0F0F0F0F0FULL);
    checksum += (__int128)reversed;
    
    int clz_result = count_leading_zeros((__int128)1 << 120);
    checksum += clz_result;
    
    /* Test 8: Classification with switch */
    const char* class1 = classify_int128((__int128)HIGH_BIT_64 << 64);
    const char* class2 = classify_int128(NEGATIVE_LARGE_CONSTANT);
    
    /* Use results to prevent dead code elimination */
    checksum += (__int128)class1[0];
    checksum += (__int128)class2[0];
    
    /* Print checksum in a way that works with 128-bit values */
    unsigned long long low = (unsigned long long)checksum;
    unsigned long long high = (unsigned long long)(checksum >> 64);
    
    printf("Checksum high: 0x%016llx\n", high);
    printf("Checksum low:  0x%016llx\n", low);
    printf("Total checksum (decimal): %lld * 2^64 + %llu\n", 
           (long long)high, low);
    
    return 0;
}
