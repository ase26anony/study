#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_DIFF_LOW_EQUAL_A (((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111111ULL)
#define HIGH_DIFF_LOW_EQUAL_B (((__int128)0x123456789ABCDEF1ULL << 64) | 0x1111111111111111ULL)

#define HIGH_EQUAL_LOW_DIFF_A (((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL)
#define HIGH_EQUAL_LOW_DIFF_B (((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000001ULL)

#define NEGATIVE_LARGE_A (((__int128)(-1LL) << 64) | 0x8000000000000000ULL)
#define NEGATIVE_LARGE_B (((__int128)(-1LL) << 64) | 0x7FFFFFFFFFFFFFFFULL)

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    HIGH_DIFF_LOW_EQUAL_A,
    HIGH_DIFF_LOW_EQUAL_B,
    HIGH_EQUAL_LOW_DIFF_A,
    HIGH_EQUAL_LOW_DIFF_B,
    NEGATIVE_LARGE_A,
    NEGATIVE_LARGE_B,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Only low word set */
    ((__int128)0x1ULL << 64) | 0x0ULL,                 /* Only high word set */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
               "High word difference comparison");
_Static_assert(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B,
               "Low word difference comparison");
_Static_assert(NEGATIVE_LARGE_A < NEGATIVE_LARGE_B,
               "Negative comparison");

/* Dead code with comparisons that GCC may evaluate during optimization */
static __int128 dead_code_comparisons(__int128 x) {
    if (0) {  /* Dead code, but compiler may still analyze constants */
        /* Comparisons that should trigger double_int::cmp logic */
        if (x < HIGH_DIFF_LOW_EQUAL_A) return x + 1;
        if (x > HIGH_DIFF_LOW_EQUAL_B) return x - 1;
        if (x <= HIGH_EQUAL_LOW_DIFF_A) return x << 2;
        if (x >= HIGH_EQUAL_LOW_DIFF_B) return x >> 2;
    }
    return x;
}

/* Function with range analysis on 128-bit values */
__int128 range_analysis_test(__int128 a, __int128 b) {
    /* Comparisons that cross 64-bit boundaries */
    if (a < ((__int128)0x8000000000000000ULL << 64)) {
        /* a has high word < 0x8000000000000000 */
        if (b > ((__int128)0x7FFFFFFFFFFFFFFFULL << 64)) {
            /* b has high word > 0x7FFFFFFFFFFFFFFF */
            return a - b;  /* Will be negative */
        }
    }
    
    /* Compare with mixed-type constant */
    if (a < 0xFFFFFFFFFFFFFFFFULL) {  /* Compare 128-bit with 64-bit */
        return a | 0x1;
    }
    
    return a ^ b;
}

/* Loop with 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that may cause VRP to analyze 128-bit ranges */
    for (__int128 i = start; i < end; i = i + (((__int128)1ULL << 60) | 1ULL)) {
        /* Shift operations that cross word boundaries */
        __int128 shifted = i << 65;  /* Shift into high word */
        sum += shifted & (((__int128)0xFFFFFFFFULL << 96) | 0xFFFFFFFFULL);
        
        /* Arithmetic shift on negative values */
        if (i < 0) {
            sum += i >> 3;  /* Arithmetic right shift */
        }
    }
    
    return sum;
}

/* Overflow checking with 128-bit values */
int overflow_checks(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* Addition overflow check */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) return -1;
    
    /* Multiplication overflow check */
    overflow = __builtin_mul_overflow(a, b, &result);
    if (overflow) return -2;
    
    /* Compare result with large constant */
    if (result > HIGH_DIFF_LOW_EQUAL_A) {
        return 1;
    }
    
    return 0;
}

/* Bitwise operations targeting specific words */
__int128 bitwise_word_operations(__int128 x) {
    /* Mask targeting high word */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    
    /* Mask targeting low word */
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Clear high word */
    __int128 low_only = x & mask_low;
    
    /* Isolate high word */
    __int128 high_only = x & mask_high;
    
    /* Swap words */
    return (low_only << 64) | (high_only >> 64);
}

/* Ternary operator with type conversions */
__int128 ternary_with_conversions(__int128 a, unsigned long long b) {
    /* Compare 128-bit with 64-bit, promote 64-bit */
    return (a > b) ? a : ((__int128)b << 32);
}

/* Switch-like logic using comparisons */
__int128 switch_like_logic(__int128 x) {
    if (x < ((__int128)0x1000000000000000ULL << 64)) {
        return x | 0x1;
    } else if (x < ((__int128)0x2000000000000000ULL << 64)) {
        return x & ~((__int128)0x1ULL);
    } else if (x < ((__int128)0x3000000000000000ULL << 64)) {
        return x ^ 0x1;
    }
    return x;
}

/* Main test function */
int main() {
    __int128 sum = 0;
    
    /* Process global constants */
    for (size_t i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        sum += global_consts[i];
    }
    
    /* Test range analysis */
    __int128 test_val = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL;
    sum += range_analysis_test(test_val, ~test_val);
    
    /* Test loop with 128-bit induction variable */
    __int128 loop_start = ((__int128)0x1000ULL << 64) | 0x0ULL;
    __int128 loop_end = ((__int128)0x1000ULL << 64) | 0x1000ULL;
    sum += loop_with_128bit_iv(loop_start, loop_end);
    
    /* Test overflow checks */
    sum += overflow_checks(test_val, 0x1000);
    
    /* Test bitwise operations */
    sum += bitwise_word_operations(test_val);
    
    /* Test ternary conversions */
    sum += ternary_with_conversions(test_val, 0xFFFFFFFFFFFFFFFFULL);
    
    /* Test switch-like logic */
    sum += switch_like_logic(test_val);
    
    /* Dead code function call (should be optimized away but constants analyzed) */
    sum += dead_code_comparisons(test_val);
    
    /* Print checksum (simplified for demonstration) */
    unsigned long long low = (unsigned long long)(sum & 0xFFFFFFFFFFFFFFFFULL);
    unsigned long long high = (unsigned long long)((sum >> 64) & 0xFFFFFFFFFFFFFFFFULL);
    
    printf("Checksum: high=0x%016llx low=0x%016llx\n", high, low);
    printf("Result: %s\n", (sum != 0) ? "Non-zero" : "Zero");
    
    return 0;
}
