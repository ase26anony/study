#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large __int128 constants with varying high/low word patterns */
#define HIGH_DIFF_LOW_EQUAL_A ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define HIGH_DIFF_LOW_EQUAL_B ((__int128)0x223456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL

#define HIGH_EQUAL_LOW_DIFF_A ((__int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x0000000000000001ULL
#define HIGH_EQUAL_LOW_DIFF_B ((__int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x0000000000000002ULL

#define NEGATIVE_LARGE_A ((__int128)(-1) << 120)  /* High bit set */
#define NEGATIVE_LARGE_B ((__int128)(-1) << 119)

#define MIXED_HIGH_LOW_A ((__int128)0x8000000000000000ULL << 64) | 0x7FFFFFFFFFFFFFFFULL
#define MIXED_HIGH_LOW_B ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0x8000000000000000ULL

/* Global arrays with __int128 constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    HIGH_DIFF_LOW_EQUAL_A,
    HIGH_DIFF_LOW_EQUAL_B,
    HIGH_EQUAL_LOW_DIFF_A,
    HIGH_EQUAL_LOW_DIFF_B,
    NEGATIVE_LARGE_A,
    NEGATIVE_LARGE_B,
    MIXED_HIGH_LOW_A,
    MIXED_HIGH_LOW_B,
    0
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
               "High word difference comparison");
_Static_assert(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B,
               "Low word difference comparison");
_Static_assert(NEGATIVE_LARGE_A < NEGATIVE_LARGE_B,
               "Negative large comparison");

/* Dead code with comparisons that compiler may evaluate during early passes */
static __int128 dead_code_comparisons(__int128 x) {
    if (0) {  /* Dead code, but constants still processed */
        if (x < HIGH_DIFF_LOW_EQUAL_A) return 1;
        if (x > HIGH_DIFF_LOW_EQUAL_B) return 2;
        if (x == HIGH_EQUAL_LOW_DIFF_A) return 3;
        if (x != HIGH_EQUAL_LOW_DIFF_B) return 4;
    }
    return 0;
}

/* Function using __int128 comparisons for range analysis */
__int128 range_analysis_function(__int128 a, __int128 b) {
    /* Comparisons that may trigger VRP analysis */
    if (a < HIGH_DIFF_LOW_EQUAL_A && b > HIGH_DIFF_LOW_EQUAL_B) {
        return a + b;
    }
    
    if (a >= HIGH_EQUAL_LOW_DIFF_A && a <= HIGH_EQUAL_LOW_DIFF_B) {
        return a - b;
    }
    
    /* Mixed-type comparison */
    unsigned long long ull = 0xFFFFFFFFFFFFFFFFULL;
    if (a > (__int128)ull) {
        return a >> 2;
    }
    
    return a * b;
}

/* Function with overflow checks using __int128 */
__int128 overflow_checks(__int128 x, __int128 y) {
    __int128 result;
    int overflow;
    
    /* Addition overflow check */
    overflow = __builtin_add_overflow(x, y, &result);
    if (overflow) {
        return x >> 1;
    }
    
    /* Multiplication overflow check */
    overflow = __builtin_mul_overflow(result, y, &result);
    if (overflow) {
        return y >> 1;
    }
    
    return result;
}

/* Function with bitwise operations crossing word boundaries */
__int128 cross_word_boundary_ops(__int128 x) {
    /* Shift moving bits from low to high word */
    __int128 shifted = x << 72;
    
    /* Mask targeting high word */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 high_bits = shifted & mask_high;
    
    /* Arithmetic right shift on negative values */
    if (x < 0) {
        return x >> 96;  /* Shifts high word bits */
    }
    
    return high_bits | (x & ~mask_high);
}

/* Loop with __int128 induction variable */
__int128 loop_with_wide_induction(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop bound comparisons with different high/low words */
    for (__int128 i = start; i < end; i += ((__int128)1 << 60)) {
        sum += i;
        
        /* Nested comparison inside loop */
        if (i > MIXED_HIGH_LOW_A && i < MIXED_HIGH_LOW_B) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Ternary operator with mixed types */
__int128 ternary_mixed_types(__int128 a, unsigned long long b) {
    /* Ternary forces conversion and comparison */
    return (a > ((__int128)b << 64)) ? a : ((__int128)b << 64);
}

/* Structure with __int128 member for initialization */
struct wide_int_struct {
    __int128 wide_field;
    int normal_field;
};

/* Global struct initialization with __int128 constant */
static struct wide_int_struct global_struct = {
    .wide_field = HIGH_DIFF_LOW_EQUAL_A,
    .normal_field = 42
};

/* Main test function */
int main() {
    __int128 result = 0;
    
    /* Test 1: Range analysis with comparisons */
    result += range_analysis_function(
        HIGH_DIFF_LOW_EQUAL_A + 1,
        HIGH_DIFF_LOW_EQUAL_B - 1
    );
    
    /* Test 2: Overflow checks */
    result += overflow_checks(
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64),
        0x1000000000000000ULL
    );
    
    /* Test 3: Cross-word operations */
    result += cross_word_boundary_ops(NEGATIVE_LARGE_A);
    result += cross_word_boundary_ops(MIXED_HIGH_LOW_B);
    
    /* Test 4: Loop with wide induction */
    result += loop_with_wide_induction(
        HIGH_EQUAL_LOW_DIFF_A,
        HIGH_EQUAL_LOW_DIFF_B + ((__int128)1 << 62)
    );
    
    /* Test 5: Ternary with mixed types */
    result += ternary_mixed_types(
        MIXED_HIGH_LOW_A,
        0xFFFFFFFFFFFFFFFFULL
    );
    
    /* Process global array (ensures constants are used) */
    for (int i = 0; i < (int)(sizeof(global_consts)/sizeof(global_consts[0])) - 1; i++) {
        result += global_consts[i] - global_consts[i + 1];
    }
    
    /* Add struct field */
    result += global_struct.wide_field;
    
    /* Dead code call (should be optimized out but constants processed) */
    result += dead_code_comparisons(result);
    
    /* Print lower 64 bits of result for verification */
    printf("Result (lower 64 bits): %llx\n", (unsigned long long)(result & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
