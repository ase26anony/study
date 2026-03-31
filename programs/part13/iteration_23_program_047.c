/* test_fixed_value.c - Test program for GCC fixed-value.cc coverage */

#include <stdio.h>
#include <limits.h>
#include <stdint.h>

/* Force compiler to consider all code paths */
static volatile int sink;

/* Helper to prevent optimization */
static void use(void *p) {
    sink = *(int*)p;
}

/* Test large integer arithmetic that requires double-int representation */
void test_large_integer_arithmetic(void) {
    /* 128-bit constants near boundaries */
    __int128 signed_max = ((__int128)LLONG_MAX << 32) | 0xFFFFFFFF;
    __int128 signed_min = ((__int128)LLONG_MIN << 32);
    unsigned __int128 unsigned_max = ((unsigned __int128)ULLONG_MAX << 64) | ULLONG_MAX;
    
    /* Multiplication that can overflow 64-bit */
    long long a = LLONG_MAX / 2;
    long long b = 3;
    __int128 prod_signed = (__int128)a * b;  /* Should trigger range analysis */
    
    /* Shift operations with large values */
    unsigned long long shift_val = ULLONG_MAX;
    int shift_amount = 1;
    unsigned __int128 shifted = (unsigned __int128)shift_val << shift_amount;
    
    /* Comparisons that mimic a_high.sgt(max_r) logic */
    if (prod_signed > signed_max) {
        sink = 1;
    }
    
    if (shifted > unsigned_max) {
        sink = 2;
    }
    
    /* Chain comparisons similar to uncovered pattern */
    __int128 test_val = ((__int128)0x7FFFFFFFFFFFFFFFLL) * 4;
    __int128 boundary = ((__int128)0x3FFFFFFFFFFFFFFFLL) << 32;
    
    if (test_val > boundary || (test_val == boundary && (unsigned __int128)test_val > 0)) {
        sink = 3;
    }
}

/* Loop with complex exit conditions requiring range analysis */
void test_loop_bounds(void) {
    long long limit = LLONG_MAX / 4;
    int factor = 5;
    long long step = LLONG_MAX / 16;
    
    /* Loop where exit condition involves multiplication */
    for (long long i = 0; i < limit * factor; i += step) {
        /* Nested loop with different type */
        for (int j = 0; j < 10; j++) {
            /* Arithmetic that may overflow */
            long long temp = i * j;
            if (temp > LLONG_MAX / 2) {
                sink = (int)temp;
                break;
            }
        }
        if (i > LLONG_MAX / 8) break;
    }
    
    /* Another loop with unsigned 128-bit comparison */
    unsigned __int128 ustart = 0;
    unsigned __int128 ulimit = ((unsigned __int128)ULLONG_MAX << 32);
    
    for (unsigned __int128 k = ustart; k < ulimit; k += (ULLONG_MAX / 256)) {
        if (k > ((unsigned __int128)ULLONG_MAX << 16)) {
            sink = (int)k;
            break;
        }
    }
}

/* Conditional branches based on wide comparisons */
void test_wide_comparisons(void) {
    /* 128-bit multiplication and comparison */
    long long x = LLONG_MAX - 1000;
    long long y = 2;
    __int128 wide_prod = (__int128)x * y;
    
    /* Direct comparison similar to uncovered code */
    if (wide_prod > ((__int128)LLONG_MAX << 32)) {
        sink = 4;
    } else if (wide_prod == ((__int128)LLONG_MAX << 32) && 
               (unsigned __int128)wide_prod > ULLONG_MAX) {
        sink = 5;
    }
    
    /* Unsigned comparisons with wrap-around */
    unsigned __int128 uval = (unsigned __int128)ULLONG_MAX * 2;
    if (uval > 0xFFFFFFFFFFFFFFFFULL) {
        sink = 6;
    }
    
    /* Ternary operator with wide comparison */
    int result = (wide_prod > 0) ? 1 : 0;
    use(&result);
}

/* Bit-field operations requiring double-int analysis */
void test_bitfield_operations(void) {
    unsigned __int128 val = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 
                            0xFEDCBA9876543210ULL;
    
    /* Extract bit-fields with large shifts */
    int shift = 60;
    unsigned long long mask = 0xFFFFFFFFULL;
    unsigned __int128 field = (val >> shift) & mask;
    
    if (field > 0x80000000ULL) {
        sink = 7;
    }
    
    /* Combine 64-bit values into 128-bit for comparison */
    unsigned long long high = ULLONG_MAX;
    unsigned long long low = 1;
    unsigned __int128 combined = ((unsigned __int128)high << 64) | low;
    
    if (combined > ((unsigned __int128)ULLONG_MAX << 63)) {
        sink = 8;
    }
}

/* Use GCC built-ins that may invoke fixed-value logic */
void test_builtins(void) {
    long long a = LLONG_MAX;
    long long b = 2;
    long long res;
    
    /* Overflow checking builtins */
    if (__builtin_mul_overflow(a, b, &res)) {
        sink = 9;
    }
    
    /* Count leading zeros on computed values */
    unsigned long long x = ULLONG_MAX >> 32;
    int clz = __builtin_clzll(x);
    use(&clz);
    
    /* 128-bit division with variable divisor */
    __int128 dividend = ((__int128)LLONG_MAX << 32);
    __int128 divisor = 3;
    __int128 quotient = dividend / divisor;
    
    if (quotient > (LLONG_MAX << 16)) {
        sink = 10;
    }
}

/* Main function that exercises all test cases */
int main(void) {
    int checksum = 0;
    
    test_large_integer_arithmetic();
    checksum ^= sink;
    
    test_loop_bounds();
    checksum ^= sink;
    
    test_wide_comparisons();
    checksum ^= sink;
    
    test_bitfield_operations();
    checksum ^= sink;
    
    test_builtins();
    checksum ^= sink;
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
