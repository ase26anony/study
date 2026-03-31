/* test-double-int.c - Designed to trigger double_int comparison logic in GCC */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64 0x8000000000000000ULL
#define MAX_64 0xFFFFFFFFFFFFFFFFULL
#define LARGE_CONST_128 ((__int128)0x123456789ABCDEF0ULL << 64 | 0xFEDCBA9876543210ULL)
#define NEG_LARGE_CONST_128 ((__int128)0xFEDCBA9876543210ULL << 64 | 0x123456789ABCDEF0ULL)

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)1 << 64) > 0, "128-bit shift should be positive");
_Static_assert(((__int128)HIGH_BIT_64 << 64) < 0, "Negative 128-bit constant");

/* Test function that exercises range analysis with __int128 */
static __int128 process_range(__int128 start, __int128 end) {
    __int128 sum = 0;
    for (__int128 i = start; i < end; i += (end - start) / 100) {
        /* Operations that may trigger overflow checks */
        __int128 temp = i * i;
        if (temp < 0) {
            sum -= i;
        } else {
            sum += i;
        }
        
        /* Cross-word bitwise operations */
        temp = (i & (((__int128)MAX_64 << 64) | MAX_64)) | 
               (((__int128)i) >> 32);
        sum ^= temp;
    }
    return sum;
}

/* Function with mixed-precision comparisons */
static int compare_mixed_types(__int128 a, unsigned long long b) {
    /* These comparisons may trigger the target double_int logic */
    if (a < (__int128)b) return -1;
    if (a > (__int128)b) return 1;
    
    /* Ternary with mixed types */
    __int128 result = (a & 1) ? (__int128)b : a;
    return (result == a) ? 0 : 1;
}

/* Test overflow builtins with 128-bit values */
static int test_overflow_ops(__int128 a, __int128 b) {
    __int128 sum, diff, prod;
    int overflow_add, overflow_sub, overflow_mul;
    
    overflow_add = __builtin_add_overflow(a, b, &sum);
    overflow_sub = __builtin_sub_overflow(a, b, &diff);
    overflow_mul = __builtin_mul_overflow(a, b, &prod);
    
    /* Comparisons that exercise the target code */
    if (sum < diff) return -1;
    if (prod > a && prod > b) return 1;
    if (overflow_add || overflow_sub || overflow_mul) return 2;
    return 0;
}

/* Switch statement with __int128 cases (compile-time constants) */
static int switch_128(__int128 val) {
    /* GCC may generate comparison trees for these cases */
    switch ((unsigned __int128)val) {
        case 0ULL:
            return 0;
        case ((unsigned __int128)1 << 63):
            return 1;
        case ((unsigned __int128)1 << 64):
            return 2;
        case ((unsigned __int128)MAX_64 << 64):
            return 3;
        case ((unsigned __int128)MAX_64 << 64 | MAX_64):
            return 4;
        default:
            return (val < 0) ? -1 : 5;
    }
}

/* Array operations to prevent dead code elimination */
static __int128 process_array(__int128 *arr, int size) {
    __int128 checksum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Compare array elements - may trigger target comparisons */
        if (i > 0) {
            if (arr[i] < arr[i-1]) checksum -= arr[i];
            else if (arr[i] > arr[i-1]) checksum += arr[i];
            else checksum ^= arr[i];
        }
        
        /* Bit operations crossing 64-bit boundary */
        arr[i] = (arr[i] << 1) | ((arr[i] >> 127) & 1); /* Rotate left */
        checksum += arr[i];
    }
    
    return checksum;
}

/* Use compiler builtins that may involve wide integer comparisons */
static int test_builtins(__int128 a, __int128 b) {
    /* __builtin_expect with 128-bit comparison */
    if (__builtin_expect(a < b, 0)) {
        return -1;
    }
    if (__builtin_expect(a > b, 1)) {
        return 1;
    }
    
    /* Count leading zeros on high and low parts */
    unsigned long long a_high = (unsigned long long)(a >> 64);
    unsigned long long a_low = (unsigned long long)a;
    
    int clz_high = __builtin_clzll(a_high);
    int clz_low = __builtin_clzll(a_low);
    
    return (clz_high < clz_low) ? -1 : (clz_high > clz_low) ? 1 : 0;
}

int main(void) {
    __int128 checksum = 0;
    
    /* Test 1: Compare values where high words differ */
    __int128 test1_a = ((__int128)0x1ULL << 64) | 0xFFFFFFFFULL;
    __int128 test1_b = ((__int128)0x2ULL << 64) | 0xFFFFFFFFULL;
    checksum += (test1_a < test1_b) ? -1 : 1;
    checksum += (test1_b > test1_a) ? 1 : -1;
    
    /* Test 2: Compare values where high words equal, low words differ */
    __int128 test2_a = ((__int128)0x1ULL << 64) | 0x1ULL;
    __int128 test2_b = ((__int128)0x1ULL << 64) | 0x2ULL;
    checksum += (test2_a < test2_b) ? -2 : 2;
    checksum += (test2_b > test2_a) ? 2 : -2;
    
    /* Test 3: Negative values comparison */
    __int128 test3_a = -((__int128)0x1ULL << 64);
    __int128 test3_b = -((__int128)0x2ULL << 64);
    checksum += (test3_a < test3_b) ? -3 : 3;
    checksum += (test3_b > test3_a) ? 3 : -3;
    
    /* Test 4: Boundary values */
    __int128 max_signed = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 min_signed = -max_signed - 1;
    unsigned __int128 max_unsigned = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    
    checksum += (max_signed > min_signed) ? 4 : -4;
    checksum += ((unsigned __int128)max_unsigned > (unsigned __int128)max_signed) ? 5 : -5;
    
    /* Test 5: Range analysis with loops */
    __int128 range_result = process_range(-1000, 1000);
    checksum += range_result;
    
    /* Test 6: Mixed precision operations */
    checksum += compare_mixed_types(test1_a, 0xFFFFFFFFULL);
    checksum += compare_mixed_types(test3_a, 0);
    
    /* Test 7: Overflow operations */
    checksum += test_overflow_ops(max_signed / 2, max_signed / 2);
    checksum += test_overflow_ops(min_signed, 1);
    
    /* Test 8: Switch with 128-bit cases */
    checksum += switch_128(0);
    checksum += switch_128((__int128)1 << 64);
    checksum += switch_128((__int128)MAX_64 << 64);
    
    /* Test 9: Array operations */
    __int128 arr[8] = {
        0,
        ((__int128)1 << 63),
        ((__int128)1 << 64),
        ((__int128)MAX_64 << 64),
        -((__int128)1 << 63),
        -((__int128)1 << 64),
        LARGE_CONST_128,
        NEG_LARGE_CONST_128
    };
    
    __int128 array_result = process_array(arr, 8);
    checksum += array_result;
    
    /* Test 10: Builtin functions */
    checksum += test_builtins(test1_a, test1_b);
    checksum += test_builtins(test2_a, test2_b);
    checksum += test_builtins(test3_a, test3_b);
    
    /* Force printf to handle 128-bit values (truncated to 64-bit for output) */
    unsigned long long checksum_high = (unsigned long long)(checksum >> 64);
    unsigned long long checksum_low = (unsigned long long)checksum;
    
    printf("Checksum high: 0x%016llx\n", checksum_high);
    printf("Checksum low:  0x%016llx\n", checksum_low);
    printf("Total checksum as decimal: %lld (high) * 2^64 + %lld (low)\n", 
           (long long)checksum_high, (long long)checksum_low);
    
    return 0;
}
