#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Test 1: Global array initializers with full 128-bit constants */
static const __int128 g_constants[] = {
    /* High word differs */
    ((__int128)0x0000000000000000ULL << 64) | 0x0000000000000000ULL,  /* 0 */
    ((__int128)0x0000000000000001ULL << 64) | 0x0000000000000000ULL,  /* 2^64 */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* -1 */
    ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL,  /* Most negative */
    
    /* High word equal, low word differs */
    ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000000ULL,
    ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000001ULL,
    ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
    
    /* Edge cases for comparisons */
    ((__int128)0x0000000000000000ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
    ((__int128)0x0000000000000001ULL << 64) | 0x0000000000000000ULL,
    ((__int128)0x0000000000000000ULL << 64) | 0x7FFFFFFFFFFFFFFFULL,
};

/* Test 2: Static assertions forcing compile-time comparisons */
#ifdef __cplusplus
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT _Static_assert
#endif

/* These should trigger double_int::cmp during constant folding */
STATIC_ASSERT((((__int128)0x1000000000000000ULL << 64) | 0x0000000000000000ULL) >
              (((__int128)0x0FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL),
              "High word comparison 1");

STATIC_ASSERT((((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000000ULL) <
              (((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000001ULL),
              "Low word comparison when high equal");

/* Test 3: Range analysis with wide integers */
__int128 range_analysis_test(__int128 a, __int128 b) {
    /* Comparisons that should trigger VRP analysis */
    __int128 large_const = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 small_const = ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
    
    if (a > large_const) {
        /* High word of a > 0x7FFFFFFFFFFFFFFF */
        return b + ((__int128)1 << 64);
    } else if (a < small_const) {
        /* High word of a < 0x8000000000000000 (negative) */
        return b - ((__int128)1 << 64);
    } else if (a == ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL) {
        /* Exact match with both words */
        return a;
    }
    
    /* Compare where high words might be equal */
    __int128 threshold = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    if (a > threshold && b > threshold) {
        return a + b;
    }
    
    return a - b;
}

/* Test 4: Bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 x) {
    /* Left shift moving bits from low to high word */
    __int128 shifted = x << 72;  /* Shifts past 64-bit boundary */
    
    /* Right shift on negative values (arithmetic shift) */
    __int128 neg_val = -(((__int128)1 << 126) | 0x1);
    __int128 shifted_neg = neg_val >> 68;
    
    /* Mask operations targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFF00000000ULL << 64) | 0x0;
    __int128 mask_low = 0x00000000FFFFFFFFULL;
    
    __int128 result = (shifted & mask_high) | (shifted_neg & mask_low);
    
    /* Comparison that might trigger double_int::cmp in optimization */
    if ((result & mask_high) > (((__int128)0x8000000000000000ULL << 64) | 0x0)) {
        return result | 0x1;
    }
    
    return result;
}

/* Test 5: Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(unsigned long long ull_val, __int128 i128_val) {
    /* Compare __int128 with unsigned long long */
    __int128 large_const = ((__int128)0x1ULL << 64) | 0x0ULL;  /* 2^64 */
    
    if (i128_val > ull_val) {
        /* Promotion of ull_val to __int128 and comparison */
        return i128_val - ull_val;
    }
    
    /* Ternary operator with type mixing */
    __int128 result = (ull_val > 1000) ? 
        (((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL) :
        (__int128)ull_val;
    
    /* Comparison where high word differs */
    if (result > large_const) {
        return result >> 1;
    }
    
    return result;
}

/* Test 6: Loop with __int128 induction variable */
void loop_with_wide_bounds(__int128 start, __int128 end, __int128 *output) {
    /* Loop bounds that differ in both high and low words */
    __int128 loop_end = end;
    __int128 sum = 0;
    
    /* Dead code path with comparisons that compiler might still analyze */
    if (0) {  /* Never executed, but compiler may analyze */
        __int128 dead_const1 = ((__int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x5555555555555555ULL;
        __int128 dead_const2 = ((__int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x5555555555555556ULL;
        if (dead_const1 < dead_const2) {
            /* Should trigger comparison where high equal, low differs */
            sum = dead_const1;
        }
    }
    
    for (__int128 i = start; i < loop_end; i += ((__int128)1 << 62)) {
        /* Loop comparison i < loop_end may trigger double_int::cmp */
        sum += i & 0xFF;
        
        /* Additional comparison inside loop */
        __int128 threshold = ((__int128)0x4000000000000000ULL << 64) | 0x0ULL;
        if (i > threshold) {
            sum += 1;
        }
    }
    
    *output = sum;
}

/* Test 7: Overflow operations */
int overflow_operations(__int128 a, __int128 b, __int128 *sum, __int128 *product) {
    __int128 s, p;
    int overflow_sum = __builtin_add_overflow(a, b, &s);
    int overflow_mul = __builtin_mul_overflow(a, b, &p);
    
    *sum = s;
    *product = p;
    
    /* Comparisons that might be part of overflow checking */
    __int128 max_val = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 min_val = ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
    
    if (s > max_val || s < min_val) {
        return 1;
    }
    
    return overflow_sum || overflow_mul;
}

/* Test 8: Global computations */
static __int128 compute_global_checksum(void) {
    __int128 checksum = 0;
    for (size_t i = 0; i < sizeof(g_constants)/sizeof(g_constants[0]); i++) {
        checksum ^= g_constants[i];
        
        /* Compare with various thresholds during computation */
        __int128 threshold1 = ((__int128)0x1000000000000000ULL << 64) | 0x0ULL;
        __int128 threshold2 = ((__int128)0x0ULL << 64) | 0x8000000000000000ULL;
        
        if (checksum > threshold1) {
            checksum >>= 1;
        } else if (checksum < threshold2) {
            checksum <<= 1;
        }
    }
    return checksum;
}

/* Main test driver */
int main(void) {
    __int128 result = 0;
    
    /* Test 1: Use global constants */
    for (size_t i = 0; i < sizeof(g_constants)/sizeof(g_constants[0]) - 1; i++) {
        if (g_constants[i] < g_constants[i + 1]) {
            result += g_constants[i];
        } else {
            result -= g_constants[i + 1];
        }
    }
    
    /* Test 2: Range analysis */
    __int128 test_val1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 test_val2 = ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    result += range_analysis_test(test_val1, test_val2);
    
    /* Test 3: Bitwise operations */
    result ^= bitwise_operations(test_val1);
    
    /* Test 4: Mixed type */
    result += mixed_type_comparisons(0xFFFFFFFFFFFFFFFFULL, test_val1);
    
    /* Test 5: Loop with wide bounds */
    __int128 loop_result;
    __int128 loop_start = ((__int128)0x0ULL << 64) | 0x0ULL;
    __int128 loop_end = ((__int128)0x0ULL << 64) | 0x100ULL;  /* Small for runtime */
    loop_with_wide_bounds(loop_start, loop_end, &loop_result);
    result += loop_result;
    
    /* Test 6: Overflow operations */
    __int128 sum, product;
    overflow_operations(test_val1, test_val2, &sum, &product);
    result += sum;
    result ^= product;
    
    /* Test 7: Global checksum */
    result += compute_global_checksum();
    
    /* Print a simple result (just low 64 bits for simplicity) */
    unsigned long long low_part = (unsigned long long)(result & 0xFFFFFFFFFFFFFFFFULL);
    printf("Result checksum (low 64 bits): 0x%016llx\n", low_part);
    
    return 0;
}
