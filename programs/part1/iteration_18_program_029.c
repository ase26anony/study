/* double_int_test.c - Test program to trigger double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Global constants with different high/low word combinations */
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

/* Global array initialization forcing constant folding */
static const __int128 GLOBAL_ARRAY[] = {
    GLOBAL_CONST_A,
    GLOBAL_CONST_B,
    GLOBAL_CONST_C,
    GLOBAL_CONST_D,
    GLOBAL_CONST_E,
    ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL,  /* Min signed */
    ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Max signed */
    0,  /* Zero */
    -1, /* All ones */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(GLOBAL_CONST_A < GLOBAL_CONST_C, "High word comparison should work");
_Static_assert(GLOBAL_CONST_A < GLOBAL_CONST_B, "Low word comparison should work");
_Static_assert(GLOBAL_CONST_D > 0, "Positive low word only");
_Static_assert(GLOBAL_CONST_E < 0, "Negative high word only");

/* Function 1: Range analysis with comparisons in both high and low words */
__int128 range_compare(__int128 x, __int128 y) {
    /* These comparisons should trigger double_int::cmp in VRP */
    if (x < GLOBAL_CONST_A) {
        return x + 1;
    }
    if (x > GLOBAL_CONST_C) {
        return x - 1;
    }
    if (y < GLOBAL_CONST_B && y > GLOBAL_CONST_D) {
        return y;
    }
    return x;
}

/* Function 2: Loop with __int128 induction variable */
__int128 loop_with_wide_bounds(__int128 start, __int128 end) {
    __int128 sum = 0;
    /* Loop that may be analyzed by VRP with wide bounds */
    for (__int128 i = start; i < end; i += ((__int128)1 << 32)) {
        sum += i;
    }
    return sum;
}

/* Function 3: Bitwise operations crossing word boundaries */
__int128 cross_word_operations(__int128 x) {
    /* Left shift moving bits from low to high word */
    __int128 shifted = x << 72;
    
    /* Right shift with sign extension */
    __int128 arith_shifted = x >> 88;
    
    /* Mask operations targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    __int128 high_only = x & mask_high;
    __int128 low_only = x & mask_low;
    
    return shifted ^ arith_shifted ^ high_only ^ low_only;
}

/* Function 4: Mixed-type comparisons and conversions */
int mixed_type_comparisons(__int128 a, unsigned long long b) {
    /* Implicit conversion and comparison */
    if (a == b) {
        return 1;
    }
    
    /* Ternary operator with type mixing */
    __int128 result = (a > 0) ? a : (__int128)b;
    
    /* Compare with different constant types */
    if (result < 100ULL) {
        return 2;
    }
    
    if (result > -100LL) {
        return 3;
    }
    
    return 0;
}

/* Function 5: Overflow checking with wide integers */
int check_overflow_operations(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow_sum, overflow_product;
    
    /* These may trigger double_int comparisons internally */
    overflow_sum = __builtin_add_overflow(a, b, &sum);
    overflow_product = __builtin_mul_overflow(a, b, &product);
    
    /* Comparisons that might be analyzed */
    if (!overflow_sum && sum > GLOBAL_CONST_A) {
        return 1;
    }
    
    if (!overflow_product && product < GLOBAL_CONST_B) {
        return 2;
    }
    
    return 0;
}

/* Function 6: Dead code with constant comparisons (may still be evaluated) */
__int128 dead_code_comparisons(__int128 x) {
    __int128 result = x;
    
    /* Dead code that compiler might still analyze */
    if (0) {  /* Always false */
        /* These comparisons differ in high word */
        if (GLOBAL_CONST_C > GLOBAL_CONST_A) {
            result += 1;
        }
        
        /* These comparisons differ in low word */
        if (GLOBAL_CONST_B > GLOBAL_CONST_A) {
            result += 2;
        }
        
        /* Equal high words, different low words */
        __int128 dead_const1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000000ULL;
        __int128 dead_const2 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000001ULL;
        if (dead_const1 < dead_const2) {
            result += 4;
        }
    }
    
    return result;
}

/* Function to compute checksum of global array */
__int128 compute_global_checksum(void) {
    __int128 checksum = 0;
    size_t count = sizeof(GLOBAL_ARRAY) / sizeof(GLOBAL_ARRAY[0]);
    
    for (size_t i = 0; i < count; i++) {
        checksum ^= GLOBAL_ARRAY[i];
    }
    
    return checksum;
}

/* Main test driver */
int main(void) {
    /* Test values covering different comparison scenarios */
    __int128 test_values[] = {
        GLOBAL_CONST_A,
        GLOBAL_CONST_B,
        GLOBAL_CONST_C,
        GLOBAL_CONST_D,
        GLOBAL_CONST_E,
        0,
        -1,
        ((__int128)1 << 64),
        ((__int128)-1 << 63),
    };
    
    __int128 total_result = 0;
    
    /* Execute various test functions */
    for (size_t i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++) {
        for (size_t j = 0; j < sizeof(test_values) / sizeof(test_values[0]); j++) {
            /* Range comparison test */
            total_result ^= range_compare(test_values[i], test_values[j]);
            
            /* Mixed type comparisons */
            total_result += mixed_type_comparisons(test_values[i], 
                (unsigned long long)(test_values[j] & 0xFFFFFFFFFFFFFFFFULL));
            
            /* Overflow checking */
            total_result += check_overflow_operations(test_values[i], test_values[j]);
        }
        
        /* Cross-word operations */
        total_result ^= cross_word_operations(test_values[i]);
        
        /* Dead code comparisons */
        total_result ^= dead_code_comparisons(test_values[i]);
    }
    
    /* Loop with wide bounds */
    __int128 loop_result = loop_with_wide_bounds(
        GLOBAL_CONST_D, 
        GLOBAL_CONST_A
    );
    total_result += loop_result;
    
    /* Global checksum */
    __int128 global_checksum = compute_global_checksum();
    total_result ^= global_checksum;
    
    /* Print a simple result to verify execution */
    unsigned long long low_part = (unsigned long long)(total_result & 0xFFFFFFFFFFFFFFFFULL);
    unsigned long long high_part = (unsigned long long)((total_result >> 64) & 0xFFFFFFFFFFFFFFFFULL);
    
    printf("Test result checksum: 0x%016llx%016llx\n", high_part, low_part);
    printf("All tests completed.\n");
    
    return 0;
}
