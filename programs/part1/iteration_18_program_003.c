/* double_int_test.c - Test program to trigger GCC's double_int::cmp logic */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_DIFF_LOW_EQUAL_A ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define HIGH_DIFF_LOW_EQUAL_B ((__int128)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL
#define HIGH_EQUAL_LOW_DIFF_A ((__int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x0000000000000001ULL
#define HIGH_EQUAL_LOW_DIFF_B ((__int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x0000000000000002ULL
#define NEGATIVE_LARGE_A      ((__int128)(-1) << 120)  /* High bit set */
#define NEGATIVE_LARGE_B      ((__int128)(-2) << 120)

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    HIGH_DIFF_LOW_EQUAL_A,
    HIGH_DIFF_LOW_EQUAL_B,
    HIGH_EQUAL_LOW_DIFF_A,
    HIGH_EQUAL_LOW_DIFF_B,
    NEGATIVE_LARGE_A,
    NEGATIVE_LARGE_B,
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL, /* -1 */
    ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL, /* MAX/2 */
};

/* Static assertions to force compile-time comparisons */
_Static_assert(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
               "High word difference comparison");
_Static_assert(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B,
               "Low word difference comparison");
_Static_assert(NEGATIVE_LARGE_A > NEGATIVE_LARGE_B,
               "Negative comparison");

/* Function 1: Range analysis with wide integer comparisons */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that may trigger double_int::cmp in VRP */
    if (x < HIGH_DIFF_LOW_EQUAL_A) {
        if (y > HIGH_EQUAL_LOW_DIFF_B) {
            return x + y;
        }
    }
    
    /* Cross-word boundary comparison */
    __int128 threshold = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    if (x > threshold && y < threshold) {
        return x - y;
    }
    
    return x * y;
}

/* Function 2: Loop with 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that may cause VRP to analyze 128-bit ranges */
    for (__int128 i = start; i < end; i += ((__int128)1 << 60)) {
        sum += i;
        
        /* Conditional with comparison against mixed constants */
        if (i < HIGH_DIFF_LOW_EQUAL_A) {
            sum += 1;
        } else if (i > HIGH_EQUAL_LOW_DIFF_B) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Function 3: Bitwise operations crossing word boundaries */
__int128 bitwise_cross_boundary(__int128 a, __int128 b) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low  = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 shifted = a << 65;  /* Moves bits from low to high word */
    __int128 masked = b & mask_high;
    
    /* Comparison after shift - high word will differ */
    if (shifted > masked) {
        return shifted | (b & mask_low);
    }
    
    /* Arithmetic right shift on negative value */
    __int128 neg_shifted = a >> 72;
    if (neg_shifted < b) {
        return neg_shifted ^ mask_high;
    }
    
    return a & b;
}

/* Function 4: Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(unsigned long long ull_val, __int128 s128_val) {
    /* Implicit conversion and comparison */
    if (s128_val < ull_val) {
        return s128_val + ull_val;
    }
    
    /* Ternary with mixed types */
    __int128 result = (ull_val > 1000) ? 
                     HIGH_DIFF_LOW_EQUAL_A : 
                     (__int128)ull_val;
    
    /* Comparison with overflow builtins */
    __int128 overflow_test;
    if (__builtin_add_overflow(s128_val, result, &overflow_test)) {
        return s128_val;
    }
    
    return result;
}

/* Function 5: Dead code with constant comparisons (still processed by early opts) */
void dead_code_with_comparisons(void) {
    if (0) {  /* Dead code, but constants still processed */
        /* These comparisons should still be evaluated during early optimization */
        if (HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) {
            volatile __int128 dead_var = HIGH_EQUAL_LOW_DIFF_A;
        }
        
        /* More complex dead comparisons */
        __int128 dead_sum = 0;
        for (__int128 i = NEGATIVE_LARGE_A; i < NEGATIVE_LARGE_B; i++) {
            dead_sum += i;
        }
    }
}

/* Function 6: Array operations with 128-bit values */
__int128 process_global_array(void) {
    __int128 sum = 0;
    
    /* Process global array - constants may be compared during optimization */
    for (size_t i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        /* Conditional based on array value comparisons */
        if (i > 0 && global_consts[i] > global_consts[i-1]) {
            sum += global_consts[i];
        } else {
            sum -= global_consts[i];
        }
    }
    
    return sum;
}

/* Function 7: Switch-like logic with large constants */
int switch_like_logic(__int128 val) {
    /* GCC may convert this to decision tree with comparisons */
    if (val == HIGH_DIFF_LOW_EQUAL_A) return 1;
    if (val == HIGH_DIFF_LOW_EQUAL_B) return 2;
    if (val == HIGH_EQUAL_LOW_DIFF_A) return 3;
    if (val == HIGH_EQUAL_LOW_DIFF_B) return 4;
    if (val == NEGATIVE_LARGE_A) return 5;
    if (val == NEGATIVE_LARGE_B) return 6;
    
    return 0;
}

/* Main function that exercises all test cases */
int main(void) {
    /* Test values designed to trigger various comparison paths */
    __int128 test_val1 = HIGH_DIFF_LOW_EQUAL_A;
    __int128 test_val2 = HIGH_EQUAL_LOW_DIFF_B;
    unsigned long long ull_test = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Call test functions */
    __int128 result1 = range_analysis_test(test_val1, test_val2);
    __int128 result2 = loop_with_128bit_iv(NEGATIVE_LARGE_A, NEGATIVE_LARGE_B);
    __int128 result3 = bitwise_cross_boundary(test_val1, test_val2);
    __int128 result4 = mixed_type_comparisons(ull_test, test_val1);
    __int128 result5 = process_global_array();
    
    /* Ensure dead code function is called (though it does nothing at runtime) */
    dead_code_with_comparisons();
    
    /* Simple checksum to verify program runs */
    __int128 checksum = result1 + result2 + result3 + result4 + result5;
    
    /* Print partial results (since printf doesn't support __int128 directly) */
    unsigned long long checksum_high = (unsigned long long)(checksum >> 64);
    unsigned long long checksum_low = (unsigned long long)checksum;
    
    printf("Test completed. Checksum: 0x%016llx%016llx\n", 
           checksum_high, checksum_low);
    
    /* Additional compile-time checks using __builtin_constant_p */
    if (__builtin_constant_p(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B)) {
        printf("Constant comparison evaluated at compile time\n");
    }
    
    return 0;
}

/* Additional global with complex initializer requiring constant folding */
static const __int128 complex_global = 
    (HIGH_DIFF_LOW_EQUAL_A > HIGH_EQUAL_LOW_DIFF_B) ? 
    HIGH_DIFF_LOW_EQUAL_A : HIGH_EQUAL_LOW_DIFF_B;

/* Struct with 128-bit fields to test structure initialization */
struct wide_int_struct {
    __int128 field1;
    __int128 field2;
};

static const struct wide_int_struct struct_const = {
    .field1 = HIGH_DIFF_LOW_EQUAL_A,
    .field2 = HIGH_EQUAL_LOW_DIFF_B,
};
