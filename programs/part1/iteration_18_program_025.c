#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with different high/low word patterns */
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

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL, /* Min signed */
    ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL, /* Max signed */
};

/* Static assertions to force compile-time comparisons */
_Static_assert(CONST_A < CONST_B, "A < B comparison");
_Static_assert(CONST_C > CONST_A, "C > A comparison");
_Static_assert(CONST_D < CONST_E, "D < E comparison");

/* Function 1: Range analysis with comparisons in both high and low words */
__int128 range_compare(__int128 x, __int128 y) {
    /* Comparisons that will trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        return x + 1;
    } else if (x > CONST_B) {
        return x - 1;
    } else if (x == CONST_A) {
        return CONST_B;
    }
    
    /* Additional comparisons with different high word values */
    if (y < CONST_C) {
        return y + CONST_D;
    } else if (y > CONST_E) {
        return y - CONST_D;
    }
    
    return x + y;
}

/* Function 2: Loop with 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that may be analyzed by VRP */
    for (__int128 i = start; i < end; i = i + 1) {
        sum += i;
        
        /* Conditional with comparison to trigger double_int::cmp */
        if (i > CONST_A && i < CONST_C) {
            sum += CONST_D;
        }
    }
    
    return sum;
}

/* Function 3: Bitwise operations crossing word boundaries */
__int128 cross_word_operations(__int128 x) {
    /* Shifts that move bits between high and low words */
    __int128 shifted = x << 65;  /* Shift by >64 bits */
    
    /* Mask operations targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    __int128 high_part = x & mask_high;
    __int128 low_part = x & mask_low;
    
    /* Arithmetic right shift on negative values */
    __int128 neg_val = -x;
    __int128 arith_shifted = neg_val >> 72;
    
    return (shifted ^ high_part) | (low_part & arith_shifted);
}

/* Function 4: Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(unsigned long long ull_val, __int128 s128_val) {
    __int128 result = 0;
    
    /* Compare __int128 with unsigned long long */
    if (s128_val < (__int128)ull_val) {
        result = CONST_A;
    } else if (s128_val > (__int128)(ull_val * 2)) {
        result = CONST_B;
    }
    
    /* Ternary operator with type mixing */
    result = (ull_val > 1000) ? CONST_C : (__int128)ull_val;
    
    /* Compare with zero in high word only */
    if (s128_val < CONST_D) {
        result += CONST_E;
    }
    
    return result;
}

/* Function 5: Overflow checking with 128-bit values */
int check_128bit_overflow(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow_sum, overflow_mul;
    
    /* Builtins that may use double_int comparisons internally */
    overflow_sum = __builtin_add_overflow(a, b, &sum);
    overflow_mul = __builtin_mul_overflow(a, b, &product);
    
    /* Comparisons of results with constants */
    if (sum > CONST_A && product < CONST_B) {
        return 1;
    }
    
    return overflow_sum | overflow_mul;
}

/* Function 6: Dead code with constant comparisons (still processed by compiler) */
__int128 dead_code_with_comparisons(__int128 x) {
    __int128 result = x;
    
    /* Dead code that compiler may still analyze */
    if (0) {  /* Always false */
        /* These comparisons should still be evaluated during compilation */
        if (CONST_A < CONST_B) {
            result = CONST_C;
        }
        if (CONST_D > CONST_E) {
            result = CONST_D;
        }
        if (((__int128)0x7FFFFFFFFFFFFFFFULL << 64) < 
            ((__int128)0x8000000000000000ULL << 64)) {
            result = CONST_E;
        }
    }
    
    return result;
}

/* Function to compute checksum of global constants */
__int128 compute_checksum(void) {
    __int128 sum = 0;
    size_t count = sizeof(global_consts) / sizeof(global_consts[0]);
    
    for (size_t i = 0; i < count; i++) {
        sum += global_consts[i];
        
        /* Add comparisons that use both high and low words */
        if (global_consts[i] < CONST_A) {
            sum += 1;
        } else if (global_consts[i] > CONST_B) {
            sum -= 1;
        }
    }
    
    return sum;
}

int main(void) {
    /* Test values that exercise different comparison paths */
    __int128 test1 = CONST_A;
    __int128 test2 = CONST_B;
    __int128 test3 = CONST_C;
    __int128 test4 = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL;
    
    /* Call functions to trigger optimizations */
    __int128 r1 = range_compare(test1, test2);
    __int128 r2 = loop_with_128bit_iv(CONST_D, CONST_E);
    __int128 r3 = cross_word_operations(test3);
    __int128 r4 = mixed_type_comparisons(0x123456789ABCDEF0ULL, test4);
    int r5 = check_128bit_overflow(test1, test2);
    __int128 r6 = dead_code_with_comparisons(test3);
    __int128 checksum = compute_checksum();
    
    /* Final computation that uses all results */
    __int128 final_result = r1 + r2 + r3 + r4 + r6 + checksum;
    
    /* Print low 64 bits of result for verification */
    printf("Result low word: 0x%016llx\n", 
           (unsigned long long)(final_result & 0xFFFFFFFFFFFFFFFFULL));
    printf("Result high word: 0x%016llx\n", 
           (unsigned long long)((final_result >> 64) & 0xFFFFFFFFFFFFFFFFULL));
    printf("Overflow check result: %d\n", r5);
    
    return 0;
}
