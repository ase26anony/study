#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to maintain function boundaries for coverage */
#define NOINLINE __attribute__((noinline))

/* Fixed-point types if available */
#ifdef __STDC_IEC_559__
#include <stdfix.h>
#endif

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

/* Test 1: Narrowing conversions with range boundaries */
NOINLINE unsigned test_narrowing_conversions(void) {
    unsigned result = 0;
    
    /* Constants at type boundaries */
    int64_t large_positive = 0x7FFFFFFFFFFFFFFFLL;
    int64_t large_negative = 0x8000000000000000LL;
    uint64_t large_unsigned = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Narrowing conversions that require range analysis */
    int32_t narrow1 = (int32_t)large_positive;  /* Should trigger range check */
    int32_t narrow2 = (int32_t)large_negative;  /* Should trigger range check */
    uint32_t narrow3 = (uint32_t)large_unsigned; /* Should trigger range check */
    
    /* Arithmetic operations before narrowing */
    int64_t a = 0x123456789ABCDEF0LL;
    int64_t b = 0x0FEDCBA987654321LL;
    int64_t sum = a + b;
    int32_t narrow_sum = (int32_t)sum;  /* Requires overflow/range analysis */
    
    /* Shifts that may overflow */
    int64_t shifted = a << 5;
    int32_t narrow_shifted = (int32_t)shifted;
    
    /* Comparisons against boundaries */
    if (narrow1 > 0x7FFFFFFF - 100) result |= 1;
    if (narrow2 < -0x7FFFFFFF + 100) result |= 2;
    if (narrow3 > 0xFFFFFFFF - 1000) result |= 4;
    
    checksum += result + narrow_sum + narrow_shifted;
    return result;
}

/* Test 2: Complex loop bound analysis */
NOINLINE unsigned test_loop_range_analysis(void) {
    unsigned result = 0;
    volatile int seed = 42; /* volatile to prevent constant propagation */
    
    /* Outer loop with bitwise-derived bounds */
    for (int32_t i = seed & 0xFFF; i < (seed | 0x7FF); i += (seed & 0x3F) + 1) {
        int32_t inner_bound = (i ^ 0xABCD) & 0xFFF;
        
        /* Inner loop with dependent bounds */
        for (int32_t j = i & 0xFF; j < inner_bound; j += (i & 0x1F) + 1) {
            /* Complex condition using bitwise ops */
            if ((j & 0xF0F) == (i & 0xF0F)) {
                result ^= j;
            }
            
            /* Comparison against shifted value */
            int32_t shifted_j = j << (i & 0x7);
            if (shifted_j > 0x7FFFFFFF - 1000) {
                result += 1;
            }
        }
        
        /* Loop with modulo operation in bound */
        int32_t k = i;
        while (k < ((i * 3) & 0xFFFF)) {
            result += k & 1;
            k += (i % 17) + 1;
        }
    }
    
    /* Nested loops with pointer arithmetic */
    int32_t arr[256];
    for (int i = 0; i < 256; i++) arr[i] = i;
    
    for (int32_t i = 0; i < 128; i++) {
        int32_t *ptr = arr + i;
        int32_t limit = (i * 2) & 0xFF;
        
        for (int32_t j = 0; j < limit; j++) {
            ptr[j] = (ptr[j] * 3) & 0x3FF;
            result += ptr[j];
        }
    }
    
    checksum += result;
    return result;
}

/* Test 3: Saturation arithmetic */
NOINLINE unsigned test_saturation_arithmetic(void) {
    unsigned result = 0;
    
    /* Manual saturation implementation */
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t tmp = (int64_t)a + (int64_t)b;
        
        /* These comparisons should trigger the uncovered range logic */
        if (tmp > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (tmp < -0x7FFFFFFF - 1) return -0x7FFFFFFF - 1;
        return (int32_t)tmp;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t tmp = (int64_t)a * (int64_t)b;
        
        /* Boundary comparisons */
        if (tmp > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (tmp < -0x7FFFFFFF - 1) return -0x7FFFFFFF - 1;
        return (int32_t)tmp;
    }
    
    /* Test cases with boundary values */
    int32_t test_cases[][2] = {
        {0x7FFFFFFF, 1},
        {0x7FFFFFFF, -1},
        {-0x7FFFFFFF - 1, -1},
        {0x40000000, 2},
        {0x3FFFFFFF, 2},
        {0, 0}
    };
    
    for (int i = 0; i < 6; i++) {
        result ^= sat_add(test_cases[i][0], test_cases[i][1]);
        result ^= sat_mul(test_cases[i][0], test_cases[i][1]);
    }
    
#ifdef __STDC_IEC_559__
    /* Fixed-point arithmetic if supported */
    _Accum acc1 = 0.5k;
    _Accum acc2 = 0.7k;
    _Accum acc_sum = acc1 + acc2;
    
    /* Comparisons that may trigger fixed-point range analysis */
    if (acc_sum > 1.0k) result += 1;
    if (acc_sum < 0.0k) result += 2;
#endif
    
    checksum += result;
    return result;
}

/* Test 4: Bit-field range analysis */
NOINLINE unsigned test_bitfield_ranges(void) {
    unsigned result = 0;
    
    /* Struct with various bit-fields */
    struct BitFields {
        unsigned int a : 3;  /* 0-7 */
        signed int b : 5;    /* -16 to 15 */
        unsigned int c : 8;  /* 0-255 */
        signed int d : 12;   /* -2048 to 2047 */
        unsigned int e : 1;  /* 0-1 */
    } bf;
    
    /* Assign boundary values */
    bf.a = 7;      /* Max for 3 bits */
    bf.b = -16;    /* Min for 5 bits signed */
    bf.c = 255;    /* Max for 8 bits */
    bf.d = 2047;   /* Max for 12 bits signed */
    bf.e = 1;      /* Max for 1 bit */
    
    /* Comparisons that should trigger range analysis */
    if (bf.a == 7) result |= 0x01;
    if (bf.b == -16) result |= 0x02;
    if (bf.c > 200) result |= 0x04;
    if (bf.d < 2048) result |= 0x08;  /* Always true, but needs range check */
    if (bf.e != 0) result |= 0x10;
    
    /* Arithmetic on bit-fields */
    unsigned int temp = bf.a + bf.c;
    if (temp > 300) result |= 0x20;
    
    /* Union with bit-fields and regular ints */
    union BitUnion {
        struct BitFields bits;
        uint32_t full;
    } u;
    
    u.full = 0xDEADBEEF;
    if (u.bits.c > 100 && u.bits.c < 200) result |= 0x40;
    
    checksum += result + u.bits.a + u.bits.b;
    return result;
}

/* Test 5: Overflow builtins with range analysis */
NOINLINE unsigned test_overflow_builtins(void) {
    unsigned result = 0;
    volatile int v = 100;
    
    /* Basic overflow checks */
    int32_t a = 0x70000000;
    int32_t b = 0x10000000;
    int32_t sum;
    
    if (__builtin_add_overflow(a, b, &sum)) {
        result |= 0x01;
    }
    
    /* Multiplication with partially known ranges */
    int32_t x = v & 0xFFFF;
    int32_t y = (v >> 16) & 0xFFFF;
    int32_t prod;
    
    /* x and y are in range 0-65535, but compiler may not know exact values */
    if (__builtin_mul_overflow(x, y, &prod)) {
        result |= 0x02;
    }
    
    /* Chained operations with overflow checking */
    int32_t vals[4] = {1000, 2000, 3000, 4000};
    int32_t total = 0;
    int overflow = 0;
    
    for (int i = 0; i < 4; i++) {
        if (__builtin_add_overflow(total, vals[i], &total)) {
            overflow = 1;
        }
    }
    
    if (overflow) result |= 0x04;
    
    /* Subtraction with boundary values */
    int32_t min_val = -0x7FFFFFFF - 1;
    int32_t sub_result;
    
    if (__builtin_sub_overflow(min_val, 1, &sub_result)) {
        result |= 0x08;
    }
    
    /* Complex expression with overflow check */
    int32_t expr1 = (v * 3) & 0x3FF;
    int32_t expr2 = (v * 5) & 0x7FF;
    int32_t final_result;
    
    if (__builtin_add_overflow(expr1, expr2, &final_result)) {
        result |= 0x10;
    }
    
    /* Use the results to prevent elimination */
    checksum += result + sum + prod + total + sub_result + final_result;
    return result;
}

/* Main function that runs all tests */
int main(void) {
    unsigned total_result = 0;
    
    printf("Running integer range analysis tests...\n");
    
    total_result ^= test_narrowing_conversions();
    total_result ^= test_loop_range_analysis();
    total_result ^= test_saturation_arithmetic();
    total_result ^= test_bitfield_ranges();
    total_result ^= test_overflow_builtins();
    
    printf("Total checksum: %u\n", checksum);
    printf("Test result: 0x%08X\n", total_result);
    
    return (total_result == 0) ? 0 : 1;
}
