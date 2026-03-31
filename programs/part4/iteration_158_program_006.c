/* test_double_int_cmp.c - Exercise GCC's double_int::cmp method */
#include <stdio.h>
#include <stdint.h>

/* Force compiler to use wide integer operations */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Volatile sink to prevent optimization */
volatile int sink;

/* Function that returns comparison result */
int compare_int128(int128_t a, int128_t b) {
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

int compare_uint128(uint128_t a, uint128_t b) {
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

/* Function with mixed signed/unsigned comparison */
int mixed_compare(int128_t s, uint128_t u) {
    /* This may trigger unsigned comparison for high parts */
    return (s < (int128_t)u) ? -1 : (s > (int128_t)u) ? 1 : 0;
}

/* Array access with wide indices */
void array_test(uint128_t idx) {
    static char arr[256];
    if (idx < 256) {
        arr[idx] = (char)idx;
        sink = arr[idx];
    }
}

/* Loop with wide counter */
void wide_loop(uint128_t start, uint128_t end) {
    for (uint128_t i = start; i < end; i += 1) {
        sink = (int)i;
        if (i > start + 10) break; /* Prevent infinite loops */
    }
}

/* Test builtin overflow operations */
void overflow_test(int128_t a, int128_t b) {
    int128_t result;
    if (__builtin_add_overflow(a, b, &result)) {
        sink = -1; /* overflow path */
    } else {
        sink = (int)result;
    }
}

/* Switch with wide constants */
const char* switch_wide(int128_t val) {
    const uint128_t CASE1 = ((uint128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    const uint128_t CASE2 = ((uint128_t)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL;
    const uint128_t CASE3 = ((uint128_t)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
    
    if (val == (int128_t)CASE1) return "CASE1";
    if (val == (int128_t)CASE2) return "CASE2";
    if (val == (int128_t)CASE3) return "CASE3";
    return "DEFAULT";
}

int main(void) {
    /* Large constants that exercise high parts */
    const uint128_t BIG1 = ((uint128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    const uint128_t BIG2 = ((uint128_t)0x123456789ABCDEF0ULL << 64) | 0x0123456789ABCDEFULL;
    const uint128_t BIG3 = ((uint128_t)0xFEDCBA9876543210ULL << 64) | 0x123456789ABCDEF0ULL;
    const uint128_t BIG4 = ((uint128_t)0x0000000000000000ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    const uint128_t BIG5 = ((uint128_t)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL;
    
    /* Signed versions with sign bit set */
    const int128_t SBIG1 = (int128_t)BIG1;
    const int128_t SBIG2 = (int128_t)BIG2;
    const int128_t SBIG3 = -((int128_t)1 << 120); /* Negative large value */
    
    /* Test 1: High part less (BIG4 < BIG1) */
    sink = compare_uint128(BIG4, BIG1);
    
    /* Test 2: High part greater (BIG3 > BIG1) */
    sink = compare_uint128(BIG3, BIG1);
    
    /* Test 3: High part equal, low part less (BIG2 < BIG1) */
    sink = compare_uint128(BIG2, BIG1);
    
    /* Test 4: High part equal, low part greater (BIG1 > BIG2) */
    sink = compare_uint128(BIG1, BIG2);
    
    /* Test 5: Equality (BIG1 == BIG1) */
    sink = compare_uint128(BIG1, BIG1);
    
    /* Test 6: Mixed signed/unsigned comparisons */
    sink = mixed_compare(SBIG1, BIG1);
    sink = mixed_compare(SBIG3, BIG4); /* Negative vs positive */
    
    /* Test 7: Control flow with wide comparisons */
    if (BIG1 < BIG3) {
        sink = 100;
    } else if (BIG1 > BIG3) {
        sink = 200;
    } else {
        sink = 300;
    }
    
    /* Test 8: Ternary operator with wide comparison */
    sink = (BIG2 < BIG1) ? 1 : 0;
    sink = (BIG5 > BIG4) ? 1 : 0;
    
    /* Test 9: Array indexing with bounds check */
    array_test(BIG4); /* idx = 0xFFFFFFFFFFFFFFFF (needs bounds check) */
    array_test((uint128_t)100);
    
    /* Test 10: Wide loop */
    wide_loop(BIG4, BIG4 + 20);
    
    /* Test 11: Arithmetic operations that may fold */
    uint128_t sum = BIG1 + BIG2;
    uint128_t diff = BIG3 - BIG1;
    uint128_t prod = BIG4 * 2;
    
    sink = compare_uint128(sum, diff);
    sink = compare_uint128(prod, BIG4);
    
    /* Test 12: Builtin overflow checks */
    overflow_test(SBIG1, SBIG2);
    overflow_test((int128_t)BIG4, (int128_t)BIG4);
    
    /* Test 13: Shift operations creating wide values */
    uint128_t shifted = ((uint128_t)1 << 127) | ((uint128_t)1 << 64);
    sink = compare_uint128(shifted, BIG5);
    
    /* Test 14: Switch-like comparisons */
    const char* result = switch_wide(SBIG1);
    sink = result[0]; /* Use result to prevent elimination */
    
    /* Test 15: Complex expression with multiple comparisons */
    int complex_result = (BIG1 < BIG2) ? -1 : 
                        (BIG1 > BIG2) ? 1 :
                        (BIG3 < BIG4) ? -2 :
                        (BIG3 > BIG4) ? 2 : 0;
    sink = complex_result;
    
    /* Test 16: Comparisons in loop conditions */
    for (uint128_t i = BIG4; i < BIG4 + 5; i++) {
        sink = (int)i;
        if (i > BIG4 + 2) break;
    }
    
    /* Test 17: Bitwise operations followed by comparison */
    uint128_t masked1 = BIG1 & 0xFFFFFFFFFFFFFFFFULL;
    uint128_t masked2 = BIG2 & 0xFFFFFFFFFFFFFFFFULL;
    sink = compare_uint128(masked1, masked2);
    
    /* Test 18: Multiplication that may overflow into high part */
    uint128_t mul_result = BIG4 * BIG4;
    sink = compare_uint128(mul_result, BIG4);
    
    /* Test 19: Division/modulus with wide operands */
    if (BIG5 != 0) {
        uint128_t div_result = BIG3 / BIG5;
        sink = compare_uint128(div_result, BIG1);
    }
    
    /* Test 20: Final aggregate check */
    int final_check = (compare_uint128(BIG1, BIG2) == -1) &&
                     (compare_uint128(BIG3, BIG1) == 1) &&
                     (compare_uint128(BIG1, BIG1) == 0);
    
    printf("All tests completed. Final check: %d\n", final_check);
    return final_check ? 0 : 1;
}
