/* double_int_cmp_test.c
 * Designed to trigger GCC's internal double_int::cmp comparisons
 * during constant folding, range analysis, and optimization passes.
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_DIFF_LOW_EQUAL_A (((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111111ULL)
#define HIGH_DIFF_LOW_EQUAL_B (((__int128)0x123456789ABCDE00ULL << 64) | 0x1111111111111111ULL)

#define HIGH_EQUAL_LOW_DIFF_A (((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL)
#define HIGH_EQUAL_LOW_DIFF_B (((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL)

#define MIXED_HIGH_LOW_A (((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL)
#define MIXED_HIGH_LOW_B (((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL)

#define NEGATIVE_LARGE (((__int128)(int64_t)-1 << 64) | 0xFFFFFFFFFFFFFFFFULL)
#define POSITIVE_LARGE (((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL)

/* Global arrays with __int128 constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    HIGH_DIFF_LOW_EQUAL_A,
    HIGH_DIFF_LOW_EQUAL_B,
    HIGH_EQUAL_LOW_DIFF_A,
    HIGH_EQUAL_LOW_DIFF_B,
    MIXED_HIGH_LOW_A,
    MIXED_HIGH_LOW_B,
    NEGATIVE_LARGE,
    POSITIVE_LARGE,
    0,
    ((__int128)1 << 127) - 1,
    (__int128)-1 << 127
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(HIGH_DIFF_LOW_EQUAL_A > HIGH_DIFF_LOW_EQUAL_B, 
               "High word difference comparison should pass");
_Static_assert(HIGH_EQUAL_LOW_DIFF_A > HIGH_EQUAL_LOW_DIFF_B,
               "Low word difference comparison should pass");
_Static_assert(MIXED_HIGH_LOW_A > MIXED_HIGH_LOW_B,
               "Mixed high/low comparison should pass");

/* Function to test range analysis with __int128 comparisons */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x > HIGH_DIFF_LOW_EQUAL_A) {
        return x + y;
    }
    if (y < HIGH_EQUAL_LOW_DIFF_B) {
        return x - y;
    }
    if (x >= MIXED_HIGH_LOW_A && y <= MIXED_HIGH_LOW_B) {
        return x * y;
    }
    
    /* Ternary with mixed types forcing conversions */
    return (x > 0) ? ((__int128)0xFFFFFFFFULL << 64) : (__int128)y;
}

/* Function with __int128 loop induction variable */
__int128 loop_with_wide_bounds(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with 128-bit induction variable and bounds */
    for (__int128 i = start; i < end; i += ((__int128)1 << 60)) {
        /* Cross-word boundary shift */
        __int128 shifted = i << 65;  /* Moves bits from low to high word */
        
        /* Bitwise operations targeting specific words */
        __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
        __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
        
        __int128 high_part = shifted & mask_high;
        __int128 low_part = shifted & mask_low;
        
        sum += (high_part >> 64) + low_part;
    }
    
    return sum;
}

/* Function using overflow builtins with __int128 */
int overflow_checks(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* These may trigger double_int comparisons internally */
    overflow = __builtin_add_overflow(a, b, &result);
    if (!overflow) {
        overflow = __builtin_mul_overflow(a, b, &result);
    }
    
    /* Comparison with narrower type */
    if (result > 0xFFFFFFFFULL) {
        return 1;
    }
    
    return overflow;
}

/* Function with dead code containing __int128 comparisons */
__int128 dead_code_paths(__int128 x) {
    __int128 result = x;
    
    /* Dead code that compiler may still analyze */
    if (0) {  /* Always false, but compiler may analyze constants */
        /* Comparisons between large constants */
        if (HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) {
            result = HIGH_DIFF_LOW_EQUAL_A;
        }
        
        /* Right shift on negative value (arithmetic shift) */
        __int128 neg_shifted = NEGATIVE_LARGE >> 32;
        if (neg_shifted > POSITIVE_LARGE) {
            result = neg_shifted;
        }
    }
    
    /* Switch-like logic using comparisons */
    if (x < 0) {
        return -x;
    } else if (x > ((__int128)1 << 120)) {
        return x >> 4;
    } else if (x >= 0xFFFFFFFFULL && x <= ((__int128)0xFFFFFFFFULL << 64)) {
        return x | 0xFF;
    }
    
    return result;
}

/* Function mixing signed and unsigned comparisons */
int mixed_type_comparisons(__int128 a, unsigned long long b) {
    /* Implicit conversion and comparison */
    if (a == b) {
        return 1;
    }
    
    /* Explicit comparison with promotion */
    if ((unsigned __int128)a > (unsigned __int128)b) {
        return 2;
    }
    
    /* Comparison where high word matters */
    if (a > ((__int128)b << 64)) {
        return 3;
    }
    
    return 0;
}

/* Main test function */
int main() {
    __int128 sum = 0;
    
    /* Process global constants */
    for (int i = 0; i < (int)(sizeof(global_consts)/sizeof(global_consts[0])); i++) {
        sum += global_consts[i];
    }
    
    /* Test range analysis */
    __int128 test_val = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    sum += range_analysis_test(test_val, test_val >> 1);
    
    /* Test loop with wide bounds */
    sum += loop_with_wide_bounds(0, ((__int128)1 << 70));
    
    /* Test overflow checks */
    sum += overflow_checks(test_val, test_val);
    
    /* Test dead code paths */
    sum += dead_code_paths(test_val);
    
    /* Test mixed type comparisons */
    sum += mixed_type_comparisons(test_val, 0xFFFFFFFFFFFFFFFFULL);
    
    /* Print a verifiable result (using 64-bit chunks for portability) */
    unsigned long long high = (unsigned long long)(sum >> 64);
    unsigned long long low = (unsigned long long)sum;
    
    printf("Result checksum: high=0x%016llx low=0x%016llx\n", high, low);
    printf("Test completed successfully.\n");
    
    return 0;
}
