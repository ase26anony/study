#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum function to prevent optimization */
static inline unsigned int checksum(const void *data, size_t len) {
    const unsigned char *bytes = (const unsigned char *)data;
    unsigned int sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum = (sum * 31) + bytes[i];
    }
    return sum;
}

/* Test 1: Narrowing conversions with boundary values */
__attribute__((noinline))
unsigned int test_narrowing_conversions(void) {
    unsigned int hash = 0;
    int32_t results[16];
    int idx = 0;
    
    /* Constants at boundaries */
    int64_t large_positive = 0x7FFFFFFF;  /* INT_MAX */
    int64_t large_negative = -0x80000000; /* INT_MIN */
    uint64_t large_unsigned = 0xFFFFFFFF;
    
    /* Narrowing with explicit casts */
    results[idx++] = (int32_t)large_positive;
    results[idx++] = (int32_t)large_negative;
    results[idx++] = (int32_t)(large_positive + 1);  /* Potential overflow */
    results[idx++] = (int32_t)(large_negative - 1);  /* Potential underflow */
    
    /* From unsigned with values beyond signed range */
    results[idx++] = (int32_t)large_unsigned;
    results[idx++] = (int32_t)(large_unsigned >> 1);
    
    /* Operations that require range analysis */
    int64_t a = 1000;
    int64_t b = 0x7FFFFFFF;
    results[idx++] = (int32_t)(a * b / 1000);  /* Should fit */
    results[idx++] = (int32_t)(b * b / 1000);  /* Might overflow 64-bit */
    
    /* Shifts that may overflow when narrowed */
    results[idx++] = (int32_t)(1ULL << 35);
    results[idx++] = (int32_t)(1ULL << 31);
    
    /* Boundary comparisons */
    int32_t x = 0x7FFFFFFF;
    int32_t y = -0x80000000;
    results[idx++] = (x > 0x7FFFFFFE) ? 1 : 0;
    results[idx++] = (y < -0x7FFFFFFF) ? 1 : 0;
    
    /* Complex boundary condition */
    int64_t val = 0x7FFFFFFFFFFFFFFFLL;
    results[idx++] = (int32_t)(val >> 32);
    results[idx++] = (int32_t)(val >> 31);
    results[idx++] = (int32_t)(val >> 33);
    
    sink = idx;
    return checksum(results, sizeof(results));
}

/* Test 2: Loop bound analysis with complex conditions */
__attribute__((noinline))
unsigned int test_loop_range_analysis(void) {
    unsigned int hash = 0;
    int32_t accum = 0;
    
    /* Outer loop with bitmasked bound */
    uint32_t outer_limit = 0x1234 & 0xFFF;  /* Constrained range */
    for (uint32_t i = 0; i < outer_limit; i++) {
        /* Inner loop with bound dependent on outer index */
        uint32_t inner_limit = (i | 0x7F) & 0xFF;  /* Range [127, 255] */
        for (uint32_t j = i & 0x3F; j < inner_limit; j += 3) {
            /* Complex condition using bitwise ops */
            if (((j ^ i) & 0x1F) > 16) {
                accum += j;
            } else {
                accum -= i;
            }
        }
        
        /* Another loop with shifting bound */
        int32_t shift_bound = (i << 2) | 0xF;
        for (int32_t k = -100; k < shift_bound && k < 1000; k++) {
            if (k > 0 && (k & (k - 1)) == 0) {  /* Power of two check */
                accum ^= k;
            }
        }
    }
    
    /* Loop with comparison at type boundary */
    int64_t big_start = -0x80000000LL;
    int64_t big_end = 0x7FFFFFFFLL;
    int32_t counter = 0;
    
    for (int64_t n = big_start; n < big_end && counter < 100; n += 0x10000000) {
        int32_t narrowed = (int32_t)n;
        if (narrowed > 0) {
            accum += narrowed;
        }
        counter++;
    }
    
    /* Nested loops with wrap-around conditions */
    uint8_t small = 0;
    for (int i = 0; i < 300; i++) {  /* Will wrap */
        small += 1;
        for (int j = 0; j < (small & 0x7); j++) {
            accum += j * small;
        }
    }
    
    sink = accum;
    return checksum(&accum, sizeof(accum));
}

/* Test 3: Saturation arithmetic */
__attribute__((noinline))
unsigned int test_saturation_arithmetic(void) {
    unsigned int hash = 0;
    int32_t results[8];
    int idx = 0;
    
    /* Manual saturation clamp */
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + (int64_t)b;
        if (result > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (result < -0x80000000) return -0x80000000;
        return (int32_t)result;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * (int64_t)b;
        if (result > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (result < -0x80000000) return -0x80000000;
        return (int32_t)result;
    }
    
    /* Test cases near boundaries */
    results[idx++] = sat_add(0x70000000, 0x10000000);  /* Would overflow */
    results[idx++] = sat_add(-0x70000000, -0x10000000); /* Would underflow */
    results[idx++] = sat_add(0x7FFFFFFF, 0);
    results[idx++] = sat_add(-0x80000000, 1);
    
    results[idx++] = sat_mul(0x10000, 0x10000);  /* 2^16 * 2^16 = 2^32 (overflow) */
    results[idx++] = sat_mul(-0x10000, 0x10000);
    results[idx++] = sat_mul(0x7FFFFFFF, 1);
    results[idx++] = sat_mul(0x7FFFFFFF, 2);  /* Definitely overflows */
    
    /* Fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.75r;
    _Accum a1 = 100.0k;
    _Accum a2 = 200.0k;
    
    /* These operations may trigger fixed-point range analysis */
    results[0] += (int32_t)(f1 * 1000);
    results[1] += (int32_t)(a1 + a2);
    #endif
    
    sink = idx;
    return checksum(results, sizeof(results));
}

/* Test 4: Bit-fields with range constraints */
__attribute__((noinline))
unsigned int test_bitfield_ranges(void) {
    unsigned int hash = 0;
    
    struct BitFields {
        unsigned int a : 3;   /* 0-7 */
        signed int b : 5;     /* -16 to 15 */
        unsigned int c : 12;  /* 0-4095 */
        signed int d : 20;    /* -524288 to 524287 */
    } bf;
    
    int32_t results[16];
    int idx = 0;
    
    /* Assign values at bit-field boundaries */
    bf.a = 7;    /* Max for 3 bits */
    bf.b = -16;  /* Min for 5-bit signed */
    bf.c = 4095; /* Max for 12 bits */
    bf.d = 524287; /* Max for 20-bit signed */
    
    results[idx++] = bf.a;
    results[idx++] = bf.b;
    results[idx++] = bf.c;
    results[idx++] = bf.d;
    
    /* Operations that might overflow bit-field */
    bf.a = 8;   /* Should wrap to 0 */
    bf.b = 16;  /* Should wrap to -16 */
    bf.c = 4096; /* Should wrap to 0 */
    bf.d = 524288; /* Should wrap to -524288 */
    
    results[idx++] = bf.a;
    results[idx++] = bf.b;
    results[idx++] = bf.c;
    results[idx++] = bf.d;
    
    /* Comparisons against bit-field capacity */
    unsigned int test_val = 10;
    if (test_val > 7) {  /* 7 is max for 3-bit unsigned */
        bf.a = 7;
    } else {
        bf.a = test_val;
    }
    
    signed int signed_test = 20;
    if (signed_test > 15) {  /* 15 is max for 5-bit signed */
        bf.b = 15;
    } else if (signed_test < -16) {
        bf.b = -16;
    } else {
        bf.b = signed_test;
    }
    
    results[idx++] = bf.a;
    results[idx++] = bf.b;
    
    /* Union with bit-field to test type punning */
    union {
        struct BitFields bf;
        uint32_t raw;
    } u;
    
    u.raw = 0xFFFFFFFF;
    results[idx++] = u.bf.a;  /* Should be 7 */
    results[idx++] = u.bf.b;  /* Should be -1 */
    results[idx++] = u.bf.c;  /* Should be 4095 */
    results[idx++] = u.bf.d;  /* Should be -1 */
    
    sink = idx;
    return checksum(results, sizeof(results));
}

/* Test 5: Overflow builtins with range analysis */
__attribute__((noinline))
unsigned int test_overflow_builtins(void) {
    unsigned int hash = 0;
    int32_t results[12];
    int idx = 0;
    int overflow;
    
    /* Basic overflow checks */
    int32_t a = 0x7FFFFFFF;
    int32_t b = 1;
    int32_t sum;
    
    overflow = __builtin_add_overflow(a, b, &sum);
    results[idx++] = overflow;
    results[idx++] = sum;
    
    a = -0x80000000;
    b = -1;
    overflow = __builtin_add_overflow(a, b, &sum);
    results[idx++] = overflow;
    results[idx++] = sum;
    
    /* Multiplication overflow */
    a = 0x10000;
    b = 0x10000;
    int32_t prod;
    overflow = __builtin_mul_overflow(a, b, &prod);
    results[idx++] = overflow;
    results[idx++] = prod;
    
    /* Subtraction overflow */
    a = -0x80000000;
    b = 1;
    int32_t diff;
    overflow = __builtin_sub_overflow(a, b, &diff);
    results[idx++] = overflow;
    results[idx++] = diff;
    
    /* Overflow in loops with constrained ranges */
    int32_t x = 100;
    for (int i = 0; i < 10; i++) {
        int32_t old_x = x;
        if (!__builtin_add_overflow(x, 0x70000000, &x)) {
            results[idx++] = x;
        } else {
            x = old_x;
            results[idx++] = -1;
        }
    }
    
    /* Complex overflow check with bitwise constraints */
    uint32_t u = 0x80000000;
    uint32_t v = 0x80000000;
    uint32_t u_sum;
    overflow = __builtin_uadd_overflow(u, v, &u_sum);
    results[idx++] = overflow;
    results[idx++] = (int32_t)u_sum;
    
    sink = idx;
    return checksum(results, sizeof(results));
}

/* Main function that runs all tests */
int main(void) {
    unsigned int final_hash = 0;
    
    final_hash ^= test_narrowing_conversions();
    final_hash ^= test_loop_range_analysis();
    final_hash ^= test_saturation_arithmetic();
    final_hash ^= test_bitfield_ranges();
    final_hash ^= test_overflow_builtins();
    
    /* Use the result to prevent optimization */
    sink = final_hash;
    
    printf("Test completed with hash: %u\n", final_hash);
    return 0;
}
