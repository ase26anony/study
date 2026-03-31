/* double-int-test.c - Test program to trigger double_int::cmp coverage */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_A  0x123456789ABCDEF0ULL
#define LOW_A   0xFEDCBA9876543210ULL
#define HIGH_B  0x123456789ABCDEF0ULL  /* Same high as A */
#define LOW_B   0xFEDCBA9876543211ULL  /* Different low */
#define HIGH_C  0x123456789ABCDEF1ULL  /* Different high */
#define LOW_C   0xFEDCBA9876543210ULL

/* Construct 128-bit constants */
static const __int128 CONST_A = ((__int128)HIGH_A << 64) | LOW_A;
static const __int128 CONST_B = ((__int128)HIGH_B << 64) | LOW_B;
static const __int128 CONST_C = ((__int128)HIGH_C << 64) | LOW_C;
static const __int128 CONST_D = ((__int128)0x8000000000000000ULL << 64) | 0x0ULL; /* Min signed */
static const __int128 CONST_E = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* Max signed */

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Only low word set */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL,  /* Only high word set */
};

/* Static assertions to force compile-time comparisons */
_Static_assert(CONST_A != CONST_B, "A != B");
_Static_assert(CONST_A < CONST_C, "A < C");
_Static_assert(CONST_D < CONST_E, "Min < Max");

/* Test 1: Function with comparisons that differ in high word */
__int128 test_high_word_comparison(__int128 x, __int128 y) {
    /* These comparisons should trigger double_int::cmp with different high words */
    if (x > CONST_C) {
        return x + CONST_A;
    } else if (x < CONST_A) {
        return x - CONST_B;
    }
    
    /* Compare where high words are equal but low words differ */
    if (y >= CONST_A && y <= CONST_B) {
        return y | CONST_A;
    }
    
    return x ^ y;
}

/* Test 2: Range analysis with 128-bit values */
__int128 test_range_analysis(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with 128-bit induction variable - may trigger VRP comparisons */
    for (__int128 i = start; i < end; i += ((__int128)1 << 64)) {
        /* Cross-word boundary operations */
        sum += i & CONST_A;
        sum += i | CONST_B;
        
        /* Comparisons that may differ in high or low words */
        if (i > CONST_D && i < CONST_E) {
            sum ^= i;
        }
    }
    
    return sum;
}

/* Test 3: Bitwise operations crossing word boundaries */
__int128 test_bitwise_operations(__int128 a, __int128 b) {
    __int128 result = 0;
    
    /* Left shift moving bits from low to high word */
    result |= (a << 65);
    
    /* Right shift on negative value (arithmetic shift) */
    result |= (b >> 96);
    
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    result &= mask_high | mask_low;
    
    /* Comparison after bitwise operations */
    if ((result & mask_high) > (CONST_C & mask_high)) {
        return result | CONST_A;
    }
    
    return result;
}

/* Test 4: Mixed-type comparisons and conversions */
__int128 test_mixed_comparisons(__int128 a, unsigned long long b) {
    /* Compare 128-bit with 64-bit - requires promotion */
    if (a > b) {
        /* Ternary with mixed types */
        __int128 temp = (b > 1000) ? CONST_A : (__int128)b;
        
        /* Comparison that may differ in high word */
        if (temp < CONST_B) {
            return temp + a;
        }
    }
    
    /* Overflow checking with 128-bit */
    __int128 overflow_test;
    if (__builtin_add_overflow(a, CONST_A, &overflow_test)) {
        return CONST_D;  /* Return min on overflow */
    }
    
    return a + b;
}

/* Test 5: Dead code with constant comparisons (still processed by compiler) */
__int128 test_dead_code_comparisons(__int128 x) {
    __int128 result = x;
    
    /* Dead code block - compiler may still evaluate constants */
    if (0) {  /* Always false */
        /* These comparisons cover all uncovered line cases */
        if (CONST_A < CONST_B) result += 1;  /* High equal, low differ */
        if (CONST_B < CONST_C) result += 2;  /* High differ */
        if (CONST_C > CONST_A) result += 4;  /* Reverse comparison */
        if (CONST_D < CONST_E) result += 8;  /* Different high words */
    }
    
    /* Live code with similar comparisons */
    if (x < CONST_A) {
        result = CONST_B;
    } else if (x > CONST_C) {
        result = CONST_C;
    }
    
    return result;
}

/* Test 6: Array operations with 128-bit constants */
__int128 test_array_operations(void) {
    __int128 sum = 0;
    
    /* Process global array - forces compiler to handle constants */
    for (size_t i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        /* Compare array elements with constants */
        if (global_array[i] > CONST_A) {
            sum += global_array[i] & CONST_B;
        } else if (global_array[i] < CONST_D) {
            sum += global_array[i] | CONST_C;
        } else {
            sum ^= global_array[i];
        }
    }
    
    return sum;
}

/* Test 7: Switch statement with 128-bit in range checks */
int test_switch_range(__int128 x) {
    /* GCC may convert switch to comparison tree */
    if (x >= CONST_D && x <= CONST_A) {
        return 1;
    } else if (x > CONST_A && x < CONST_B) {
        return 2;
    } else if (x >= CONST_B && x <= CONST_C) {
        return 3;
    } else if (x > CONST_C && x <= CONST_E) {
        return 4;
    }
    return 0;
}

/* Main function that exercises all tests */
int main(void) {
    /* Initialize test values with different high/low patterns */
    __int128 test1 = CONST_A;
    __int128 test2 = CONST_B;
    __int128 test3 = CONST_C;
    __int128 test4 = CONST_D;
    __int128 test5 = CONST_E;
    
    /* Run tests designed to trigger double_int::cmp */
    __int128 result1 = test_high_word_comparison(test1, test2);
    __int128 result2 = test_range_analysis(test4, test5);
    __int128 result3 = test_bitwise_operations(test1, test3);
    __int128 result4 = test_mixed_comparisons(test2, 0x123456789ABCDEFULL);
    __int128 result5 = test_dead_code_comparisons(test3);
    __int128 result6 = test_array_operations();
    int result7 = test_switch_range(test1);
    
    /* Combine results into a simple checksum */
    __int128 final = result1 ^ result2 ^ result3 ^ result4 ^ result5 ^ result6 ^ result7;
    
    /* Print partial result (simplified for 64-bit output) */
    unsigned long long low = (unsigned long long)final;
    unsigned long long high = (unsigned long long)(final >> 64);
    
    printf("Test checksum: high=0x%016llx low=0x%016llx\n", high, low);
    printf("All tests completed (compiler should have exercised double_int::cmp)\n");
    
    return 0;
}
