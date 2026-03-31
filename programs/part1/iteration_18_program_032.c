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

#define CONST_D_HIGH 0x0ULL                 /* Small high word */
#define CONST_D_LOW  0xFFFFFFFFFFFFFFFFULL
#define CONST_D (((__int128)CONST_D_HIGH << 64) | CONST_D_LOW)

#define CONST_E_HIGH 0xFFFFFFFFFFFFFFFFULL  /* Max negative */
#define CONST_E_LOW  0x0ULL
#define CONST_E (((__int128)CONST_E_HIGH << 64) | CONST_E_LOW)

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    -CONST_A,
    -CONST_B,
    ((__int128)0x8000000000000000ULL << 64) | 0x0ULL, /* Min 128-bit */
    ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL, /* Max 128-bit */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(CONST_A < CONST_B, "A < B (low word comparison)");
_Static_assert(CONST_B < CONST_C, "B < C (high word comparison)");
_Static_assert(CONST_D > 0, "D > 0 (mixed word comparison)");
_Static_assert(CONST_E < 0, "E < 0 (negative high word)");

/* Dead code with comparisons that GCC may evaluate during optimization */
static __int128 dead_code_comparisons(__int128 x) {
    if (0) {  /* Dead code, but constants may be compared during folding */
        if (CONST_A < CONST_C) return CONST_A;
        if (CONST_B > CONST_D) return CONST_B;
        if (CONST_E == ((__int128)-1 << 64)) return CONST_E;
    }
    return x;
}

/* Function to test range analysis with 128-bit values */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger VRP analysis */
    if (x < CONST_A) {
        if (y > CONST_B) {
            return x + y;
        } else if (y < CONST_D) {
            return x - y;
        }
    } else if (x > CONST_C) {
        if (y < CONST_E) {
            return x * 2;
        }
    }
    
    /* Ternary with mixed types forcing conversions */
    return (x < 100ULL) ? (__int128)100ULL : y;
}

/* Loop with 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    for (__int128 i = start; i < end; i += ((__int128)1 << 32)) {
        sum += i;
        /* Cross-word comparison in loop condition */
        if (i > CONST_D && i < CONST_A) {
            sum += 1;
        }
    }
    return sum;
}

/* Bitwise operations crossing word boundaries */
__int128 bitwise_crossword_ops(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 shifted = x << 65;  /* Moves bits from low to high word */
    __int128 masked = x & mask_high;
    __int128 combined = (x & mask_high) | ((x << 32) & mask_low);
    
    /* Comparisons after bitwise ops */
    if (shifted > CONST_B) return shifted;
    if (masked < CONST_D) return masked;
    return combined;
}

/* Overflow checking with 128-bit values */
int overflow_checks(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* These may trigger double_int comparisons internally */
    overflow = __builtin_add_overflow(a, b, &result);
    if (!overflow && result > CONST_A) return 1;
    
    overflow = __builtin_mul_overflow(a, b, &result);
    if (!overflow && result < CONST_E) return -1;
    
    return 0;
}

/* Mixed-type comparisons */
int mixed_type_comparisons(unsigned long long ull, __int128 i128) {
    /* Implicit conversions requiring comparisons */
    if (i128 == ull) return 1;
    if (i128 < ull) return -1;
    if (i128 > CONST_D && ull < CONST_D_LOW) return 2;
    
    /* Ternary with different types */
    __int128 result = (ull > 1000) ? (__int128)ull : i128;
    return result > 0 ? 1 : 0;
}

/* Structure with 128-bit bitfield (implementation-defined, but may trigger conversions) */
struct with_128bit {
    __int128 value;
    unsigned int flag : 1;
};

/* Global initialization with computed values */
static const __int128 computed_global = 
    (CONST_A + CONST_B) / 2;

/* Array operations forcing constant folding */
__int128 process_global_array(void) {
    __int128 sum = 0;
    for (size_t i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        /* Compare with various constants during summation */
        if (global_consts[i] > CONST_D) {
            sum += global_consts[i];
        } else if (global_consts[i] < CONST_E) {
            sum -= global_consts[i];
        } else {
            sum ^= global_consts[i];
        }
    }
    return sum;
}

/* Main test function */
int main(void) {
    /* Initialize with values that exercise different comparison paths */
    __int128 test_vals[] = {
        CONST_A,
        CONST_B,
        CONST_C,
        CONST_D,
        CONST_E,
        0,
        -1,
        ((__int128)1 << 127) - 1,
        (__int128)-1 << 127,
    };
    
    __int128 total = 0;
    
    /* Exercise various comparison scenarios */
    for (int i = 0; i < sizeof(test_vals)/sizeof(test_vals[0]); i++) {
        total += dead_code_comparisons(test_vals[i]);
        total += range_analysis_test(test_vals[i], test_vals[(i+1) % 8]);
        total += bitwise_crossword_ops(test_vals[i]);
        
        /* Test comparisons where high words are equal but low words differ */
        if (i > 0) {
            if (test_vals[i] < test_vals[i-1]) total += 1;
            if (test_vals[i] > test_vals[i-1]) total += 2;
        }
    }
    
    /* Test loops with 128-bit bounds */
    total += loop_with_128bit_iv(CONST_D, CONST_A);
    
    /* Test overflow checks */
    for (int i = 0; i < 4; i++) {
        total += overflow_checks(test_vals[i], test_vals[i+4]);
    }
    
    /* Test mixed-type comparisons */
    for (unsigned long long ull = 0; ull < 1000; ull += 100) {
        total += mixed_type_comparisons(ull, CONST_D + ull);
    }
    
    /* Process global array */
    total += process_global_array();
    
    /* Use computed global */
    total ^= computed_global;
    
    /* Print a verifiable result (truncated to 64-bit for portability) */
    unsigned long long result_low = (unsigned long long)(total & 0xFFFFFFFFFFFFFFFFULL);
    printf("Result checksum: 0x%016llx\n", result_low);
    
    /* Additional static assertions for compile-time evaluation */
    _Static_assert((CONST_A & ((__int128)0xFFFFFFFFULL << 96)) != 0, 
                   "High word bits check");
    _Static_assert((CONST_D | ((__int128)1 << 64)) > CONST_D,
                   "Bit setting crosses word boundary");
    
    return 0;
}
