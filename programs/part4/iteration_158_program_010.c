/* test_double_int_cmp.c - Exercise GCC's double_int::cmp method */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to use wide integer operations */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Volatile sink to prevent optimization */
volatile int sink;

/* Function to force comparisons */
int compare_int128(int128_t a, int128_t b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

int compare_uint128(uint128_t a, uint128_t b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

/* Mixed signed/unsigned comparison */
int mixed_compare(int128_t s, uint128_t u) {
    /* This may trigger unsigned comparison for high parts */
    return (s < u) ? -1 : (s > u) ? 1 : 0;
}

/* Array indexing with wide integers */
void array_test(uint128_t idx) {
    static char arr[256] = {0};
    if (idx < 256) {
        arr[idx] = 1;
        sink = arr[idx];
    }
}

/* Loop with wide integer counter */
void loop_test(int128_t start, int128_t end) {
    for (int128_t i = start; i < end; i += 1) {
        sink = (int)i;
        if (i > end - 10) break;
    }
}

/* Switch with wide constant cases */
const char* switch_test(int128_t val) {
    /* Create case values with different high parts */
    const int128_t CASE1 = ((int128_t)0x1ULL << 64) | 0x1234ULL;
    const int128_t CASE2 = ((int128_t)0x2ULL << 64) | 0x5678ULL;
    const int128_t CASE3 = ((int128_t)0x1ULL << 64) | 0x9ABCULL;
    
    switch (val) {
        case 0: return "zero";
        case CASE1: return "case1";
        case CASE2: return "case2";
        case CASE3: return "case3";
        default: return "other";
    }
}

/* Use builtins that may trigger comparisons */
int builtin_test(int128_t a, int128_t b) {
    int128_t result;
    if (__builtin_add_overflow(a, b, &result)) {
        return -1;  /* overflow */
    }
    return (result > 0) ? 1 : 0;
}

int main() {
    /* Create wide constants with different high/low parts */
    const uint128_t UMAX = ~(uint128_t)0;
    const int128_t IMIN = ((int128_t)1ULL << 127);
    
    /* Test case 1: High part different (unsigned comparison) */
    const uint128_t big1 = ((uint128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    const uint128_t big2 = ((uint128_t)0x123456789ABCDEF1ULL << 64) | 0x0000000000000000ULL;
    
    /* big1.high < big2.high */
    sink = compare_uint128(big1, big2);  /* Should return -1 */
    
    /* big2.high > big1.high */
    sink = compare_uint128(big2, big1);  /* Should return 1 */
    
    /* Test case 2: High part equal, low part different */
    const uint128_t big3 = ((uint128_t)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL;
    const uint128_t big4 = ((uint128_t)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000001ULL;
    
    /* big3.low < big4.low */
    sink = compare_uint128(big3, big4);  /* Should return -1 */
    
    /* big4.low > big3.low */
    sink = compare_uint128(big4, big3);  /* Should return 1 */
    
    /* Test case 3: Equal values */
    const int128_t big5 = ((int128_t)0xDEADBEEFDEADBEEFULL << 64) | 0xCAFEBABECAFEBABEULL;
    const int128_t big6 = big5;
    
    sink = compare_int128(big5, big6);  /* Should return 0 */
    
    /* Test case 4: Mixed signed/unsigned */
    const int128_t sval = -1;  /* All bits set in two's complement */
    const uint128_t uval = UMAX;  /* All bits set */
    
    /* This comparison may use unsigned logic for high parts */
    sink = mixed_compare(sval, uval);
    
    /* Test case 5: Array indexing with bounds check */
    const uint128_t idx1 = 100;
    const uint128_t idx2 = ((uint128_t)1ULL << 64) | 100;  /* Too large */
    array_test(idx1);
    array_test(idx2);
    
    /* Test case 6: Loop with wide counter */
    const int128_t start = ((int128_t)0x1ULL << 64) | 0x0ULL;
    const int128_t end = ((int128_t)0x1ULL << 64) | 0x10ULL;
    loop_test(start, end);
    
    /* Test case 7: Switch with wide constants */
    const int128_t switch_val = ((int128_t)0x1ULL << 64) | 0x1234ULL;
    const char* result = switch_test(switch_val);
    sink = result[0];  /* Use result to prevent optimization */
    
    /* Test case 8: Builtin overflow checks */
    const int128_t max_val = ((int128_t)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    sink = builtin_test(max_val, 1);
    
    /* Test case 9: Arithmetic that creates new wide values */
    uint128_t x = big1 + big2;
    uint128_t y = big3 - big4;
    sink = compare_uint128(x, y);
    
    /* Test case 10: Shift operations */
    uint128_t shifted = big1 >> 64;
    sink = compare_uint128(shifted, 0x123456789ABCDEF0ULL);
    
    /* Test case 11: Multiplication creating large values */
    uint128_t prod = (uint128_t)0xFFFFFFFFFFFFFFFFULL * 0xFFFFFFFFFFFFFFFFULL;
    sink = compare_uint128(prod, UMAX);
    
    /* Test case 12: Edge cases with minimal high part */
    const uint128_t small_high1 = ((uint128_t)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    const uint128_t small_high2 = ((uint128_t)0x1ULL << 64) | 0x0ULL;
    sink = compare_uint128(small_high1, small_high2);  /* 0.high < 1.high */
    
    /* Test case 13: Negative signed values */
    const int128_t neg_big = -((int128_t)0x1ULL << 64);
    const int128_t pos_big = ((int128_t)0x1ULL << 64);
    sink = compare_int128(neg_big, pos_big);
    
    /* Test case 14: Ternary operator with wide comparison */
    uint128_t ternary_result = (big1 < big2) ? big1 : big2;
    sink = ternary_result & 0xFF;
    
    /* Test case 15: Complex expression forcing constant folding */
    const uint128_t complex = (big1 + big2) * (big3 - big4) / (uint128_t)0x1000ULL;
    const uint128_t complex2 = (big2 + big3) * (big4 - big1) / (uint128_t)0x1000ULL;
    sink = compare_uint128(complex, complex2);
    
    printf("All tests completed (sink = %d)\n", sink);
    return 0;
}
