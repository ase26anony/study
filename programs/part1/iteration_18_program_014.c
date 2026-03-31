/* double_int_cmp_test.c - Test program to trigger double_int::cmp in GCC */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define CONST_A_HIGH 0x123456789ABCDEF0ULL
#define CONST_A_LOW  0xFEDCBA9876543210ULL
#define CONST_B_HIGH 0x123456789ABCDEF0ULL  /* Same high as A */
#define CONST_B_LOW  0xFEDCBA9876543211ULL  /* Different low */
#define CONST_C_HIGH 0x123456789ABCDEF1ULL  /* Different high */
#define CONST_C_LOW  0xFEDCBA9876543210ULL

/* Construct 128-bit constants */
static const __int128 LARGE_A = ((__int128)CONST_A_HIGH << 64) | CONST_A_LOW;
static const __int128 LARGE_B = ((__int128)CONST_B_HIGH << 64) | CONST_B_LOW;
static const __int128 LARGE_C = ((__int128)CONST_C_HIGH << 64) | CONST_C_LOW;

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_array[] = {
    LARGE_A,
    LARGE_B,
    LARGE_C,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Only low word set */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL,  /* Only high word set */
    ((__int128)0x8000000000000000ULL << 64) | 0x0ULL,  /* Minimum negative */
    ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL, /* Max positive */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(LARGE_A < LARGE_C, "Compile-time comparison 1");
_Static_assert(LARGE_B > LARGE_A, "Compile-time comparison 2");
_Static_assert(LARGE_A != LARGE_B, "Compile-time comparison 3");

/* Test 1: Function with 128-bit comparisons for VRP */
__int128 test_range_analysis(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x < LARGE_A) {
        return x + 1;
    }
    if (y > LARGE_B) {
        return y - 1;
    }
    
    /* Ternary with mixed comparisons */
    __int128 result = (x < y) ? LARGE_A : LARGE_B;
    
    /* Nested comparisons */
    if ((x > LARGE_A) && (x < LARGE_C)) {
        result = result | 0x1;
    }
    
    return result;
}

/* Test 2: Loop with 128-bit induction variable */
__int128 test_loop_comparisons(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that compares 128-bit values */
    for (__int128 i = start; i < end; i = i + 1) {
        /* Additional comparison inside loop */
        if (i > LARGE_A) {
            sum = sum + i;
        } else {
            sum = sum - i;
        }
        
        /* Prevent infinite loops with practical bounds */
        if (i > start + 1000) break;
    }
    
    return sum;
}

/* Test 3: Bitwise operations crossing word boundaries */
__int128 test_bitwise_operations(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    
    /* Operations that require reasoning about both words */
    __int128 shifted = x << 65;  /* Crosses word boundary */
    __int128 masked = (x & mask_high) | ((x & mask_low) >> 32);
    
    /* Comparison after shift */
    if (shifted > LARGE_B) {
        return masked;
    }
    
    /* Arithmetic right shift on negative values */
    __int128 neg_val = -x;
    __int128 arith_shifted = neg_val >> 70;
    
    if (arith_shifted < 0) {
        return arith_shifted;
    }
    
    return masked;
}

/* Test 4: Overflow operations */
__int128 test_overflow_operations(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* Addition with overflow check */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) {
        return LARGE_A;
    }
    
    /* Multiplication with overflow check */
    overflow = __builtin_mul_overflow(result, a, &result);
    if (overflow) {
        return LARGE_B;
    }
    
    /* Compare result with large constants */
    if (result > LARGE_C) {
        return result >> 4;
    }
    
    return result;
}

/* Test 5: Mixed-type comparisons and conversions */
__int128 test_mixed_comparisons(__int128 x, unsigned long long y) {
    /* Implicit conversion and comparison */
    __int128 y_extended = y;
    
    /* Mixed comparisons */
    if (x < y_extended) {
        return x;
    }
    
    /* Ternary with different types */
    __int128 result = (x > 1000) ? y_extended : x;
    
    /* Compare with constant after conversion */
    if (result < (__int128)0xFFFFFFFFFFFFFFFFULL) {  /* Compare with 64-bit max */
        return result + 1;
    }
    
    return result;
}

/* Test 6: Dead code with 128-bit comparisons (still processed by compiler) */
void test_dead_code_paths(void) {
    /* Dead code that still gets parsed and potentially constant-folded */
    if (0) {  /* Always false */
        /* Complex comparisons that should trigger double_int::cmp */
        if (LARGE_A < LARGE_B) {
            volatile __int128 dead_var = LARGE_C;
        }
        
        /* Multiple comparisons in dead code */
        __int128 dead_sum = 0;
        for (__int128 i = LARGE_A; i < LARGE_C; i = i + LARGE_B) {
            dead_sum = dead_sum + i;
        }
    }
    
    /* Another dead branch */
    if (sizeof(__int128) == 0) {  /* Always false */
        _Static_assert(LARGE_A != LARGE_B, "Dead static assert");
    }
}

/* Test 7: Global array computations */
__int128 compute_global_checksum(void) {
    __int128 checksum = 0;
    const int size = sizeof(global_array) / sizeof(global_array[0]);
    
    for (int i = 0; i < size; i++) {
        /* Compare array elements with constants */
        if (global_array[i] > LARGE_A) {
            checksum = checksum + global_array[i];
        } else {
            checksum = checksum - global_array[i];
        }
        
        /* Compare consecutive elements */
        if (i > 0 && global_array[i] > global_array[i-1]) {
            checksum = checksum | 0x1;
        }
    }
    
    return checksum;
}

/* Main test driver */
int main(void) {
    /* Initialize test values with different high/low patterns */
    __int128 test1 = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL;
    __int128 test2 = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAABULL;
    __int128 test3 = ((__int128)0x5555555555555556ULL << 64) | 0xAAAAAAAAAAAAAAAAULL;
    
    printf("Starting 128-bit comparison tests...\n");
    
    /* Run all tests */
    __int128 result1 = test_range_analysis(test1, test2);
    __int128 result2 = test_loop_comparisons(test1, test3);
    __int128 result3 = test_bitwise_operations(test2);
    __int128 result4 = test_overflow_operations(test1, test3);
    __int128 result5 = test_mixed_comparisons(test1, 0xFFFFFFFFFFFFFFFFULL);
    
    test_dead_code_paths();
    
    __int128 checksum = compute_global_checksum();
    
    /* Combine results into a final checksum */
    __int128 final_result = result1 + result2 + result3 + result4 + result5 + checksum;
    
    /* Print low 64 bits of result for verification */
    unsigned long long low_part = (unsigned long long)final_result;
    unsigned long long high_part = (unsigned long long)(final_result >> 64);
    
    printf("Test completed. Result: 0x%016llx%016llx\n", high_part, low_part);
    printf("High word: 0x%016llx, Low word: 0x%016llx\n", high_part, low_part);
    
    return 0;
}
