/* double_int_cmp_test.c
 * Designed to trigger GCC's double_int::cmp during compilation
 * by forcing comparisons of 128-bit integers in constant folding,
 * range analysis, and various optimization passes.
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_WORD_DIFFERS_A ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define HIGH_WORD_DIFFERS_B ((__int128)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL
#define HIGH_WORD_EQUAL_LOW_DIFFERS_A ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL
#define HIGH_WORD_EQUAL_LOW_DIFFERS_B ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000001ULL
#define NEGATIVE_LARGE ((__int128)(-1) << 120)  /* High bit set */
#define POSITIVE_LARGE ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL

/* Global arrays with 128-bit constants - forces constant folding */
static const __int128 global_consts[] = {
    HIGH_WORD_DIFFERS_A,
    HIGH_WORD_DIFFERS_B,
    HIGH_WORD_EQUAL_LOW_DIFFERS_A,
    HIGH_WORD_EQUAL_LOW_DIFFERS_B,
    NEGATIVE_LARGE,
    POSITIVE_LARGE,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Only low word set */
    ((__int128)0x1ULL << 64) | 0x0ULL,                 /* Only high word set */
};

/* Static assertions to force compile-time comparisons */
_Static_assert(HIGH_WORD_DIFFERS_A < HIGH_WORD_DIFFERS_B, 
               "High word comparison should trigger");
_Static_assert(HIGH_WORD_EQUAL_LOW_DIFFERS_A < HIGH_WORD_EQUAL_LOW_DIFFERS_B,
               "Low word comparison when high words equal");

/* Function 1: Range analysis with comparisons in both high and low words */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that may trigger double_int::cmp in VRP */
    if (x < HIGH_WORD_DIFFERS_A) {
        if (y > HIGH_WORD_EQUAL_LOW_DIFFERS_B) {
            return x + y;
        }
    }
    
    /* Ternary with mixed comparisons */
    __int128 result = (x < y) ? x : y;
    
    /* Nested comparisons */
    if ((x > NEGATIVE_LARGE) && (x < POSITIVE_LARGE)) {
        result += global_consts[0];
    }
    
    return result;
}

/* Function 2: Bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 a, __int128 b) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 shifted = a << 65;  /* Moves bits from low to high word */
    __int128 masked = b & mask_high;
    
    /* Comparison after bitwise ops */
    if (shifted > masked) {
        return shifted | mask_low;
    }
    
    return masked & ~mask_high;
}

/* Function 3: Arithmetic with overflow checking */
__int128 arithmetic_with_overflow(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow;
    
    /* Overflow checks may use double_int comparisons internally */
    overflow = __builtin_add_overflow(a, b, &sum);
    if (overflow) {
        /* Force comparison in dead code path */
        if (a > HIGH_WORD_DIFFERS_B && b < NEGATIVE_LARGE) {
            return 0;  /* Dead code, but constants compared */
        }
        return a;
    }
    
    overflow = __builtin_mul_overflow(a, b, &product);
    if (overflow) {
        /* Another dead code comparison */
        if (product < HIGH_WORD_EQUAL_LOW_DIFFERS_A) {
            return b;
        }
    }
    
    return sum + product;
}

/* Function 4: Loop with 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop bound comparisons require double_int::cmp */
    for (__int128 i = start; i < end; i += ((__int128)1 << 62)) {
        /* Mix in comparisons inside loop */
        if (i > HIGH_WORD_DIFFERS_A) {
            sum += global_consts[1];
        } else if (i < HIGH_WORD_EQUAL_LOW_DIFFERS_B) {
            sum += global_consts[2];
        }
        
        /* Prevent infinite loops with reasonable bounds */
        if (i > (end - ((__int128)1 << 70))) {
            break;
        }
    }
    
    return sum;
}

/* Function 5: Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(__int128 a, unsigned long long b) {
    /* Implicit conversion and comparison */
    if (a < b) {
        return a + b;
    }
    
    /* Ternary with mixed types */
    __int128 result = (b > 1000) ? (__int128)b * 2 : a;
    
    /* Comparison after conversion */
    __int128 converted = b;
    if (converted > HIGH_WORD_EQUAL_LOW_DIFFERS_A) {
        result += converted;
    }
    
    return result;
}

/* Function 6: Dead code with constant comparisons */
void dead_code_with_comparisons(void) {
    /* Dead code that still gets constant folded */
    if (0) {
        /* These comparisons should still be evaluated during compilation */
        volatile __int128 dead1 = HIGH_WORD_DIFFERS_A;
        volatile __int128 dead2 = HIGH_WORD_DIFFERS_B;
        
        if (dead1 < dead2) {
            /* Empty */
        }
        
        /* More complex dead comparisons */
        __int128 dead3 = dead1 + dead2;
        if (dead3 > NEGATIVE_LARGE && dead3 < POSITIVE_LARGE) {
            /* Empty */
        }
    }
    
    /* Another dead code path */
    if (__builtin_constant_p(0)) {
        /* Force constant evaluation */
        _Static_assert(HIGH_WORD_DIFFERS_A != HIGH_WORD_DIFFERS_B,
                      "Dead constant comparison");
    }
}

/* Function 7: Switch with large constants (simulated with if-else chain) */
int switch_like_comparisons(__int128 val) {
    /* GCC may convert this to a decision tree with comparisons */
    if (val == HIGH_WORD_DIFFERS_A) return 1;
    if (val == HIGH_WORD_DIFFERS_B) return 2;
    if (val == HIGH_WORD_EQUAL_LOW_DIFFERS_A) return 3;
    if (val == HIGH_WORD_EQUAL_LOW_DIFFERS_B) return 4;
    if (val == NEGATIVE_LARGE) return 5;
    if (val == POSITIVE_LARGE) return 6;
    return 0;
}

/* Main function that exercises all test cases */
int main(void) {
    /* Initialize with values that span different comparison cases */
    __int128 test_a = HIGH_WORD_DIFFERS_A;
    __int128 test_b = HIGH_WORD_DIFFERS_B;
    __int128 test_c = HIGH_WORD_EQUAL_LOW_DIFFERS_A;
    __int128 test_d = HIGH_WORD_EQUAL_LOW_DIFFERS_B;
    
    /* Test 1: Range analysis */
    __int128 result1 = range_analysis_test(test_a, test_b);
    
    /* Test 2: Bitwise operations */
    __int128 result2 = bitwise_operations(test_c, test_d);
    
    /* Test 3: Arithmetic with overflow */
    __int128 result3 = arithmetic_with_overflow(test_a, test_c);
    
    /* Test 4: Loop with 128-bit IV */
    __int128 result4 = loop_with_128bit_iv(NEGATIVE_LARGE, POSITIVE_LARGE);
    
    /* Test 5: Mixed type comparisons */
    __int128 result5 = mixed_type_comparisons(test_b, 0xFFFFFFFFULL);
    
    /* Test 6: Dead code (still compiled) */
    dead_code_with_comparisons();
    
    /* Test 7: Switch-like comparisons */
    int result7 = switch_like_comparisons(test_a);
    
    /* Compute a simple checksum from global constants */
    __int128 checksum = 0;
    for (size_t i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        checksum += global_consts[i];
    }
    
    /* Add all results */
    checksum += result1 + result2 + result3 + result4 + result5 + result7;
    
    /* Print lower 64 bits of checksum for verification */
    printf("Checksum (lower 64 bits): %llu\n", (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
