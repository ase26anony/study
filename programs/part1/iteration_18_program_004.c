#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_A 0x123456789ABCDEF0ULL
#define LOW_A  0xFEDCBA9876543210ULL
#define HIGH_B 0x123456789ABCDEF0ULL  /* Same high as A */
#define LOW_B  0xFEDCBA9876543211ULL  /* Different low */
#define HIGH_C 0x123456789ABCDEF1ULL  /* Different high */
#define LOW_C  0xFEDCBA9876543210ULL

/* Full 128-bit constants */
static const __int128 CONST_A = ((__int128)HIGH_A << 64) | LOW_A;
static const __int128 CONST_B = ((__int128)HIGH_B << 64) | LOW_B;
static const __int128 CONST_C = ((__int128)HIGH_C << 64) | LOW_C;
static const __int128 CONST_D = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* -1 */
static const __int128 CONST_E = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* Max positive */

/* Global arrays with 128-bit constants - forces compile-time initialization */
static const __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    ((__int128)0x0ULL << 64) | 0x0ULL,  /* Zero */
    ((__int128)0x8000000000000000ULL << 64) | 0x0ULL,  /* Min negative */
};

/* Test 1: Function with comparisons that should trigger double_int::cmp */
__int128 test_comparison(__int128 x, __int128 y) {
    /* Comparisons where high words differ */
    if (x < CONST_C) {
        /* Force compiler to compare x with CONST_C (different high word) */
        x = x + 1;
    }
    
    /* Comparisons where high words are equal but low words differ */
    if (y > CONST_A && y < CONST_B) {
        /* This range is empty since CONST_A < CONST_B and high words equal */
        return CONST_A;
    }
    
    /* Mixed comparisons */
    if (x == CONST_A) {
        return y;
    }
    
    return x;
}

/* Test 2: Range analysis with 128-bit values */
__int128 range_analysis_test(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with 128-bit induction variable */
    for (__int128 i = start; i < end; i = i + 1) {
        /* Force range analysis to compare i with end */
        if (i < CONST_A) {
            sum = sum + i;
        } else {
            sum = sum - i;
        }
        
        /* Prevent infinite loops with practical bounds */
        if (i > start + 1000) break;
    }
    
    return sum;
}

/* Test 3: Arithmetic operations crossing word boundaries */
__int128 boundary_crossing_ops(__int128 val) {
    /* Left shift moving bits from low to high word */
    __int128 shifted = val << 72;
    
    /* Right shift on negative value (arithmetic shift) */
    __int128 neg_val = -val;
    __int128 arith_shifted = neg_val >> 80;
    
    /* Bitwise operations targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    __int128 high_only = val & mask_high;
    __int128 low_only = val & mask_low;
    
    return shifted + arith_shifted + (high_only >> 64) + low_only;
}

/* Test 4: Mixed-type comparisons and conversions */
int mixed_type_comparisons(__int128 a, unsigned long long b) {
    /* Compare 128-bit with 64-bit - requires promotion */
    if (a > (__int128)b) {
        return 1;
    }
    
    /* Ternary operator with mixed types */
    __int128 result = (a < 100) ? (__int128)b : a;
    
    /* Compare with overflow builtins */
    __int128 sum;
    if (__builtin_add_overflow(a, result, &sum)) {
        return -1;
    }
    
    return (int)(sum & 0x7FFFFFFF);
}

/* Test 5: Static assertions with 128-bit constants */
/* These force compile-time evaluation */
#define STATIC_ASSERT_128(cond) static_assert(cond, "128-bit assertion failed")

/* Dead code with comparisons that compiler may still evaluate */
void dead_code_paths(void) {
    if (0) {  /* Dead code, but compiler may still analyze constants */
        /* Comparisons that should trigger double_int::cmp */
        if (CONST_A < CONST_B) {
            /* High words equal, low words differ */
            volatile __int128 dummy = CONST_A;
        }
        
        if (CONST_B < CONST_C) {
            /* High words differ */
            volatile __int128 dummy = CONST_B;
        }
        
        if (CONST_D < CONST_E) {
            /* Both negative, high words differ */
            volatile __int128 dummy = CONST_D;
        }
    }
}

/* Test 6: Switch statement with 128-bit values (simulated) */
int switch_like_128(__int128 val) {
    /* GCC doesn't directly support switch on __int128, but we can simulate */
    if (val == CONST_A) return 1;
    if (val == CONST_B) return 2;
    if (val == CONST_C) return 3;
    if (val == CONST_D) return 4;
    if (val == CONST_E) return 5;
    
    /* Range comparisons */
    if (val < CONST_A) return 6;
    if (val > CONST_E) return 7;
    if (val > CONST_A && val < CONST_B) return 8;  /* Impossible range */
    
    return 0;
}

/* Test 7: Array operations with 128-bit values */
__int128 process_global_array(void) {
    __int128 sum = 0;
    
    for (int i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        /* Compare array elements with constants */
        if (global_array[i] < CONST_A) {
            sum = sum + global_array[i];
        } else if (global_array[i] > CONST_C) {
            sum = sum - global_array[i];
        } else {
            sum = sum ^ global_array[i];
        }
    }
    
    return sum;
}

/* Main function that exercises all tests */
int main(void) {
    /* Initialize with values that will trigger various comparison paths */
    __int128 test_val1 = CONST_A;
    __int128 test_val2 = CONST_B;
    __int128 test_val3 = CONST_C;
    
    /* Test 1: Direct comparisons */
    __int128 result1 = test_comparison(test_val1, test_val2);
    
    /* Test 2: Range analysis */
    __int128 result2 = range_analysis_test(CONST_A - 100, CONST_A + 100);
    
    /* Test 3: Boundary crossing */
    __int128 result3 = boundary_crossing_ops(test_val1);
    
    /* Test 4: Mixed types */
    int result4 = mixed_type_comparisons(test_val1, 0xFFFFFFFFFFFFFFFFULL);
    
    /* Test 5: Dead code (should be optimized out but constants analyzed) */
    dead_code_paths();
    
    /* Test 6: Switch-like behavior */
    int result6 = switch_like_128(test_val3);
    
    /* Test 7: Array processing */
    __int128 result7 = process_global_array();
    
    /* Combine results into a simple checksum */
    __int128 final_checksum = result1 + result2 + result3 + result7;
    final_checksum = final_checksum ^ ((__int128)result4 << 32);
    final_checksum = final_checksum ^ ((__int128)result6 << 16);
    
    /* Print low 64 bits of checksum for verification */
    printf("Checksum (low 64 bits): %llu\n", 
           (unsigned long long)(final_checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Static assertions that force compile-time comparisons */
    STATIC_ASSERT_128(CONST_A < CONST_C);  /* High words differ */
    STATIC_ASSERT_128(CONST_A < CONST_B);  /* High equal, low differ */
    STATIC_ASSERT_128(CONST_D < CONST_E);  /* Both negative, high differ */
    
    return 0;
}
