/* test-double-int.c */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64 0x8000000000000000ULL
#define MAX_64 0xFFFFFFFFFFFFFFFFULL
#define LARGE_CONST_128 ((__int128)HIGH_BIT_64 << 64 | HIGH_BIT_64)
#define NEG_LARGE_CONST_128 (-((__int128)HIGH_BIT_64 << 64 | HIGH_BIT_64))

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)0x1ULL << 120) > (((__int128)0x1ULL << 119) * 2 - 1), 
               "Compile-time 128-bit comparison 1");
_Static_assert((unsigned __int128)MAX_64 < (unsigned __int128)(MAX_64 + 1),
               "Compile-time 128-bit comparison 2");

/* Function to exercise range analysis with __int128 */
__int128 range_analysis_func(int x) {
    __int128 result = 0;
    /* Create value ranges that span both high and low words */
    if (x > 0) {
        result = ((__int128)x << 60) + HIGH_BIT_64;
    } else if (x < 0) {
        result = -((__int128)(-x) << 60) - HIGH_BIT_64;
    } else {
        result = (__int128)HIGH_BIT_64 << 64;
    }
    return result;
}

/* Function using builtin overflow checks with __int128 */
int check_overflow_ops(__int128 a, __int128 b, __int128 *sum, __int128 *prod) {
    int overflow_sum = 0, overflow_prod = 0;
    
    /* These builtins may trigger double_int comparisons internally */
    overflow_sum = __builtin_add_overflow(a, b, sum);
    overflow_prod = __builtin_mul_overflow(a, b, prod);
    
    /* Comparisons that exercise the uncovered code path */
    if (*sum < a && a > 0) return -1;
    if (*prod > b && b < 0) return 1;
    
    return overflow_sum | overflow_prod;
}

/* Mixed-precision comparison function */
int mixed_precision_compare(__int128 a, unsigned long long b) {
    /* Force conversions and comparisons */
    if ((unsigned __int128)a < (unsigned __int128)b) return -1;
    if ((__int128)b > a) return 1;
    
    /* Ternary with different types */
    return (a > (__int128)b) ? 1 : ((a < (__int128)b) ? -1 : 0);
}

/* Switch statement with __int128 case labels (compile-time constants) */
const char* classify_int128(__int128 val) {
    switch (val) {
        case ((__int128)0ULL):
            return "zero";
        case ((__int128)1ULL << 63):
            return "2^63";
        case ((__int128)HIGH_BIT_64 << 64):
            return "2^127";
        case -((__int128)HIGH_BIT_64 << 64):
            return "-2^127";
        default:
            if (val > ((__int128)HIGH_BIT_64 << 64 | HIGH_BIT_64))
                return "very large";
            else if (val < -((__int128)HIGH_BIT_64 << 64 | HIGH_BIT_64))
                return "very small";
            return "other";
    }
}

/* Loop with __int128 induction variable near 64-bit boundaries */
void int128_loop_test(__int128 start, __int128 end, int *counter) {
    for (__int128 i = start; i < end; i += (end - start) / 10) {
        /* Comparisons that exercise high/low word logic */
        if (i < start + (end - start)/2) {
            (*counter)++;
        }
        
        /* Bitwise operations crossing 64-bit boundary */
        __int128 shifted = (i << 1) | (i >> 127);
        if (shifted & ((__int128)1 << 120)) {
            (*counter)--;
        }
    }
}

/* Use builtin functions that may operate on wide integers */
int count_leading_zeros_int128(__int128 x) {
    /* Split and use builtins on 64-bit parts */
    unsigned long long high = (unsigned long long)(x >> 64);
    unsigned long long low = (unsigned long long)x;
    
    if (high != 0) {
        return __builtin_clzll(high);
    } else if (low != 0) {
        return 64 + __builtin_clzll(low);
    }
    return 128;
}

int main(void) {
    int checksum = 0;
    
    /* Test 1: Compare values where only high words differ */
    __int128 test1_a = (__int128)HIGH_BIT_64 << 64;      /* 2^127 */
    __int128 test1_b = (__int128)(HIGH_BIT_64 - 1) << 64; /* 2^127 - 2^64 */
    
    checksum += (test1_a > test1_b) ? 1 : 0;
    checksum += (test1_b < test1_a) ? 2 : 0;
    
    /* Test 2: Compare values where high words equal, low words differ */
    __int128 test2_a = ((__int128)HIGH_BIT_64 << 64) | 0x1ULL;
    __int128 test2_b = ((__int128)HIGH_BIT_64 << 64) | 0x2ULL;
    
    checksum += (test2_a < test2_b) ? 4 : 0;
    checksum += (test2_b > test2_a) ? 8 : 0;
    
    /* Test 3: Boundary comparisons (signed/unsigned) */
    __int128 max_signed = ((__int128)HIGH_BIT_64 << 64) - 1;
    __int128 min_signed = -((__int128)HIGH_BIT_64 << 64);
    unsigned __int128 max_unsigned = ~(unsigned __int128)0;
    
    checksum += (max_signed > min_signed) ? 16 : 0;
    checksum += ((unsigned __int128)max_unsigned > (unsigned __int128)max_signed) ? 32 : 0;
    
    /* Test 4: Arithmetic with overflow checking */
    __int128 sum, prod;
    int overflow = check_overflow_ops(max_signed / 2, max_signed / 2, &sum, &prod);
    checksum += overflow * 64;
    
    /* Test 5: Mixed precision comparisons */
    checksum += mixed_precision_compare(test1_a, HIGH_BIT_64);
    
    /* Test 6: Range analysis function */
    __int128 range_result = range_analysis_func(100);
    checksum += (range_result > 0) ? 128 : 0;
    
    /* Test 7: Loop with __int128 induction variable */
    int loop_counter = 0;
    int128_loop_test(-((__int128)1 << 62), (__int128)1 << 62, &loop_counter);
    checksum += loop_counter;
    
    /* Test 8: Array operations with __int128 */
    __int128 arr[8] = {
        0,
        (__int128)1 << 63,
        (__int128)1 << 64,
        (__int128)1 << 96,
        -((__int128)1 << 63),
        -((__int128)1 << 64),
        -((__int128)1 << 96),
        LARGE_CONST_128
    };
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Multiple comparisons to exercise all paths */
            if (arr[i] < arr[j]) checksum += 1;
            if (arr[i] > arr[j]) checksum += 2;
            if ((unsigned __int128)arr[i] < (unsigned __int128)arr[j]) checksum += 4;
            if ((unsigned __int128)arr[i] > (unsigned __int128)arr[j]) checksum += 8;
        }
    }
    
    /* Test 9: Builtin function usage */
    checksum += count_leading_zeros_int128((__int128)1 << 120);
    
    /* Test 10: Ternary operator with different types */
    long long narrow = HIGH_BIT_64;
    __int128 wide = (__int128)narrow << 32;
    __int128 ternary_result = (checksum & 1) ? narrow : wide;
    checksum += (ternary_result == narrow) ? 256 : 512;
    
    /* Force use of printf with __int128 (triggers conversions) */
    printf("Checksum: %d\n", checksum);
    printf("Sample values: %lld (high), %lld (low)\n", 
           (long long)(test1_a >> 64), (long long)test1_a);
    
    /* Use __builtin_expect with __int128 comparison */
    if (__builtin_expect(test1_a > test2_b, 1)) {
        checksum += 1024;
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;
}
