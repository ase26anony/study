/* double-int-test.c - Test program to trigger double_int::cmp in GCC */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define CONST_A_HIGH 0x123456789ABCDEF0ULL
#define CONST_A_LOW  0xFEDCBA9876543210ULL
#define CONST_A (((__int128)CONST_A_HIGH << 64) | CONST_A_LOW)

#define CONST_B_HIGH 0x123456789ABCDEF0ULL  /* Same high as A */
#define CONST_B_LOW  0xFEDCBA9876543211ULL  /* Different low */
#define CONST_B (((__int128)CONST_B_HIGH << 64) | CONST_B_LOW)

#define CONST_C_HIGH 0x123456789ABCDEF1ULL  /* Different high */
#define CONST_C_LOW  0xFEDCBA9876543210ULL
#define CONST_C (((__int128)CONST_C_HIGH << 64) | CONST_C_LOW)

#define CONST_D_HIGH 0x0000000000000000ULL  /* Zero high word */
#define CONST_D_LOW  0xFFFFFFFFFFFFFFFFULL  /* Max low word */
#define CONST_D (((__int128)CONST_D_HIGH << 64) | CONST_D_LOW)

#define CONST_E_HIGH 0xFFFFFFFFFFFFFFFFULL  /* Max high word */
#define CONST_E_LOW  0x0000000000000000ULL  /* Zero low word */
#define CONST_E (((__int128)CONST_E_HIGH << 64) | CONST_E_LOW)

#define CONST_F_HIGH 0x8000000000000000ULL  /* High bit set */
#define CONST_F_LOW  0x0000000000000000ULL
#define CONST_F (((__int128)CONST_F_HIGH << 64) | CONST_F_LOW)

/* Global arrays with __int128 constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    CONST_A, CONST_B, CONST_C, CONST_D, CONST_E, CONST_F,
    -CONST_A, -CONST_B, -CONST_C, /* Negative versions */
    ((__int128)0x1ULL << 127) - 1, /* Max positive 128-bit */
    (__int128)0x1ULL << 127,       /* Most negative 128-bit */
};

/* Structure with __int128 fields */
struct wide_pair {
    __int128 a;
    __int128 b;
};

/* Global structure with __int128 constants */
static const struct wide_pair pairs[] = {
    { CONST_A, CONST_B },
    { CONST_C, CONST_D },
    { CONST_E, CONST_F },
    { -CONST_A, -CONST_B },
};

/* Test 1: Function with __int128 comparisons for VRP */
__int128 test_range_analysis(__int128 x, __int128 y) {
    /* These comparisons should trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        if (y > CONST_B) {
            return x + y;
        }
    }
    
    if (x > CONST_C && y < CONST_D) {
        return x - y;
    }
    
    /* Compare high words equal, low words different */
    if (x >= CONST_A && x <= CONST_B) {
        return x | CONST_E;
    }
    
    return x ^ y;
}

/* Test 2: Loop with __int128 induction variable */
__int128 test_loop_comparisons(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with __int128 bounds - compiler must compare start < end */
    for (__int128 i = start; i < end; i += (end - start) / 100) {
        /* Force comparisons in loop body */
        if (i < CONST_D) {
            sum += i & CONST_F;
        } else if (i > CONST_E) {
            sum += i | CONST_A;
        } else {
            sum += i ^ CONST_B;
        }
        
        /* Additional comparison to trigger more double_int::cmp */
        if ((i & CONST_C) == 0) {
            sum >>= 1;
        }
    }
    
    return sum;
}

/* Test 3: Overflow operations with __int128 */
int test_overflow_ops(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* These may trigger double_int comparisons in overflow checking */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) return -1;
    
    overflow = __builtin_mul_overflow(a, CONST_A, &result);
    if (overflow) return -2;
    
    overflow = __builtin_sub_overflow(b, CONST_B, &result);
    if (overflow) return -3;
    
    return 0;
}

/* Test 4: Bitwise operations crossing word boundaries */
__int128 test_bitwise_boundaries(__int128 x) {
    __int128 result = 0;
    
    /* Left shift moving bits from low to high word */
    result |= (x << 65);  /* Shift by >64 bits */
    result |= (x << 96);  /* Large shift */
    
    /* Right shift with sign extension (arithmetic shift for signed) */
    result |= (x >> 72);  /* Shift crossing word boundary */
    
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    result |= (x & mask_high) >> 32;
    result |= (x & mask_low) << 32;
    
    return result;
}

/* Test 5: Mixed-type comparisons and conversions */
int test_mixed_comparisons(__int128 big, unsigned long long small) {
    int result = 0;
    
    /* Compare __int128 with narrower types */
    if (big < small) result |= 1;
    if (big > (__int128)small) result |= 2;
    if (big == (__int128)small) result |= 4;
    
    /* Ternary operator with mixed types */
    __int128 ternary_result = (small > 1000) ? CONST_A : small;
    result += (ternary_result == CONST_A) ? 8 : 0;
    
    /* Compare with zero (special case) */
    if (big > 0) result |= 16;
    if (big < 0) result |= 32;
    
    return result;
}

/* Test 6: Dead code with __int128 comparisons (for constant folding) */
void dead_code_paths(void) {
    /* These comparisons should be evaluated at compile time */
    if (0) {  /* Dead code, but constants still processed */
        if (CONST_A < CONST_B) {
            /* High words equal, low words differ */
            volatile __int128 dummy = CONST_A + CONST_B;
        }
        
        if (CONST_A < CONST_C) {
            /* High words differ */
            volatile __int128 dummy = CONST_A - CONST_C;
        }
        
        if (CONST_D < CONST_E) {
            /* Zero high vs max high */
            volatile __int128 dummy = CONST_D | CONST_E;
        }
        
        if (CONST_F < 0) {
            /* Negative value (high bit set) */
            volatile __int128 dummy = ~CONST_F;
        }
    }
}

/* Test 7: Static assertions with __int128 comparisons */
#ifdef __cplusplus
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT _Static_assert
#endif

/* These force compile-time evaluation of comparisons */
STATIC_ASSERT(CONST_A != CONST_B, "Constants must differ");
STATIC_ASSERT(CONST_C > CONST_A, "C should be greater than A");
STATIC_ASSERT(CONST_D < CONST_E, "D should be less than E");

/* Test 8: Switch statement with __int128 (GCC extension) */
int test_switch(__int128 value) {
    /* GCC doesn't directly support __int128 in switch, but we can work around */
    int result = 0;
    
    if (value == CONST_A) {
        result = 1;
    } else if (value == CONST_B) {
        result = 2;
    } else if (value == CONST_C) {
        result = 3;
    } else if (value == CONST_D) {
        result = 4;
    } else if (value == CONST_E) {
        result = 5;
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    __int128 sum = 0;
    __int128 checksum = 0;
    
    printf("Testing __int128 operations to trigger double_int::cmp\n");
    
    /* Initialize with some test values */
    __int128 test_val1 = CONST_A;
    __int128 test_val2 = CONST_B;
    __int128 test_val3 = CONST_C;
    
    /* Test 1: Range analysis */
    __int128 range_result = test_range_analysis(test_val1, test_val2);
    sum += range_result;
    
    /* Test 2: Loop comparisons */
    __int128 loop_result = test_loop_comparisons(CONST_D, CONST_E);
    sum += loop_result;
    
    /* Test 3: Overflow operations */
    int overflow_result = test_overflow_ops(test_val1, test_val2);
    sum += overflow_result;
    
    /* Test 4: Bitwise boundaries */
    __int128 bitwise_result = test_bitwise_boundaries(test_val3);
    sum += bitwise_result;
    
    /* Test 5: Mixed comparisons */
    int mixed_result = test_mixed_comparisons(test_val1, 0x123456789ABCDEFULL);
    sum += mixed_result;
    
    /* Test 6: Dead code (should be optimized out) */
    dead_code_paths();
    
    /* Test 8: Switch-like comparisons */
    int switch_result = test_switch(CONST_A);
    sum += switch_result;
    
    /* Process global array of constants */
    for (size_t i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        checksum ^= global_consts[i];
    }
    
    /* Process structure pairs */
    for (size_t i = 0; i < sizeof(pairs)/sizeof(pairs[0]); i++) {
        checksum += pairs[i].a - pairs[i].b;
    }
    
    /* Final computation to ensure all values are used */
    __int128 final_result = sum + checksum;
    
    /* Print low 64 bits of result for verification */
    unsigned long long low_part = (unsigned long long)final_result;
    printf("Result (low 64 bits): 0x%016llx\n", low_part);
    
    /* Additional compile-time checks using __builtin_constant_p */
    if (__builtin_constant_p(CONST_A < CONST_B)) {
        printf("Constant comparison A < B is evaluable at compile time\n");
    }
    
    if (__builtin_constant_p(CONST_A < CONST_C)) {
        printf("Constant comparison A < C is evaluable at compile time\n");
    }
    
    return 0;
}
