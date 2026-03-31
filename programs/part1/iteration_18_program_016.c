/* double_int_test.c - Test program to trigger double_int::cmp in GCC */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define LOW_MASK  0xFEDCBA9876543210ULL
#define HIGH_MASK 0x123456789ABCDEF0ULL

/* Constants where high words differ */
static const __int128 C1 = ((__int128)HIGH_MASK << 64) | LOW_MASK;
static const __int128 C2 = ((__int128)(HIGH_MASK - 1) << 64) | LOW_MASK;
static const __int128 C3 = ((__int128)(HIGH_MASK + 1) << 64) | LOW_MASK;

/* Constants where high words are equal but low words differ */
static const __int128 C4 = ((__int128)HIGH_MASK << 64) | (LOW_MASK - 1);
static const __int128 C5 = ((__int128)HIGH_MASK << 64) | (LOW_MASK + 1);

/* Negative 128-bit constants */
static const __int128 C_NEG = -(((__int128)HIGH_MASK << 64) | LOW_MASK);
static const __int128 C_NEG2 = -(((__int128)(HIGH_MASK - 1) << 64) | (LOW_MASK + 1));

/* Global arrays with 128-bit constants - forces compile-time initialization */
static const __int128 global_array[] = {
    C1, C2, C3, C4, C5, C_NEG, C_NEG2,
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL, /* MAX */
    ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL, /* MIN */
    0, 1, -1
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(C1 > C2, "C1 should be greater than C2");
_Static_assert(C3 > C1, "C3 should be greater than C1");
_Static_assert(C4 < C1, "C4 should be less than C1");
_Static_assert(C5 > C1, "C5 should be greater than C1");
_Static_assert(C_NEG < 0, "C_NEG should be negative");

/* Function 1: Range analysis with comparisons in both high and low words */
__int128 range_compare(__int128 a, __int128 b) {
    /* These comparisons should trigger double_int::cmp during VRP */
    if (a < C1) {
        if (b > C2) {
            return a + b;
        } else if (b < C_NEG) {
            return a - b;
        }
    } else if (a > C3) {
        return a * 2;
    }
    
    /* Compare where high words might be equal */
    if (a >= C4 && a <= C5) {
        return a | 0x1;
    }
    
    return a;
}

/* Function 2: Loop with 128-bit induction variable */
__int128 loop_with_128bit_counter(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that should cause VRP to analyze 128-bit ranges */
    for (__int128 i = start; i < end; i += ((__int128)1 << 64) | 1) {
        sum += i;
        
        /* Nested comparisons with different constants */
        if (i < C1) {
            sum += 1;
        } else if (i > C3) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Function 3: Bitwise operations crossing word boundaries */
__int128 cross_word_operations(__int128 x) {
    /* Left shift moving bits from low to high word */
    __int128 shifted = x << 65;
    
    /* Right shift on negative value (arithmetic shift) */
    __int128 arith_shifted = C_NEG >> 32;
    
    /* Bitwise operations with masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    __int128 high_only = x & mask_high;
    __int128 low_only = x & mask_low;
    
    /* Comparisons that might trigger double_int::cmp */
    if ((high_only > mask_high >> 1) && (low_only < mask_low >> 1)) {
        return shifted | arith_shifted;
    }
    
    return high_only | low_only;
}

/* Function 4: Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(unsigned long long ull_val, __int128 i128_val) {
    __int128 result = 0;
    
    /* Compare __int128 with unsigned long long */
    if (i128_val > ull_val) {
        result += 1;
    }
    
    /* Ternary operator with mixed types */
    result += (ull_val > 1000) ? C1 : (__int128)ull_val;
    
    /* Switch-like logic using comparisons */
    if (i128_val < 0) {
        result += C_NEG;
    } else if (i128_val > ((__int128)1 << 120)) {
        result += C3;
    } else {
        result += C4;
    }
    
    return result;
}

/* Function 5: Overflow checking with 128-bit values */
int check_128bit_overflow(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow_sum, overflow_mul;
    
    /* These builtins may internally use double_int comparisons */
    overflow_sum = __builtin_add_overflow(a, b, &sum);
    overflow_mul = __builtin_mul_overflow(a, b, &product);
    
    /* Comparisons of results */
    if (!overflow_sum && sum > C1) {
        return 1;
    }
    
    if (!overflow_mul && product < C_NEG) {
        return 2;
    }
    
    return 0;
}

/* Function 6: Dead code with constant comparisons (still processed by compiler) */
__int128 dead_code_with_comparisons(__int128 x) {
    __int128 result = x;
    
    /* Dead code that compiler may still analyze */
    if (0) {  /* Always false */
        /* These comparisons should be evaluated at compile time */
        if (C1 < C2) result += 1;
        if (C3 > C4) result += 2;
        if (C_NEG < C_NEG2) result += 4;
        
        /* Compare where high words equal, low words differ */
        if (C1 > C4) result += 8;
        if (C1 < C5) result += 16;
    }
    
    return result;
}

/* Function to compute checksum of global array */
__int128 compute_checksum(void) {
    __int128 sum = 0;
    size_t count = sizeof(global_array) / sizeof(global_array[0]);
    
    for (size_t i = 0; i < count; i++) {
        sum += global_array[i];
        
        /* Add some comparison-based logic */
        if (global_array[i] < 0) {
            sum += 1;
        } else if (global_array[i] > C1) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Main function that exercises all test cases */
int main(void) {
    __int128 checksum = 0;
    
    /* Test 1: Range comparisons */
    checksum += range_compare(C2, C4);
    checksum += range_compare(C3, C_NEG);
    
    /* Test 2: Loop with wide bounds */
    checksum += loop_with_128bit_counter(C_NEG, C1);
    
    /* Test 3: Cross-word operations */
    checksum += cross_word_operations(C1);
    checksum += cross_word_operations(C_NEG);
    
    /* Test 4: Mixed-type comparisons */
    checksum += mixed_type_comparisons(0xFFFFFFFFFFFFFFFFULL, C2);
    checksum += mixed_type_comparisons(1000, C5);
    
    /* Test 5: Overflow checking */
    int overflow_result = check_128bit_overflow(C1 >> 2, C2 >> 2);
    checksum += overflow_result;
    
    /* Test 6: Dead code (still may trigger compile-time analysis) */
    checksum += dead_code_with_comparisons(C3);
    
    /* Final checksum computation */
    checksum += compute_checksum();
    
    /* Print result (simplified for demonstration) */
    unsigned long long low = (unsigned long long)checksum;
    unsigned long long high = (unsigned long long)(checksum >> 64);
    
    printf("Checksum: 0x%016llx%016llx\n", high, low);
    printf("Test completed - compile with -O2 -fdump-tree-vrp1 to see double_int::cmp usage\n");
    
    return 0;
}
