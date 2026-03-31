/* test_fixed_value.c - Test program for GCC fixed-value.cc coverage */
#include <stdio.h>
#include <limits.h>
#include <stdint.h>

/* Force compiler to perform range analysis on wide integers */
static volatile unsigned long long checksum = 0;

/* Helper to prevent dead code elimination */
static void accumulate(unsigned long long val) {
    checksum ^= val;
}

/* Test 128-bit arithmetic and comparisons */
void test_128bit_arithmetic(void) {
    /* Near-boundary 64-bit values */
    const long long max64 = LLONG_MAX;
    const long long min64 = LLONG_MIN;
    const unsigned long long umax64 = ULLONG_MAX;
    
    /* 128-bit arithmetic that may overflow */
    __int128 s128_a = (__int128)max64 * 3;  /* Will overflow signed 64-bit */
    __int128 s128_b = (__int128)min64 * 2;  /* Large negative */
    unsigned __int128 u128_a = (unsigned __int128)umax64 * 4;  /* Will overflow unsigned 64-bit */
    
    /* Store intermediate results to force evaluation */
    accumulate((unsigned long long)s128_a);
    accumulate((unsigned long long)(s128_a >> 64));
    accumulate((unsigned long long)s128_b);
    accumulate((unsigned long long)(s128_b >> 64));
    accumulate((unsigned long long)u128_a);
    accumulate((unsigned long long)(u128_a >> 64));
    
    /* Complex comparisons mimicking a_high.sgt(max_r) logic */
    const __int128 max128 = ((__int128)max64 << 64) | umax64;
    const __int128 min128 = ((__int128)min64 << 64) | 0ULL;
    
    /* These comparisons should trigger double-int range checks */
    if (s128_a > max128) {
        accumulate(0x1);
    }
    if (s128_b < min128) {
        accumulate(0x2);
    }
    if (u128_a > (unsigned __int128)max128) {
        accumulate(0x4);
    }
    
    /* Chain comparisons similar to uncovered pattern */
    __int128 test_val = ((__int128)max64 << 32) | 0xFFFFFFFFULL;
    if (test_val > max128 || (test_val == max128 && (unsigned __int128)test_val > (unsigned __int128)umax64)) {
        accumulate(0x8);
    }
}

/* Test loop bounds with wide integer calculations */
void test_loop_bounds(void) {
    /* Variables that force range analysis */
    long long base = LLONG_MAX / 4;
    int factor = 5;
    int step = 3;
    
    /* Loop with bound that may overflow 64-bit */
    for (long long i = 0; i < base * factor; i += step) {
        /* Inner computation with potential overflow */
        __int128 product = (__int128)i * i;
        if (product > (__int128)base * base * 16) {
            accumulate(i);
            break;  /* Prevent infinite loop */
        }
        if (i > 1000) break; /* Safety limit */
    }
    
    /* Nested loops with different integer types */
    for (int j = 0; j < 100; ++j) {
        for (long long k = LLONG_MAX - 1000; k < LLONG_MAX; ++k) {
            /* Comparison that needs double-int analysis */
            __int128 combined = (__int128)j * k;
            if (combined > ((__int128)LLONG_MAX << 32)) {
                accumulate(k);
                break;
            }
        }
    }
}

/* Test bit-field operations on wide integers */
void test_bitfield_ops(void) {
    unsigned __int128 wide_val = ((unsigned __int128)0xFEDCBA9876543210ULL << 64) | 0x0123456789ABCDEFULL;
    
    /* Variable shifts that force range analysis */
    for (int shift = 0; shift < 128; shift += 31) {
        unsigned __int128 shifted = wide_val >> shift;
        unsigned long long masked = (shifted >> 32) & 0xFFFFFFFFULL;
        
        /* Comparison that may trigger the uncovered logic */
        if (shifted > ((unsigned __int128)ULLONG_MAX << 64)) {
            accumulate(masked);
        }
        
        /* Extract bit-field with large shift */
        int bitfield = (shifted >> 60) & 0xF;
        accumulate(bitfield);
    }
    
    /* Combine 64-bit values into 128-bit for comparison */
    unsigned long long high = 0x8000000000000000ULL;
    unsigned long long low = 0xFFFFFFFFFFFFFFFFULL;
    unsigned __int128 combined = ((unsigned __int128)high << 64) | low;
    
    if (combined > (unsigned __int128)ULLONG_MAX) {
        accumulate(high);
    }
}

/* Test compiler built-ins for overflow detection */
void test_builtin_overflow(void) {
    long long a = LLONG_MAX / 2;
    long long b = 3;
    long long result;
    int overflow;
    
    /* These built-ins may invoke fixed-value logic */
    overflow = __builtin_mul_overflow(a, b, &result);
    if (overflow) {
        accumulate(0x10);
    }
    
    /* Test with 128-bit types if supported */
    __int128 c = (__int128)a * b;
    __int128 d = (__int128)LLONG_MAX;
    
    if (__builtin_add_overflow_p(c, d, (__int128)0)) {
        accumulate(0x20);
    }
}

/* Test boundary cases with explicit comparisons */
void test_boundary_comparisons(void) {
    /* Test cases designed to hit the exact uncovered pattern:
     * if (a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)))
     */
    
    /* Case 1: a_high > max_r */
    __int128 val1 = ((__int128)1 << 127) | 1;  /* High part = 1, low part = 1 */
    __int128 max_r_val = 0;  /* max_r.high = 0, max_r.low = 0 */
    
    if (val1 > max_r_val) {
        accumulate(0x40);
    }
    
    /* Case 2: a_high == max_r && a_low > max_s */
    /* max_s = (-1).zext(i_f_bits) - we need to create similar boundary */
    unsigned __int128 max_s_val = (unsigned __int128)-1;  /* All bits set */
    /* For i_f_bits = 64, max_s = 0xFFFFFFFFFFFFFFFF */
    max_s_val = max_s_val & ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL);
    
    __int128 val2 = 0;  /* a_high = 0 (equals max_r) */
    unsigned __int128 low_part = (unsigned __int128)0xFFFFFFFFFFFFFFFFULL + 1;  /* Exceeds max_s */
    
    /* Manually split for comparison */
    unsigned long long val2_high = (unsigned long long)(val2 >> 64);
    unsigned long long val2_low = (unsigned long long)val2;
    unsigned long long max_r_high = 0;
    unsigned long long max_s_low = (unsigned long long)max_s_val;
    
    if (val2_high > max_r_high || 
        (val2_high == max_r_high && val2_low > max_s_low)) {
        accumulate(0x80);
    }
    
    /* Test with negative values for signed comparisons */
    __int128 neg_val = ((__int128)-1 << 64) | 0x8000000000000000ULL;
    __int128 boundary = ((__int128)-1 << 63);
    
    if (neg_val > boundary) {
        accumulate(0x100);
    }
}

int main(void) {
    printf("Testing GCC fixed-value double-int comparison logic...\n");
    
    test_128bit_arithmetic();
    test_loop_bounds();
    test_bitfield_ops();
    test_builtin_overflow();
    test_boundary_comparisons();
    
    printf("Checksum: %llu\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
