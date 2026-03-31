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
#define CONST_C_LOW  0x0000000000000000ULL
#define CONST_C (((__int128)CONST_C_HIGH << 64) | CONST_C_LOW)

#define CONST_D_HIGH 0x0000000000000000ULL  /* Zero high */
#define CONST_D_LOW  0xFFFFFFFFFFFFFFFFULL  /* Max low */
#define CONST_D (((__int128)CONST_D_HIGH << 64) | CONST_D_LOW)

#define CONST_E_HIGH 0xFFFFFFFFFFFFFFFFULL  /* Max high */
#define CONST_E_LOW  0x0000000000000000ULL  /* Zero low */
#define CONST_E (((__int128)CONST_E_HIGH << 64) | CONST_E_LOW)

/* Global arrays with 128-bit constants - forces compile-time initialization */
static const __int128 global_consts[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL, /* Min signed */
    ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL, /* Max signed */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(CONST_A < CONST_B, "A should be less than B (low word diff)");
_Static_assert(CONST_A < CONST_C, "A should be less than C (high word diff)");
_Static_assert(CONST_D < CONST_E, "D should be less than E");
_Static_assert(CONST_A != CONST_B, "A and B should be different");

/* Dead code with comparisons that may still be evaluated during optimization */
static __int128 dead_code_comparisons(__int128 x) {
    if (0) {  /* Dead branch, but constants may be compared during early opts */
        if (x < CONST_A) return CONST_B;
        if (x > CONST_C) return CONST_D;
        if (CONST_A < CONST_B) return CONST_E;
        if ((unsigned __int128)CONST_A < (unsigned __int128)CONST_B) return 0;
    }
    return x;
}

/* Function using value range propagation with 128-bit values */
__int128 vrp_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger VRP analysis */
    if (x < CONST_A) {
        if (y > CONST_B) {
            return x + y;
        }
    }
    
    /* Range checks spanning both words */
    if (x >= CONST_D && x <= CONST_E) {
        return x | CONST_A;
    }
    
    /* Ternary with mixed types */
    return (x < 1000ULL) ? (__int128)1000 : CONST_C;
}

/* Loop with 128-bit induction variable */
__int128 loop_test(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop where bounds differ in high words */
    for (__int128 i = start; i < end; i += ((__int128)1 << 64)) {
        sum += i;
        
        /* Comparison inside loop */
        if (i > CONST_A && i < CONST_C) {
            sum |= CONST_B;
        }
    }
    return sum;
}

/* Bitwise operations crossing word boundaries */
__int128 bitwise_test(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low  = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 shifted = x << 65;  /* Moves bits from low to high word */
    __int128 masked = x & mask_high;
    __int128 combined = (x & mask_high) | (CONST_A & mask_low);
    
    /* Comparisons after bitwise ops */
    if (shifted > CONST_B) {
        return masked;
    }
    
    return combined;
}

/* Arithmetic with overflow checking */
__int128 overflow_test(__int128 a, __int128 b) {
    __int128 result;
    
    /* Overflow checks may use double_int comparisons internally */
    if (__builtin_add_overflow(a, b, &result)) {
        return CONST_E;  /* Return max on overflow */
    }
    
    if (__builtin_mul_overflow(a, b, &result)) {
        return CONST_D;
    }
    
    return result;
}

/* Mixed-type comparisons and conversions */
int mixed_type_test(__int128 x, unsigned long long y) {
    /* Implicit conversions and comparisons */
    if (x < y) {  /* y promoted to 128-bit */
        return -1;
    }
    
    if (x > (__int128)y * 2) {
        return 1;
    }
    
    /* Ternary with mixed types */
    __int128 z = (y > 1000) ? CONST_A : (__int128)y;
    
    return (z == x) ? 0 : -1;
}

/* Switch statement with large constants (simulated with if-else) */
int switch_like_test(__int128 x) {
    if (x == CONST_A) return 1;
    if (x == CONST_B) return 2;
    if (x == CONST_C) return 3;
    if (x == CONST_D) return 4;
    if (x == CONST_E) return 5;
    return 0;
}

/* Preprocessor comparisons using __builtin_constant_p */
#ifdef __GNUC__
#define COMPILE_TIME_CMP(a, b) \
    (__builtin_constant_p(a) && __builtin_constant_p(b) ? (a) < (b) : 0)
#endif

/* Main test function */
int main() {
    __int128 test_values[] = {
        CONST_A,
        CONST_B,
        CONST_C,
        0,
        -1,
        ((__int128)1 << 127) - 1,  /* Max positive */
        (__int128)1 << 127,        /* Min negative */
    };
    
    __int128 sum = 0;
    
    /* Process global constants */
    for (size_t i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        sum += global_consts[i];
    }
    
    /* Test various functions */
    for (size_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        sum += dead_code_comparisons(test_values[i]);
        sum += vrp_test(test_values[i], test_values[(i+1) % 7]);
        sum += bitwise_test(test_values[i]);
        sum += overflow_test(test_values[i], CONST_A);
        
        /* Call functions but discard result to ensure they're not optimized away */
        (void)loop_test(CONST_D, CONST_E);
        (void)mixed_type_test(test_values[i], (unsigned long long)i);
        (void)switch_like_test(test_values[i]);
    }
    
    /* Compute a simple checksum to print */
    unsigned long long checksum = (unsigned long long)(sum >> 64) ^ (unsigned long long)sum;
    printf("Checksum: 0x%016llx\n", checksum);
    
    return 0;
}
