/* double_int_cmp_test.c
 * Test program to trigger GCC's double_int::cmp logic during compilation
 * Specifically targets lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Global constants with explicit high/low word differences */
static const __int128 GLOBAL_CONST_A = 
    ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
static const __int128 GLOBAL_CONST_B = 
    ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543211ULL;  /* Low word differs */
static const __int128 GLOBAL_CONST_C = 
    ((__int128)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL;  /* High word differs */
static const __int128 GLOBAL_CONST_D = 
    ((__int128)0x0000000000000000ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;  /* Only low word set */
static const __int128 GLOBAL_CONST_E = 
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL;  /* Only high word set */

/* Global array with constants that force comparisons during initialization */
static const __int128 global_array[] = {
    GLOBAL_CONST_A,
    GLOBAL_CONST_B,
    GLOBAL_CONST_C,
    GLOBAL_CONST_D,
    GLOBAL_CONST_E,
    ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL,  /* Min signed */
    ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Max signed */
    0,  /* Zero */
    -1,  /* All ones */
};

/* Static assertions to force compile-time comparisons */
_Static_assert(GLOBAL_CONST_A < GLOBAL_CONST_B, "A < B should hold");
_Static_assert(GLOBAL_CONST_B < GLOBAL_CONST_C, "B < C should hold");
_Static_assert(GLOBAL_CONST_D < GLOBAL_CONST_E, "D < E should hold");

/* Function 1: Range analysis with comparisons in both high and low words */
__int128 range_compare(__int128 x, __int128 y) {
    /* These comparisons should trigger double_int::cmp in VRP */
    if (x < GLOBAL_CONST_A) {
        return x + 1;
    } else if (x > GLOBAL_CONST_C) {
        return x - 1;
    } else if (x == GLOBAL_CONST_B) {
        return y;
    }
    
    /* Compare parameters with each other */
    if (x < y) {
        return x;
    } else if (x > y) {
        return y;
    }
    
    return x + y;
}

/* Function 2: Loop with __int128 induction variable */
__int128 loop_with_wide_bounds(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that may be analyzed by VRP with wide bounds */
    for (__int128 i = start; i < end; i = i + 1) {
        sum = sum + i;
        
        /* Conditional with comparison to trigger cmp */
        if (i < (GLOBAL_CONST_A >> 2)) {
            sum = sum + 1;
        }
    }
    
    return sum;
}

/* Function 3: Bitwise operations crossing word boundaries */
__int128 cross_word_operations(__int128 x) {
    /* Left shift moving bits from low to high word */
    __int128 shifted = x << 65;
    
    /* Right shift with sign extension */
    __int128 arith_shifted = shifted >> 32;
    
    /* Mask operations targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    __int128 high_part = x & mask_high;
    __int128 low_part = x & mask_low;
    
    /* Comparisons that may trigger double_int::cmp */
    if (high_part < (mask_high >> 1)) {
        return low_part;
    }
    
    return arith_shifted;
}

/* Function 4: Overflow checking with __int128 */
int check_overflow_operations(__int128 a, __int128 b, __int128 *sum, __int128 *product) {
    __int128 tmp_sum, tmp_prod;
    int overflow_sum, overflow_prod;
    
    /* These may internally use double_int comparisons */
    overflow_sum = __builtin_add_overflow(a, b, &tmp_sum);
    overflow_prod = __builtin_mul_overflow(a, b, &tmp_prod);
    
    if (sum) *sum = tmp_sum;
    if (product) *product = tmp_prod;
    
    /* Compare results with constants */
    if (!overflow_sum && tmp_sum > GLOBAL_CONST_D) {
        return 1;
    }
    
    return overflow_sum || overflow_prod;
}

/* Function 5: Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(__int128 wide_val, unsigned long long narrow_val) {
    /* Implicit conversion and comparison */
    if (wide_val < narrow_val) {
        return narrow_val;
    }
    
    /* Ternary with different types */
    __int128 result = (wide_val > 0) ? wide_val : (__int128)narrow_val;
    
    /* Compare with shifted value */
    if (result < (wide_val << 4)) {
        return result + 1;
    }
    
    return result;
}

/* Function 6: Dead code with constant comparisons (may still be evaluated) */
__int128 dead_code_with_comparisons(__int128 x) {
    __int128 result = x;
    
    /* Dead code that contains constant comparisons */
    if (0) {  /* Always false */
        /* These comparisons may still be evaluated during early optimization */
        if (GLOBAL_CONST_A < GLOBAL_CONST_B) {
            result = result + 1;
        }
        
        if (((__int128)0x5555555555555555ULL << 64) > 
            ((__int128)0xAAAAAAAAAAAAAAAALL << 64)) {
            result = result - 1;
        }
        
        /* Case where high words are equal but low words differ */
        __int128 const1 = ((__int128)0x1111111111111111ULL << 64) | 0x2222222222222222ULL;
        __int128 const2 = ((__int128)0x1111111111111111ULL << 64) | 0x2222222222222223ULL;
        if (const1 < const2) {
            result = result * 2;
        }
    }
    
    return result;
}

/* Function to compute checksum of global array */
__int128 compute_checksum(void) {
    __int128 checksum = 0;
    size_t count = sizeof(global_array) / sizeof(global_array[0]);
    
    for (size_t i = 0; i < count; i++) {
        checksum = checksum ^ global_array[i];
        
        /* Add comparison to trigger optimizations */
        if (global_array[i] < checksum) {
            checksum = checksum + i;
        }
    }
    
    return checksum;
}

/* Main test driver */
int main(void) {
    __int128 test_val1 = GLOBAL_CONST_A;
    __int128 test_val2 = GLOBAL_CONST_B;
    __int128 test_val3 = -GLOBAL_CONST_C;
    
    /* Test 1: Range comparison */
    __int128 result1 = range_compare(test_val1, test_val2);
    
    /* Test 2: Loop with wide bounds */
    __int128 result2 = loop_with_wide_bounds(
        GLOBAL_CONST_D, 
        GLOBAL_CONST_D + 1000
    );
    
    /* Test 3: Cross-word operations */
    __int128 result3 = cross_word_operations(test_val1);
    
    /* Test 4: Overflow checking */
    __int128 sum, product;
    int overflow_result = check_overflow_operations(
        test_val1, 
        test_val2, 
        &sum, 
        &product
    );
    
    /* Test 5: Mixed-type comparisons */
    __int128 result5 = mixed_type_comparisons(test_val1, 0xFFFFFFFFULL);
    
    /* Test 6: Dead code path */
    __int128 result6 = dead_code_with_comparisons(test_val3);
    
    /* Final checksum */
    __int128 final_checksum = compute_checksum();
    
    /* Combine results in a way that can't be optimized away */
    __int128 combined = 
        result1 + result2 + result3 + result5 + result6 + 
        sum + product + final_checksum;
    
    /* Print lower 64 bits for verification */
    unsigned long long low_part = (unsigned long long)combined;
    unsigned long long high_part = (unsigned long long)(combined >> 64);
    
    printf("Result low part: 0x%016llx\n", low_part);
    printf("Result high part: 0x%016llx\n", high_part);
    printf("Overflow detected: %d\n", overflow_result);
    
    return 0;
}
