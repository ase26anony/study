/* test_fixed_value.c
 * 
 * This program is designed to exercise GCC's double-int arithmetic
 * and range comparison logic, specifically targeting the uncovered
 * lines in fixed-value.cc that check if a high-part value exceeds
 * a maximum range or if it equals the maximum and the low-part
 * exceeds a secondary maximum.
 *
 * Compile with: gcc -O2 -fwrapv -fstrict-overflow -march=native -Wall -Wextra -Wpedantic test_fixed_value.c -o test_fixed_value
 */

#include <stdio.h>
#include <limits.h>
#include <stdint.h>

/* Use __int128 if available */
#ifdef __SIZEOF_INT128__
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;
#else
/* Fallback for compilers without __int128 */
typedef struct {
    uint64_t low;
    uint64_t high;
} int128_t;
typedef struct {
    uint64_t low;
    uint64_t high;
} uint128_t;
#endif

/* Helper to print 128-bit values (for debugging) */
void print_int128(int128_t val) {
#ifdef __SIZEOF_INT128__
    printf("0x%016llx%016llx", 
           (unsigned long long)((uint128_t)val >> 64),
           (unsigned long long)((uint128_t)val & 0xFFFFFFFFFFFFFFFFULL));
#else
    printf("0x%016llx%016llx", (unsigned long long)val.high, (unsigned long long)val.low);
#endif
}

int main(void) {
    volatile uint64_t checksum = 0; /* volatile to prevent dead code elimination */
    
    /* 1. Large Integer Arithmetic with Overflow/Underflow */
    printf("=== Large Integer Arithmetic ===\n");
    
    /* Multiplication that exceeds 64-bit range */
#ifdef __SIZEOF_INT128__
    int128_t x = (int128_t)LLONG_MAX * 4;
    checksum ^= (uint64_t)x ^ (uint64_t)(x >> 64);
    printf("LLONG_MAX * 4 = ");
    print_int128(x);
    printf("\n");
    
    /* Shift operations on maximum values */
    unsigned long long y = ~0ULL;
    unsigned long long y_shifted = y >> 3;  /* Force range analysis */
    checksum ^= y_shifted;
    printf("~0ULL >> 3 = 0x%016llx\n", y_shifted);
    
    /* Compile-time constants near boundaries */
    static const int128_t NEAR_MAX = (int128_t)LLONG_MAX * 2;
    static const int128_t NEAR_MIN = (int128_t)LLONG_MIN * 2;
    checksum ^= (uint64_t)NEAR_MAX ^ (uint64_t)(NEAR_MAX >> 64);
    checksum ^= (uint64_t)NEAR_MIN ^ (uint64_t)(NEAR_MIN >> 64);
    
    /* 2. Loop Bounds with Complex Exit Conditions */
    printf("\n=== Loop Bounds ===\n");
    
    long long limit = 1000;
    long long factor = LLONG_MAX / 1000;  /* Large factor */
    long long step = 1;
    
    /* Loop with exit condition that may overflow in analysis */
    for (long long i = 0; i < limit * factor; i += step) {
        if (i % 100000000 == 0) {
            checksum ^= i;
            printf("Loop i = %lld\n", i);
        }
        /* Break early to avoid infinite loop */
        if (i >= 1000000) break;
    }
    
    /* Nested loops with different types */
    int inner_limit = 100;
    for (long long outer = LLONG_MAX - 1000; outer < LLONG_MAX; outer++) {
        for (int inner = 0; inner < inner_limit; inner++) {
            checksum ^= outer ^ inner;
        }
        /* Break after reasonable iterations */
        if (outer > LLONG_MAX - 900) break;
    }
    
    /* 3. Conditional Branches Based on Wide Comparisons */
    printf("\n=== Wide Comparisons ===\n");
    
    int128_t a = (int128_t)LLONG_MAX * 3;
    int128_t b = (int128_t)LLONG_MAX * 2;
    
    /* Direct 128-bit comparison */
    if (a > b) {
        checksum ^= 0xAAAAAAAA;
        printf("a > b (both ~128-bit)\n");
    }
    
    /* Chained comparisons mimicking the uncovered logic */
    uint64_t a_high = (uint64_t)((uint128_t)a >> 64);
    uint64_t a_low = (uint64_t)a;
    uint64_t max_r = 0;
    uint64_t max_s = ~0ULL;
    
    /* This directly exercises the double-int comparison pattern:
     * if (a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))) */
    if ((int64_t)a_high > (int64_t)max_r || 
        (a_high == max_r && a_low > max_s)) {
        checksum ^= 0xBBBBBBBB;
        printf("High/low comparison triggered\n");
    }
    
    /* Unsigned comparisons with wrap-around */
    uint128_t uval = (uint128_t)~0ULL;
    uint128_t uval2 = uval + 100;
    
    if (uval2 > uval) {
        checksum ^= 0xCCCCCCCC;
        printf("Unsigned 128-bit wrap comparison\n");
    }
    
    /* 4. Bit-Field Operations and Masking */
    printf("\n=== Bit-Field Operations ===\n");
    
    uint128_t wide_val = ((uint128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    int shift = 60;
    uint64_t mask = 0xFFFFFFFFULL;
    
    uint64_t field = (uint64_t)((wide_val >> shift) & mask);
    checksum ^= field;
    printf("Extracted field = 0x%016llx\n", (unsigned long long)field);
    
    /* Combine 64-bit values into 128-bit */
    uint64_t high_part = 0x5555555555555555ULL;
    uint64_t low_part = 0xAAAAAAAAAAAAAAAAULL;
    uint128_t combined = ((uint128_t)high_part << 64) | low_part;
    
    if (combined > wide_val) {
        checksum ^= 0xDDDDDDDD;
        printf("Combined value > wide_val\n");
    }
    
    /* 5. Compiler Built-ins for Wide Arithmetic */
    printf("\n=== Compiler Built-ins ===\n");
    
    long long l1 = LLONG_MAX;
    long long l2 = 2;
    long long l_result;
    int overflow;
    
    /* Overflow checking built-ins */
    overflow = __builtin_mul_overflow(l1, l2, &l_result);
    checksum ^= l_result ^ overflow;
    printf("mul_overflow(LLONG_MAX, 2) = %lld, overflow = %d\n", l_result, overflow);
    
    /* Count leading zeros on computed values */
    unsigned long long clz_val = (unsigned long long)LLONG_MAX << 2;
    int clz = __builtin_clzll(clz_val);
    checksum ^= clz;
    printf("clzll(LLONG_MAX << 2) = %d\n", clz);
    
    /* 128-bit division with non-constant divisor */
#ifdef __SIZEOF_INT128__
    int128_t dividend = (int128_t)LLONG_MAX << 10;
    int128_t divisor = (int128_t)LLONG_MAX >> 10;
    int128_t quotient = dividend / divisor;
    checksum ^= (uint64_t)quotient ^ (uint64_t)(quotient >> 64);
    printf("128-bit division result = ");
    print_int128(quotient);
    printf("\n");
#endif
    
    /* Final checksum output to prevent optimization */
    printf("\n=== Final Checksum ===\n");
    printf("checksum = 0x%016llx\n", (unsigned long long)checksum);
    
    return 0;
}
