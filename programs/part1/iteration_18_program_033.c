#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_DIFF_LOW_EQUAL_A ((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111111ULL
#define HIGH_DIFF_LOW_EQUAL_B ((__int128)0x123456789ABCDEF1ULL << 64) | 0x1111111111111111ULL

#define HIGH_EQUAL_LOW_DIFF_A ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x1111111111111110ULL
#define HIGH_EQUAL_LOW_DIFF_B ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x1111111111111111ULL

#define NEGATIVE_LARGE_A ((__int128)(-1) * (((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL))
#define NEGATIVE_LARGE_B ((__int128)(-1) * (((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL))

/* Global arrays with 128-bit constants - forces compile-time initialization */
static const __int128 global_consts[] = {
    HIGH_DIFF_LOW_EQUAL_A,
    HIGH_DIFF_LOW_EQUAL_B,
    HIGH_EQUAL_LOW_DIFF_A,
    HIGH_EQUAL_LOW_DIFF_B,
    NEGATIVE_LARGE_A,
    NEGATIVE_LARGE_B,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Only low word set */
    ((__int128)0x1ULL << 64) | 0x0ULL,                 /* Only high word set */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
               "High word difference comparison should pass");
_Static_assert(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B,
               "Low word difference comparison should pass");
_Static_assert(NEGATIVE_LARGE_A < NEGATIVE_LARGE_B,
               "Negative value comparison should pass");

/* Dead code with comparisons that GCC may evaluate during optimization */
static __int128 dead_code_comparisons(__int128 x) {
    __int128 result = 0;
    
    /* These comparisons are dead but may be evaluated during constant folding */
    if (0) {
        if (x < HIGH_DIFF_LOW_EQUAL_A) result = 1;
        if (x > HIGH_DIFF_LOW_EQUAL_B) result = 2;
        if (x >= HIGH_EQUAL_LOW_DIFF_A && x <= HIGH_EQUAL_LOW_DIFF_B) result = 3;
    }
    
    /* More dead comparisons with different high/low patterns */
    if (0) {
        const __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
        const __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
        
        if ((x & mask_high) == mask_high) result = 4;
        if ((x & mask_low) < 0x1000000000000000ULL) result = 5;
    }
    
    return result;
}

/* Function using __int128 comparisons for Value Range Propagation */
__int128 vrp_test(__int128 a, __int128 b) {
    /* Comparisons that should trigger VRP analysis */
    if (a < HIGH_DIFF_LOW_EQUAL_A) {
        /* Range: a < HIGH_DIFF_LOW_EQUAL_A */
        if (b > HIGH_EQUAL_LOW_DIFF_B) {
            /* Range: b > HIGH_EQUAL_LOW_DIFF_B */
            return a + b;
        } else if (b < NEGATIVE_LARGE_A) {
            /* Range: b < NEGATIVE_LARGE_A */
            return a - b;
        }
    } else if (a > HIGH_DIFF_LOW_EQUAL_B) {
        /* Range: a > HIGH_DIFF_LOW_EQUAL_B */
        if (b >= HIGH_EQUAL_LOW_DIFF_A && b <= HIGH_EQUAL_LOW_DIFF_B) {
            /* Range: b in [HIGH_EQUAL_LOW_DIFF_A, HIGH_EQUAL_LOW_DIFF_B] */
            return a * b;
        }
    }
    
    /* Default case with overflow check */
    __int128 sum;
    if (__builtin_add_overflow(a, b, &sum)) {
        return a;
    }
    return sum;
}

/* Loop with __int128 induction variable */
__int128 loop_with_128bit_induction(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that may trigger range analysis on 128-bit bounds */
    for (__int128 i = start; i < end; i += ((__int128)1 << 60)) {
        /* Shift operations that cross word boundaries */
        __int128 shifted = i << 65;  /* Shift > 64 bits */
        sum += shifted >> 63;        /* Arithmetic right shift */
        
        /* Bitwise operations with word-specific masks */
        __int128 high_only = i & ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
        __int128 low_only = i & 0xFFFFFFFFFFFFFFFFULL;
        sum ^= high_only | low_only;
    }
    
    return sum;
}

/* Mixed-type comparisons and conversions */
int mixed_type_comparisons(__int128 a, unsigned long long b) {
    int result = 0;
    
    /* Compare __int128 with unsigned long long */
    if (a < b) result |= 1;
    if (a > (__int128)b) result |= 2;
    
    /* Ternary operator with type conversion */
    __int128 c = (b > 1000) ? HIGH_DIFF_LOW_EQUAL_A : (__int128)b;
    if (c == a) result |= 4;
    
    /* Multiplication with overflow check */
    __int128 prod;
    if (__builtin_mul_overflow(a, (__int128)b, &prod)) {
        result |= 8;
    }
    
    return result;
}

/* Function using preprocessor comparisons */
#ifdef __SIZEOF_INT128__
/* This #if condition ensures __int128 is available */
#define COMPILE_TIME_CHECK(cond) _Static_assert(cond, "Compile-time check failed")
#else
#define COMPILE_TIME_CHECK(cond)
#endif

/* Initialize and process global array */
__int128 process_global_consts(void) {
    __int128 sum = 0;
    
    for (size_t i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        /* Perform operations that require full 128-bit arithmetic */
        sum += global_consts[i];
        sum ^= global_consts[i] << 32;  /* Shift within low word */
        sum ^= global_consts[i] << 96;  /* Shift crossing to high word */
    }
    
    return sum;
}

int main(void) {
    /* Test with various 128-bit values */
    __int128 test_values[] = {
        HIGH_DIFF_LOW_EQUAL_A,
        HIGH_DIFF_LOW_EQUAL_B,
        HIGH_EQUAL_LOW_DIFF_A,
        HIGH_EQUAL_LOW_DIFF_B,
        NEGATIVE_LARGE_A,
        NEGATIVE_LARGE_B,
        0,
        ~(__int128)0,  /* All bits set */
    };
    
    __int128 total_sum = 0;
    
    /* Call functions that should trigger double_int comparisons */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            total_sum += vrp_test(test_values[i], test_values[j]);
            total_sum += mixed_type_comparisons(test_values[i], (unsigned long long)test_values[j]);
        }
        
        /* Test loops with 128-bit bounds */
        __int128 loop_start = test_values[i];
        __int128 loop_end = test_values[i] + ((__int128)1 << 70);
        total_sum += loop_with_128bit_induction(loop_start, loop_end);
    }
    
    /* Process global constants */
    total_sum += process_global_consts();
    
    /* Call dead code function (compiler may still analyze it) */
    total_sum += dead_code_comparisons(total_sum);
    
    /* Print a simple checksum */
    unsigned long long low = (unsigned long long)total_sum;
    unsigned long long high = (unsigned long long)(total_sum >> 64);
    
    printf("Checksum: 0x%016llx%016llx\n", high, low);
    
    return 0;
}
