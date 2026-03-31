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
static const __int128 CONST_E = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* Max signed */

/* Global arrays with 128-bit constants - forces compile-time initialization */
static const __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    ((__int128)0x0ULL << 64) | 0x0ULL,  /* Zero */
    ((__int128)0x1ULL << 64) | 0x0ULL,  /* 2^64 */
    ((__int128)0x0ULL << 64) | 0x1ULL,  /* 1 */
    ((__int128)0x8000000000000000ULL << 64) | 0x0ULL,  /* Min signed */
};

/* Static assertions that force compile-time comparisons */
_Static_assert(CONST_A < CONST_B, "A should be less than B (same high, different low)");
_Static_assert(CONST_A < CONST_C, "A should be less than C (different high)");
_Static_assert(CONST_D < CONST_A, "-1 should be less than A");
_Static_assert(CONST_E > CONST_A, "Max signed should be greater than A");

/* Function that triggers VRP comparisons with 128-bit values */
__int128 compare_with_constants(__int128 x, __int128 y) {
    /* These comparisons should trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        return x + 1;
    }
    if (y > CONST_B) {
        return y - 1;
    }
    if (x == CONST_C) {
        return x;
    }
    
    /* Ternary with mixed-type comparisons */
    return (x < y) ? x : y;
}

/* Function with 128-bit loop induction variable */
__int128 sum_range(__int128 start, __int128 end) {
    __int128 sum = 0;
    /* Loop with 128-bit comparison in condition */
    for (__int128 i = start; i < end; i = i + 1) {
        sum += i;
        /* Break early to avoid long runtime */
        if (i > start + 100) break;
    }
    return sum;
}

/* Function using bitwise operations crossing word boundaries */
__int128 manipulate_bits(__int128 x) {
    /* Shift operations that cross 64-bit boundary */
    __int128 shifted = x << 65;  /* Moves bits from low to high word */
    __int128 masked = shifted & (((__int128)0xFFFFFFFFULL << 96) | 0x0ULL);
    
    /* Arithmetic right shift of negative value */
    __int128 neg = -x;
    __int128 arith_shifted = neg >> 70;
    
    return masked | (arith_shifted & 0xFF);
}

/* Function with overflow checks that may use double_int comparisons */
int check_overflow(__int128 a, __int128 b) {
    __int128 result;
    int overflow = __builtin_add_overflow(a, b, &result);
    
    if (!overflow) {
        overflow = __builtin_mul_overflow(a, b, &result);
    }
    
    return overflow;
}

/* Function comparing 128-bit with 64-bit values */
__int128 mixed_type_comparison(__int128 a, unsigned long long b) {
    /* Implicit conversion and comparison */
    if (a < b) {
        return a;
    }
    
    /* Explicit comparison with sign extension */
    __int128 b_128 = (__int128)b;
    if (a > b_128) {
        return b_128;
    }
    
    return a;
}

/* Dead code with comparisons that compiler may still evaluate */
void dead_code_paths(void) {
    if (0) {  /* Dead code, but compiler may still analyze constants */
        /* Comparisons that should trigger double_int::cmp */
        volatile __int128 dummy;
        if (CONST_A < CONST_B) dummy = 1;
        if (CONST_B > CONST_C) dummy = 2;
        if (CONST_A == CONST_A) dummy = 3;
        
        /* More complex comparisons */
        __int128 temp = CONST_A + CONST_B;
        if (temp > CONST_C) dummy = 4;
    }
}

/* Switch statement with 128-bit derived values (simulated) */
int switch_with_large_values(__int128 x) {
    /* Convert to range suitable for switch */
    int val = 0;
    if (x < CONST_A) val = 1;
    else if (x < CONST_B) val = 2;
    else if (x < CONST_C) val = 3;
    else val = 4;
    
    switch (val) {
        case 1: return 100;
        case 2: return 200;
        case 3: return 300;
        case 4: return 400;
        default: return 0;
    }
}

/* Main function that exercises all test cases */
int main(void) {
    __int128 result = 0;
    
    /* Test 1: Compare with constants */
    result += compare_with_constants(CONST_A - 10, CONST_B + 10);
    
    /* Test 2: Sum range with 128-bit bounds */
    __int128 start = CONST_A - 1000;
    __int128 end = CONST_A + 1000;
    result += sum_range(start, end);
    
    /* Test 3: Bit manipulation */
    result += manipulate_bits(CONST_A);
    
    /* Test 4: Overflow checks */
    result += check_overflow(CONST_A, CONST_B);
    
    /* Test 5: Mixed type comparisons */
    result += mixed_type_comparison(CONST_A, 0xFFFFFFFFFFFFFFFFULL);
    
    /* Test 6: Dead code (should be optimized out but may trigger comparisons) */
    dead_code_paths();
    
    /* Test 7: Switch with derived values */
    result += switch_with_large_values(CONST_B);
    
    /* Test 8: Process global array */
    for (size_t i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        result += global_array[i] & 0xFF;  /* Use only low byte to avoid overflow */
    }
    
    /* Additional comparisons in main */
    if (CONST_A < CONST_B && CONST_B < CONST_C) {
        result += 1;
    }
    
    /* Print result (simplified to 64-bit for portability) */
    unsigned long long low = (unsigned long long)result;
    unsigned long long high = (unsigned long long)(result >> 64);
    printf("Result: 0x%016llx%016llx\n", high, low);
    
    return 0;
}
