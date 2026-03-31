/* double_int_test.c - Test program to trigger GCC's double_int::cmp logic */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large __int128 constants with varying high/low word patterns */
static const __int128 C1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
static const __int128 C2 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543211ULL;
static const __int128 C3 = ((__int128)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL;
static const __int128 C4 = ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
static const __int128 C5 = ((__int128)0x1ULL << 64) | 0x0ULL;
static const __int128 C6 = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* -1 */
static const __int128 C7 = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* max signed */

/* Global arrays with __int128 constants */
static const __int128 global_array[] = {
    C1, C2, C3, C4, C5, C6, C7,
    ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL,
    ((__int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL,
    ((__int128)0x0ULL << 64) | 0x1ULL,
    ((__int128)0x1ULL << 64) | 0x0ULL,
    ((__int128)0x0ULL << 64) | 0x0ULL
};

/* Test 1: Constant folding with static assertions */
#ifdef __cplusplus
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

/* These force compile-time comparisons of __int128 constants */
STATIC_ASSERT(C1 != C2, "C1 should not equal C2");
STATIC_ASSERT(C3 > C1, "C3 should be greater than C1");
STATIC_ASSERT(C4 < C5, "C4 should be less than C5");
STATIC_ASSERT(C6 < C7, "-1 should be less than max signed");

/* Dead code with __int128 comparisons (may still be evaluated during early passes) */
static __int128 dead_code_comparisons(void) {
    __int128 result = 0;
    
    if (0) {  /* Dead code, but constants may still be compared */
        if (C1 < C2) result += 1;  /* High words equal, low words differ */
        if (C3 > C1) result += 2;  /* High words differ */
        if (C6 < C7) result += 4;  /* Signed comparison with high bit set */
        if (C4 < C5) result += 8;  /* High word zero vs one */
    }
    
    return result;
}

/* Test 2: Range analysis with __int128 parameters */
static __int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger VRP analysis */
    if (x < C1) {
        if (y > C2) {
            return x + y;
        } else if (y < C4) {
            return x - y;
        }
    } else if (x > C3) {
        if (y < C5) {
            return x * 2;
        }
    }
    
    /* Ternary operator with mixed types */
    unsigned long long ull = 0xFFFFFFFFFFFFFFFFULL;
    __int128 mixed = (x < 0) ? (__int128)ull : C7;
    
    return mixed + y;
}

/* Test 3: Arithmetic operations crossing word boundaries */
static __int128 boundary_crossing_ops(__int128 a, __int128 b) {
    /* Left shift moving bits from low to high word */
    __int128 shift1 = a << 65;  /* Crosses 64-bit boundary */
    __int128 shift2 = b << 96;  /* Large shift */
    
    /* Right shift on negative values (arithmetic shift) */
    __int128 neg = C6;  /* -1 */
    __int128 arith_shift = neg >> 72;
    
    /* Bitwise operations with masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    __int128 high_only = a & mask_high;
    __int128 low_only = b & mask_low;
    
    /* Overflow checking */
    __int128 sum, product;
    int overflow_add = __builtin_add_overflow(a, b, &sum);
    int overflow_mul = __builtin_mul_overflow(a, b, &product);
    
    (void)overflow_add; (void)overflow_mul; /* Suppress unused warnings */
    
    return shift1 + shift2 + arith_shift + high_only + low_only + sum + product;
}

/* Test 4: Loop with __int128 induction variable */
static __int128 loop_with_wide_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with __int128 bounds that differ in both high and low words */
    for (__int128 i = start; i < end; i += ((__int128)1 << 62)) {
        /* Complex condition to force range analysis */
        if (i > C4 && i < C5) {
            sum += i;
        } else if (i < C6 || i > C7) {
            sum -= i;
        }
        
        /* Nested comparison */
        __int128 temp = (i < 0) ? -i : i;
        if (temp > ((__int128)0x1ULL << 63)) {
            sum >>= 1;
        }
    }
    
    return sum;
}

/* Test 5: Mixed-type comparisons and conversions */
static __int128 mixed_type_comparisons(__int128 wide, unsigned long long narrow) {
    /* Compare __int128 with unsigned long long */
    __int128 result = 0;
    
    if (wide > (__int128)narrow) {
        result += 1;
    }
    
    if ((unsigned __int128)wide > (unsigned __int128)narrow) {
        result += 2;
    }
    
    /* Ternary with mixed types */
    __int128 ternary_result = (narrow > 1000) ? C1 : (__int128)narrow;
    
    /* Switch-like behavior using if-else chain */
    if (wide == C1) result += 4;
    else if (wide == C2) result += 8;
    else if (wide == C3) result += 16;
    else if (wide == C4) result += 32;
    
    return result + ternary_result;
}

/* Test 6: Global array processing */
static __int128 process_global_array(void) {
    __int128 sum = 0;
    const int size = sizeof(global_array) / sizeof(global_array[0]);
    
    /* Process array with comparisons against constants */
    for (int i = 0; i < size; i++) {
        if (global_array[i] < C1) {
            sum += global_array[i];
        } else if (global_array[i] > C3) {
            sum -= global_array[i];
        } else {
            sum ^= global_array[i];
        }
        
        /* Additional comparison to force analysis */
        for (int j = 0; j < size; j++) {
            if (global_array[i] > global_array[j]) {
                sum |= global_array[i];
            }
        }
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(void) {
    /* Initialize test values with patterns that exercise different code paths */
    __int128 test_val1 = C1;
    __int128 test_val2 = C2;
    __int128 test_val3 = C6;  /* -1 */
    __int128 test_val4 = ((__int128)0x0ULL << 64) | 0x8000000000000000ULL;
    
    /* Run tests */
    __int128 result1 = range_analysis_test(test_val1, test_val2);
    __int128 result2 = boundary_crossing_ops(test_val1, test_val3);
    __int128 result3 = loop_with_wide_iv(C4, C5);  /* Range where high word differs */
    __int128 result4 = mixed_type_comparisons(test_val1, 0xFFFFFFFFFFFFFFFFULL);
    __int128 result5 = process_global_array();
    
    /* Dead code function call (should be optimized away but constants may be compared) */
    __int128 result6 = dead_code_comparisons();
    
    /* Combine results into a simple checksum */
    __int128 final_result = result1 + result2 + result3 + result4 + result5 + result6;
    
    /* Print low 64 bits of result for verification */
    unsigned long long low_part = (unsigned long long)final_result;
    unsigned long long high_part = (unsigned long long)(final_result >> 64);
    
    printf("Result low part: 0x%016llx\n", low_part);
    printf("Result high part: 0x%016llx\n", high_part);
    printf("Checksum: 0x%016llx%016llx\n", high_part, low_part);
    
    return 0;
}
