/* double-int-test.c - Test program to trigger double_int::cmp coverage */

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
static const __int128 LARGE_D = ((__int128)0x8000000000000000ULL << 64) | 0x0ULL; /* Min signed */
static const __int128 LARGE_E = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* Max signed */

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_array[] = {
    LARGE_A,
    LARGE_B,
    LARGE_C,
    LARGE_D,
    LARGE_E,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Only low word set */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL,  /* Only high word set */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(LARGE_A < LARGE_C, "Compile-time comparison 1");
_Static_assert(LARGE_B > LARGE_A, "Compile-time comparison 2");
_Static_assert(LARGE_D < LARGE_E, "Compile-time comparison 3");

/* Dead code with comparisons that compiler may evaluate during optimization */
static __int128 dead_code_comparisons(__int128 x) {
    if (0) {  /* Dead code, but compiler may still analyze constants */
        if (LARGE_A < LARGE_B) return LARGE_A;
        if (LARGE_C > LARGE_B) return LARGE_C;
        if (((__int128)0x1ULL << 120) > ((__int128)0x1ULL << 119)) return x;
    }
    return x;
}

/* Function to test range analysis with 128-bit comparisons */
__int128 range_analysis_test(__int128 input) {
    /* Comparisons that should trigger VRP analysis */
    if (input < LARGE_A) {
        return input + 1;
    } else if (input > LARGE_C) {
        return input - 1;
    } else if (input >= LARGE_B && input <= LARGE_C) {
        /* This range spans both high and low word boundaries */
        return input * 2;
    }
    
    /* Compare with mixed-type constant */
    if (input < 0xFFFFFFFFULL) {  /* unsigned long long comparison */
        return input | 0xFF;
    }
    
    return input;
}

/* Loop with 128-bit induction variable */
void loop_with_128bit_iv(__int128 start, __int128 end) {
    volatile __int128 sum = 0;  /* volatile to prevent complete optimization */
    
    /* Loop where bounds differ in both high and low words */
    for (__int128 i = start; i < end; i = i + (((__int128)1ULL << 60) | 1ULL)) {
        sum += i;
        
        /* Nested comparison with large constant */
        if (i > LARGE_A) {
            sum -= 1;
        }
    }
    
    /* Prevent unused variable warning */
    (void)sum;
}

/* Test bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 x, __int128 y) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 result = (x & mask_high) | (y & mask_low);
    
    /* Shift operations moving bits between words */
    result = result << 65;  /* Moves bits from low to high word */
    result = result >> 32;  /* Partial shift back */
    
    /* Arithmetic shift with negative value */
    __int128 negative = -((__int128)1ULL << 120);
    result = negative >> 70;  /* Arithmetic shift affecting high word */
    
    return result;
}

/* Test overflow builtins with 128-bit values */
int overflow_operations(__int128 a, __int128 b, __int128 *sum, __int128 *prod) {
    __int128 temp_sum, temp_prod;
    int overflow_add = __builtin_add_overflow(a, b, &temp_sum);
    int overflow_mul = __builtin_mul_overflow(a, b, &temp_prod);
    
    if (sum) *sum = temp_sum;
    if (prod) *prod = temp_prod;
    
    /* Comparisons that may be analyzed */
    if (temp_sum > LARGE_E) return 2;
    if (temp_prod < LARGE_D) return 3;
    
    return overflow_add | (overflow_mul << 1);
}

/* Ternary operator with type conversions */
__int128 ternary_with_conversions(int selector, unsigned long long val) {
    /* Mixed-type ternary forcing conversions and comparisons */
    return selector ? 
           ((__int128)val << 64) | val :  /* Large 128-bit value */
           (__int128)val;                 /* Promoted 64-bit value */
}

/* Switch statement with 128-bit comparisons (simulated) */
int switch_like_128bit(__int128 key) {
    /* GCC doesn't allow 128-bit in switch directly, but we can simulate */
    if (key == LARGE_A) return 1;
    if (key == LARGE_B) return 2;
    if (key == LARGE_C) return 3;
    if (key == LARGE_D) return 4;
    if (key == LARGE_E) return 5;
    
    /* Range comparisons */
    if (key > LARGE_A && key < LARGE_C) return 6;
    if (key < 0 && key > LARGE_D) return 7;
    
    return 0;
}

/* Main test function */
int main() {
    __int128 test_values[] = {
        LARGE_A,
        LARGE_B,
        LARGE_C,
        0,
        -1,
        ((__int128)1ULL << 127) - 1,  /* Max positive */
        (__int128)1ULL << 127,        /* Most negative */
    };
    
    __int128 checksum = 0;
    
    /* Test range analysis */
    for (int i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        checksum += range_analysis_test(test_values[i]);
    }
    
    /* Test loops with 128-bit bounds */
    loop_with_128bit_iv(LARGE_D, LARGE_D + (((__int128)1ULL << 64) | 0xFFULL));
    
    /* Test bitwise operations */
    checksum += bitwise_operations(LARGE_A, LARGE_B);
    
    /* Test overflow operations */
    __int128 sum, prod;
    overflow_operations(LARGE_A, LARGE_B, &sum, &prod);
    checksum += sum + prod;
    
    /* Test ternary conversions */
    checksum += ternary_with_conversions(1, 0x123456789ABCDEFULL);
    
    /* Test switch-like comparisons */
    for (int i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        checksum += switch_like_128bit(test_values[i]);
    }
    
    /* Use dead code function */
    checksum += dead_code_comparisons(checksum);
    
    /* Compute simple output to verify execution */
    unsigned long long low = (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL);
    unsigned long long high = (unsigned long long)((checksum >> 64) & 0xFFFFFFFFFFFFFFFFULL);
    
    printf("Checksum: 0x%016llx%016llx\n", high, low);
    printf("Test completed successfully.\n");
    
    return 0;
}
