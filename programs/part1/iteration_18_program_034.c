#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_A 0x123456789ABCDEF0ULL
#define LOW_A  0xFEDCBA9876543210ULL
#define HIGH_B 0x123456789ABCDEF0ULL  /* Same high as A */
#define LOW_B  0xFEDCBA9876543211ULL  /* Different low */
#define HIGH_C 0x123456789ABCDEEFULL  /* Different high */
#define LOW_C  0xFEDCBA9876543210ULL

/* Construct 128-bit constants */
static const __int128 CONST_A = ((__int128)HIGH_A << 64) | LOW_A;
static const __int128 CONST_B = ((__int128)HIGH_B << 64) | LOW_B;
static const __int128 CONST_C = ((__int128)HIGH_C << 64) | LOW_C;
static const __int128 CONST_MAX = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
static const __int128 CONST_MIN = ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_MAX,
    CONST_MIN,
    ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL,
    ((__int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL,
};

/* Test 1: Compile-time comparisons in static assertions */
_Static_assert(CONST_A != CONST_B, "Constants should differ in low word");
_Static_assert(CONST_A != CONST_C, "Constants should differ in high word");
_Static_assert(CONST_A < CONST_MAX, "A should be less than max");
_Static_assert(CONST_MIN < CONST_A, "Min should be less than A");

/* Test 2: Function with range analysis on 128-bit values */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        if (y > CONST_B) {
            /* Force comparison of high words */
            return x + y;
        }
    }
    
    /* Compare parameters directly */
    if (x < y) {
        return x - y;
    } else if (x > y) {
        return y - x;
    }
    
    return x;
}

/* Test 3: Loop with 128-bit induction variable */
__int128 loop_test(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that may trigger comparisons in loop optimization passes */
    for (__int128 i = start; i < end; i += ((__int128)1 << 32)) {
        /* Cross-word boundary operations */
        sum += i & CONST_A;
        sum -= i | CONST_B;
        
        /* Conditional based on comparison */
        if (i > CONST_C) {
            sum ^= ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
        }
    }
    
    return sum;
}

/* Test 4: Bitwise operations crossing word boundaries */
__int128 bitwise_test(__int128 x) {
    /* Shifts that move bits between high and low words */
    __int128 shifted = x << 65;  /* Moves bits from low to high word */
    shifted |= x >> 63;          /* Arithmetic right shift */
    
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Comparisons after masking */
    if ((shifted & mask_high) > CONST_A) {
        return shifted | mask_low;
    }
    
    if ((shifted & mask_low) < CONST_B) {
        return shifted & mask_high;
    }
    
    return shifted;
}

/* Test 5: Mixed-type comparisons and conversions */
__int128 mixed_type_test(unsigned long long ull_val, __int128 i128_val) {
    /* Compare 128-bit with 64-bit - requires promotion */
    if (i128_val > ull_val) {
        /* Ternary with mixed types */
        __int128 result = (ull_val > 1000) ? CONST_A : CONST_B;
        
        /* Compare result with parameter */
        if (result < i128_val) {
            return result + ull_val;
        }
    }
    
    /* Overflow checking with 128-bit values */
    __int128 overflow_check;
    if (__builtin_add_overflow(i128_val, ull_val, &overflow_check)) {
        return CONST_MAX;
    }
    
    if (__builtin_mul_overflow(i128_val, 2, &overflow_check)) {
        return CONST_MIN;
    }
    
    return i128_val + ull_val;
}

/* Test 6: Dead code with comparisons (still processed by early passes) */
void dead_code_test(void) {
    if (0) {  /* Dead code, but constants still evaluated */
        /* These comparisons should still be processed during constant folding */
        if (CONST_A < CONST_B) {
            volatile __int128 unused = CONST_C;
        }
        
        if (((__int128)0x8000000000000000ULL << 64) > 0) {
            volatile __int128 unused2 = CONST_A + CONST_B;
        }
    }
}

/* Test 7: Switch statement with large constants (if supported) */
int switch_test(__int128 val) {
    /* GCC may convert switch to decision tree with comparisons */
    if (val == CONST_A) return 1;
    if (val == CONST_B) return 2;
    if (val == CONST_C) return 3;
    if (val == CONST_MAX) return 4;
    if (val == CONST_MIN) return 5;
    
    return 0;
}

/* Test 8: Array operations with 128-bit values */
__int128 array_checksum(void) {
    __int128 sum = 0;
    
    /* Process global array - forces compiler to handle constants */
    for (size_t i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        sum += global_array[i];
        
        /* Compare array elements */
        if (i > 0 && global_array[i] > global_array[i-1]) {
            sum ^= global_array[i];
        }
    }
    
    return sum;
}

/* Main function that exercises all tests */
int main(void) {
    /* Initialize test values */
    __int128 test_val1 = CONST_A;
    __int128 test_val2 = CONST_B;
    __int128 test_val3 = CONST_C;
    
    /* Run tests */
    __int128 result1 = range_analysis_test(test_val1, test_val2);
    __int128 result2 = loop_test(CONST_MIN >> 2, CONST_MAX >> 2);
    __int128 result3 = bitwise_test(test_val3);
    __int128 result4 = mixed_type_test(0xFFFFFFFFFFFFFFFFULL, test_val1);
    __int128 result5 = array_checksum();
    
    dead_code_test();
    int switch_result = switch_test(test_val1);
    
    /* Combine results into a simple checksum */
    __int128 final_checksum = result1 + result2 + result3 + result4 + result5 + switch_result;
    
    /* Print low 64 bits of checksum for verification */
    unsigned long long low_bits = (unsigned long long)final_checksum;
    printf("Checksum (low 64 bits): 0x%016llx\n", low_bits);
    
    /* Additional compile-time checks using __builtin_constant_p */
    if (__builtin_constant_p(CONST_A < CONST_B)) {
        printf("Constant comparison folded at compile time\n");
    }
    
    return 0;
}
