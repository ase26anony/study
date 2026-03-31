#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Test 1: Global array initializers with full 128-bit constants */
static const __int128 global_constants[] = {
    /* High words differ */
    ((__int128)0x0000000000000000ULL << 64) | 0x0000000000000000ULL,  // 0
    ((__int128)0x0000000000000001ULL << 64) | 0x0000000000000000ULL,  // 2^64
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  // -1
    ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL,  // min signed
    
    /* High words equal, low words differ */
    ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000000ULL,
    ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000001ULL,
    ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
    
    /* Edge cases for comparisons */
    ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  // max signed
    ((__int128)0x0000000000000000ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  // max unsigned low
};

/* Test 2: Static assertions forcing compile-time comparisons */
#ifdef __cplusplus
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT _Static_assert
#endif

/* These should trigger double_int::cmp during constant folding */
STATIC_ASSERT((((__int128)0x1000000000000000ULL << 64) | 0x0ULL) > 
              (((__int128)0x0FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL),
              "High word comparison test");

STATIC_ASSERT((((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000001ULL) >
              (((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000000ULL),
              "Low word comparison test");

/* Test 3: Function with __int128 comparisons for VRP */
__int128 compare_with_constants(__int128 a, __int128 b) {
    /* Constants that differ in high word */
    const __int128 threshold_high = ((__int128)0x1000000000000000ULL << 64) | 0x0ULL;
    
    /* Constants with same high word, different low word */
    const __int128 threshold_low1 = ((__int128)0x2000000000000000ULL << 64) | 0x1000000000000000ULL;
    const __int128 threshold_low2 = ((__int128)0x2000000000000000ULL << 64) | 0x2000000000000000ULL;
    
    __int128 result = 0;
    
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (a > threshold_high) {
        result += ((__int128)0x1ULL << 64);
    }
    
    if (b < threshold_low1) {
        result += 1;
    }
    
    /* Ternary with mixed comparisons */
    result = (a > b) ? result : -result;
    
    /* Compare high words equal, low words differ */
    if (a > threshold_low1 && a < threshold_low2) {
        result |= ((__int128)0x1ULL << 63);
    }
    
    return result;
}

/* Test 4: Loop with __int128 induction variable */
__int128 loop_with_wide_bounds(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop bounds that span multiple 64-bit words */
    for (__int128 i = start; i < end; i += ((__int128)0x100000000ULL << 32)) {
        /* Force comparison in loop condition */
        if (i > ((__int128)0x8000000000000000ULL << 64)) {
            sum += i;
        } else {
            sum -= i;
        }
    }
    
    return sum;
}

/* Test 5: Bitwise operations crossing word boundaries */
__int128 cross_word_operations(__int128 x) {
    /* Shift operations that move bits between words */
    __int128 shifted = x << 65;  /* Moves bits from low to high word */
    __int128 masked = shifted & (((__int128)0xFFFFFFFFULL << 64) | 0xFFFFFFFFULL);
    
    /* Arithmetic right shift of negative number */
    __int128 neg = -x;
    __int128 arith_shifted = neg >> 96;  /* Affects high word significantly */
    
    return masked | arith_shifted;
}

/* Test 6: Overflow operations */
int check_overflow_operations(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow_sum, overflow_mul;
    
    /* These may trigger double_int comparisons internally */
    overflow_sum = __builtin_add_overflow(a, b, &sum);
    overflow_mul = __builtin_mul_overflow(a, b, &product);
    
    /* Compare results with constants */
    const __int128 large_const = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL;
    
    if (sum > large_const) {
        return 1;
    }
    
    if (product < -large_const) {
        return -1;
    }
    
    return 0;
}

/* Test 7: Mixed-type comparisons */
int mixed_type_comparisons(__int128 wide, unsigned long long narrow) {
    int result = 0;
    
    /* Implicit conversion and comparison */
    if (wide > narrow) {
        result |= 0x1;
    }
    
    /* Ternary with different types */
    __int128 temp = (wide < 0) ? (__int128)narrow : wide;
    
    /* Compare with promoted value */
    if (temp == (__int128)narrow) {
        result |= 0x2;
    }
    
    return result;
}

/* Test 8: Dead code with constant comparisons (still processed by early opts) */
void dead_code_with_comparisons(void) {
    if (0) {  /* Dead code, but constants still folded */
        const __int128 dead1 = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL;
        const __int128 dead2 = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAABULL;
        
        /* These comparisons should still be evaluated during constant folding */
        if (dead1 < dead2) {
            /* High words equal, low words differ */
            volatile int dummy = 1;
            (void)dummy;
        }
        
        const __int128 dead3 = ((__int128)0x5555555555555556ULL << 64) | 0x0000000000000000ULL;
        if (dead2 < dead3) {
            /* High words differ */
            volatile int dummy = 2;
            (void)dummy;
        }
    }
}

/* Test 9: Structure with __int128 members */
struct wide_struct {
    __int128 a;
    __int128 b;
    int result;
};

int compare_struct_members(struct wide_struct *ws) {
    /* Direct comparison of structure members */
    if (ws->a > ws->b) {
        ws->result = 1;
    } else if (ws->a < ws->b) {
        ws->result = -1;
    } else {
        ws->result = 0;
    }
    
    return ws->result;
}

/* Main test driver */
int main(void) {
    __int128 checksum = 0;
    
    /* Test 1: Process global constants */
    for (size_t i = 0; i < sizeof(global_constants)/sizeof(global_constants[0]); i++) {
        checksum += global_constants[i];
    }
    
    /* Test 2 & 3: Function with comparisons */
    const __int128 test_a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    const __int128 test_b = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543211ULL;
    
    checksum += compare_with_constants(test_a, test_b);
    
    /* Test 4: Loop with wide bounds */
    const __int128 loop_start = ((__int128)0x1000000000000000ULL << 64);
    const __int128 loop_end = ((__int128)0x1000000000000001ULL << 64) | 0x1000000000000000ULL;
    
    checksum += loop_with_wide_bounds(loop_start, loop_end);
    
    /* Test 5: Cross-word operations */
    checksum += cross_word_operations(test_a);
    
    /* Test 6: Overflow checks */
    checksum += check_overflow_operations(test_a, test_b);
    
    /* Test 7: Mixed-type comparisons */
    checksum += mixed_type_comparisons(test_a, 0xFFFFFFFFFFFFFFFFULL);
    
    /* Test 8: Trigger dead code (for constant folding) */
    dead_code_with_comparisons();
    
    /* Test 9: Structure comparisons */
    struct wide_struct ws = {
        .a = ((__int128)0x1ULL << 64) | 0x0ULL,
        .b = ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        .result = 0
    };
    checksum += compare_struct_members(&ws);
    
    /* Additional constant comparisons in main */
    const __int128 cmp1 = ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    const __int128 cmp2 = ((__int128)0x1ULL << 64) | 0x0ULL;
    
    if (cmp1 < cmp2) {  /* High word comparison */
        checksum += 1;
    }
    
    /* Print a simple result to verify execution */
    unsigned long long low = (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL);
    printf("Checksum low word: 0x%016llx\n", low);
    
    return 0;
}
