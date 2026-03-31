#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_DIFF_LOW_EQUAL_A (((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL)
#define HIGH_DIFF_LOW_EQUAL_B (((__int128)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL)

#define HIGH_EQUAL_LOW_DIFF_A (((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL)
#define HIGH_EQUAL_LOW_DIFF_B (((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000001ULL)

#define NEGATIVE_LARGE_A (((__int128)(-1) << 64) | 0x8000000000000000ULL)
#define NEGATIVE_LARGE_B (((__int128)(-1) << 64) | 0x7FFFFFFFFFFFFFFFULL)

/* Global arrays with 128-bit constants - forces compile-time initialization */
static const __int128 global_consts[] = {
    HIGH_DIFF_LOW_EQUAL_A,
    HIGH_DIFF_LOW_EQUAL_B,
    HIGH_EQUAL_LOW_DIFF_A,
    HIGH_EQUAL_LOW_DIFF_B,
    NEGATIVE_LARGE_A,
    NEGATIVE_LARGE_B,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Only low word set */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL,  /* Only high word set */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
               "High word difference comparison");
_Static_assert(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B,
               "Low word difference comparison");
_Static_assert(NEGATIVE_LARGE_A < NEGATIVE_LARGE_B,
               "Negative 128-bit comparison");

/* Dead code with comparisons that GCC may still evaluate during optimization */
static __int128 dead_code_comparisons(__int128 x) {
    if (0) {  /* Dead branch, but compiler may still analyze constants */
        if (x < HIGH_DIFF_LOW_EQUAL_A) return x + 1;
        if (x > HIGH_DIFF_LOW_EQUAL_B) return x - 1;
        if (x == HIGH_EQUAL_LOW_DIFF_A) return 0;
    }
    return x;
}

/* Function with range analysis on 128-bit values */
__int128 range_analysis_function(__int128 a, __int128 b) {
    /* Comparisons that should trigger VRP analysis */
    if (a < HIGH_DIFF_LOW_EQUAL_A && b > HIGH_EQUAL_LOW_DIFF_B) {
        return a + b;
    }
    
    if (a > NEGATIVE_LARGE_A && b < NEGATIVE_LARGE_B) {
        return a - b;
    }
    
    /* Ternary with mixed types forcing conversions */
    unsigned long long ull_val = 0xFFFFFFFFFFFFFFFFULL;
    __int128 result = (a < b) ? (__int128)ull_val : HIGH_DIFF_LOW_EQUAL_A;
    
    return result;
}

/* Function with bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 cleared_high = x & ~mask_high;
    __int128 cleared_low = x & ~mask_low;
    
    /* Shifts moving bits between words */
    __int128 shift_left = x << 65;  /* Moves bits from low to high word */
    __int128 shift_right = x >> 65; /* Moves bits from high to low word */
    
    return cleared_high | (cleared_low >> 32) | (shift_left & mask_high) | (shift_right & mask_low);
}

/* Loop with 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop bound comparisons involve 128-bit values */
    for (__int128 i = start; i < end; i += ((__int128)1 << 62)) {
        sum += i;
        
        /* Additional comparison inside loop */
        if (i > HIGH_DIFF_LOW_EQUAL_A) {
            sum += HIGH_EQUAL_LOW_DIFF_B;
        }
    }
    
    return sum;
}

/* Function using overflow builtins with 128-bit operands */
int overflow_checks(__int128 a, __int128 b, __int128 *result) {
    __int128 sum, product;
    int overflow_add, overflow_mul;
    
    overflow_add = __builtin_add_overflow(a, b, &sum);
    overflow_mul = __builtin_mul_overflow(a, b, &product);
    
    *result = sum + product;
    return overflow_add | overflow_mul;
}

/* Switch statement with 128-bit values (simulated with if-else chain) */
int switch_like_128bit(__int128 val) {
    if (val == HIGH_DIFF_LOW_EQUAL_A) return 1;
    if (val == HIGH_DIFF_LOW_EQUAL_B) return 2;
    if (val == HIGH_EQUAL_LOW_DIFF_A) return 3;
    if (val == HIGH_EQUAL_LOW_DIFF_B) return 4;
    if (val == NEGATIVE_LARGE_A) return 5;
    if (val == NEGATIVE_LARGE_B) return 6;
    return 0;
}

/* Main test function */
int main() {
    __int128 test_val1 = HIGH_DIFF_LOW_EQUAL_A;
    __int128 test_val2 = HIGH_EQUAL_LOW_DIFF_B;
    
    /* Test range analysis */
    __int128 range_result = range_analysis_function(test_val1, test_val2);
    
    /* Test bitwise operations */
    __int128 bitwise_result = bitwise_operations(test_val1);
    
    /* Test loop with 128-bit bounds */
    __int128 loop_result = loop_with_128bit_iv(
        ((__int128)0x1000000000000000ULL << 64),  /* Start */
        ((__int128)0x1000000000000001ULL << 64)   /* End - differs in low word */
    );
    
    /* Test overflow checks */
    __int128 overflow_result;
    int overflow = overflow_checks(test_val1, test_val2, &overflow_result);
    
    /* Process global constants */
    __int128 global_sum = 0;
    for (size_t i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        global_sum += global_consts[i];
    }
    
    /* Call dead code function (should be optimized out but may trigger comparisons) */
    dead_code_comparisons(test_val1);
    
    /* Simulate switch behavior */
    int switch_result = switch_like_128bit(test_val1);
    
    /* Final checksum computation */
    __int128 final_checksum = range_result + bitwise_result + loop_result + 
                             overflow_result + global_sum + switch_result;
    
    /* Print low 64 bits of checksum for verification */
    printf("Checksum low word: 0x%016llx\n", (unsigned long long)(final_checksum & 0xFFFFFFFFFFFFFFFFULL));
    printf("Checksum high word: 0x%016llx\n", (unsigned long long)(final_checksum >> 64));
    
    return 0;
}
