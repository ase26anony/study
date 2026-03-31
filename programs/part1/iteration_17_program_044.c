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
#define LARGE_CONST_128 ((__int128)0x123456789ABCDEF0ULL << 64 | 0xFEDCBA9876543210ULL)
#define NEG_LARGE_CONST_128 ((__int128)0x8000000000000000ULL << 64 | 0x0000000000000001ULL)

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)1 << 120) > (((__int128)1 << 120) - 1), 
               "Compile-time comparison 1");
_Static_assert((unsigned __int128)MAX_64 < (unsigned __int128)(MAX_64 + 1),
               "Compile-time comparison 2");

/* Function to exercise range analysis with __int128 */
static __int128 process_range(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with __int128 induction variable near 64-bit boundaries */
    for (__int128 i = start; i < end; i += (end - start) / 100) {
        /* Force comparisons that might use double_int logic */
        if (i < 0) {
            sum -= i;
        } else if (i > ((__int128)HIGH_BIT_64 << 64)) {
            sum += i >> 4;
        } else {
            sum += i;
        }
        
        /* Cross 64-bit boundary comparisons */
        if ((unsigned __int128)i < (unsigned __int128)(HIGH_BIT_64 << 64)) {
            sum |= 1;
        }
    }
    return sum;
}

/* Function with overflow checks requiring wide comparisons */
static int check_overflow_128(__int128 a, __int128 b) {
    __int128 result;
    
    /* Use builtins that may trigger internal comparisons */
    if (__builtin_add_overflow(a, b, &result)) {
        return -1;  /* Overflow in addition */
    }
    
    if (__builtin_mul_overflow(a, b, &result)) {
        return -2;  /* Overflow in multiplication */
    }
    
    /* Force comparison between signed and unsigned */
    if ((unsigned __int128)a > (unsigned __int128)b) {
        return 1;
    }
    
    return 0;
}

/* Switch statement with __int128 case labels (compile-time constants) */
static int switch_128(__int128 val) {
    switch (val) {
        case ((__int128)0 << 64 | 0):
            return 0;
        case ((__int128)1 << 64 | 0):
            return 1;
        case ((__int128)HIGH_BIT_64 << 64 | 0):
            return 2;
        case ((__int128)MAX_64 << 64 | MAX_64):
            return 3;
        case NEG_LARGE_CONST_128:
            return 4;
        default:
            return -1;
    }
}

/* Mixed precision operations */
static __int128 mixed_precision_ops(long long a, unsigned long long b, size_t c) {
    /* Ternary with different types */
    __int128 result = (a > 0) ? (__int128)a : (__int128)b;
    
    /* Comparisons between different types */
    if (result < (__int128)c) {
        result += (__int128)c << 32;
    }
    
    /* Bitwise operations crossing 64-bit boundary */
    result = (result << 64) | (result >> 64);
    
    return result;
}

/* Array operations to give optimizer substantial work */
static __int128 process_array(__int128 arr[], int size) {
    __int128 checksum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Different comparison patterns */
        if (i % 4 == 0) {
            /* Compare where only high words differ */
            if (arr[i] < LARGE_CONST_128) {
                checksum += arr[i];
            }
        } else if (i % 4 == 1) {
            /* Compare where high words equal but low words differ */
            __int128 masked = arr[i] & ~((__int128)MAX_64);
            if (masked == (((__int128)MAX_64 << 64) & ~((__int128)MAX_64))) {
                checksum -= arr[i] & MAX_64;
            }
        } else if (i % 4 == 2) {
            /* Boundary comparisons */
            if (arr[i] > 0 && (unsigned __int128)arr[i] < (unsigned __int128)(HIGH_BIT_64 << 64)) {
                checksum ^= arr[i];
            }
        } else {
            /* Use builtin_expect with __int128 comparison */
            if (__builtin_expect(arr[i] != 0, 1)) {
                checksum |= arr[i];
            }
        }
        
        /* Force overflow arithmetic */
        __int128 temp;
        if (!__builtin_add_overflow(checksum, arr[i] * 3, &temp)) {
            checksum = temp;
        }
    }
    
    return checksum;
}

/* Variadic function to trigger conversions */
static void print_128_values(int count, ...) {
    /* This would normally use va_args, but we'll simulate for coverage */
    __int128 values[] = {
        0,
        ((__int128)1 << 127) - 1,  /* INT128_MAX-ish */
        (__int128)-1 << 127,       /* INT128_MIN-ish */
        (__int128)MAX_64 << 64 | MAX_64,  /* UINT128_MAX-ish */
    };
    
    for (int i = 0; i < 4; i++) {
        /* Comparisons during formatting simulation */
        if (values[i] < 0) {
            printf("Negative large value\n");
        } else if ((unsigned __int128)values[i] > (unsigned __int128)(HIGH_BIT_64 << 64)) {
            printf("Large positive value\n");
        }
    }
}

/* Bit manipulation functions */
static int count_leading_zeroes_128(unsigned __int128 x) {
    /* Operations that may use double_int comparisons internally */
    if (x == 0) return 128;
    
    int count = 0;
    while ((x & ((unsigned __int128)1 << 127)) == 0) {
        x <<= 1;
        count++;
        /* Comparison in loop condition */
        if (count >= 128) break;
    }
    
    return count;
}

int main(void) {
    __int128 checksum = 0;
    
    /* Test 1: Comparisons where high words differ */
    __int128 large1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111111ULL;
    __int128 large2 = ((__int128)0x123456789ABCDEF1ULL << 64) | 0x1111111111111111ULL;
    
    if (large1 < large2) checksum += 1;  /* Should trigger high word comparison */
    if (large2 > large1) checksum += 2;
    
    /* Test 2: Comparisons where high words equal, low words differ */
    __int128 same_high1 = ((__int128)0x5555555555555555ULL << 64) | 0x0000000000000001ULL;
    __int128 same_high2 = ((__int128)0x5555555555555555ULL << 64) | 0x0000000000000002ULL;
    
    if (same_high1 < same_high2) checksum += 4;  /* Should trigger low word comparison */
    if (same_high2 > same_high1) checksum += 8;
    
    /* Test 3: Boundary values */
    __int128 near_max = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 near_min = ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
    
    if (near_max > near_min) checksum += 16;
    if ((unsigned __int128)near_max < (unsigned __int128)near_min) checksum += 32;
    
    /* Test 4: Range analysis */
    checksum += process_range(-1000, 1000);
    checksum += process_range(HIGH_BIT_64 << 32, (HIGH_BIT_64 << 32) + 1000);
    
    /* Test 5: Overflow checks */
    checksum += check_overflow_128(
        ((__int128)1 << 126),
        ((__int128)1 << 126)
    );
    
    /* Test 6: Switch with __int128 */
    checksum += switch_128(((__int128)1 << 64) | 0);
    checksum += switch_128(NEG_LARGE_CONST_128);
    
    /* Test 7: Mixed precision */
    checksum += mixed_precision_ops(LLONG_MAX, ULLONG_MAX, SIZE_MAX);
    
    /* Test 8: Array processing (8+ elements as requested) */
    __int128 arr[12];
    for (int i = 0; i < 12; i++) {
        arr[i] = ((__int128)i << (i * 4)) | (0xFEDCBA9876543210ULL >> (i * 4));
    }
    checksum += process_array(arr, 12);
    
    /* Test 9: Bit manipulation */
    checksum += count_leading_zeroes_128((unsigned __int128)1 << 120);
    checksum += count_leading_zeroes_128((unsigned __int128)0);
    
    /* Test 10: Variadic-like conversions */
    print_128_values(4, 0, 0, 0, 0);
    
    /* Final output to prevent dead code elimination */
    printf("Checksum (low 64 bits): %llx\n", (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    printf("Checksum (high 64 bits): %llx\n", (unsigned long long)(checksum >> 64));
    
    return 0;
}
