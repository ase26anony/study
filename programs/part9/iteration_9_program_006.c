#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum function to prevent optimization */
__attribute__((noinline)) 
uint32_t compute_checksum(uint32_t *data, size_t len) {
    uint32_t hash = 0x811c9dc5;
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 0x01000193;
    }
    return hash;
}

/* Test 1: Narrowing conversions with boundary values */
__attribute__((noinline))
uint32_t test_narrowing_conversions(void) {
    uint32_t results[16] = {0};
    int idx = 0;
    
    /* Constants at type boundaries */
    int64_t large_positive = 0x7FFFFFFF;
    int64_t large_negative = -0x80000000;
    uint64_t huge_unsigned = 0xFFFFFFFFULL;
    
    /* Narrowing conversions that require range analysis */
    int32_t narrow1 = (int32_t)large_positive;  /* Exactly at INT32_MAX */
    int32_t narrow2 = (int32_t)large_negative;  /* Exactly at INT32_MIN */
    uint32_t narrow3 = (uint32_t)huge_unsigned; /* Exactly at UINT32_MAX */
    
    /* Operations that may overflow */
    int64_t sum = large_positive + 1;
    int32_t narrow4 = (int32_t)sum;  /* Potential overflow */
    
    /* Shifts that require range checking */
    uint32_t x = 0x80000000U;
    uint32_t shifted = x >> 31;  /* Shift to boundary */
    int32_t signed_shift = (int32_t)x >> 31;
    
    results[idx++] = narrow1;
    results[idx++] = narrow2;
    results[idx++] = narrow3;
    results[idx++] = narrow4;
    results[idx++] = shifted;
    results[idx++] = signed_shift;
    
    /* Comparisons against boundaries */
    if (narrow1 > 0x7FFFFFFE) results[idx++] = 1;
    if (narrow2 < -0x7FFFFFFF) results[idx++] = 2;
    if (narrow3 == 0xFFFFFFFF) results[idx++] = 3;
    
    return compute_checksum(results, idx);
}

/* Test 2: Complex loop bound analysis */
__attribute__((noinline))
uint32_t test_loop_range_analysis(void) {
    uint32_t results[32] = {0};
    int idx = 0;
    
    /* Variables with constrained ranges */
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    int32_t c = 100;
    
    /* Loop with bitwise operation in bounds */
    uint32_t checksum = 0;
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += (c & 0x3F)) {
        checksum ^= i;
        if (i > 0xFFFFFFFF - 1000) break;  /* Boundary check */
        results[idx++ % 32] = i;
    }
    results[0] = checksum;
    
    /* Nested loops with dependent bounds */
    int32_t outer_max = 50;
    for (int32_t j = -100; j < outer_max; j += 3) {
        /* Inner loop bound depends on outer index */
        int32_t inner_bound = (j & 0x1F) + 10;
        for (int32_t k = 0; k < inner_bound; k++) {
            int32_t val = j * k;
            if (val > 10000 || val < -10000) {
                results[(j + 100) % 32] = val;
            }
        }
    }
    
    /* Loop with shifting bound */
    uint32_t mask = 0xFFFF;
    for (uint32_t i = 0; i < (mask << 4); i += (mask >> 8)) {
        results[i % 32] ^= i;
        if (i > 0xFFFFFFF0) break;
    }
    
    return compute_checksum(results, 32);
}

/* Test 3: Saturation arithmetic */
__attribute__((noinline))
uint32_t test_saturation_arithmetic(void) {
    uint32_t results[16] = {0};
    int idx = 0;
    
    /* Manual saturation implementation */
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t sum = (int64_t)a + (int64_t)b;
        if (sum > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (sum < -0x80000000) return -0x80000000;
        return (int32_t)sum;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t prod = (int64_t)a * (int64_t)b;
        if (prod > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (prod < -0x80000000) return -0x80000000;
        return (int32_t)prod;
    }
    
    /* Test cases near boundaries */
    int32_t test_cases[][2] = {
        {0x7FFFFFF0, 100},      /* Near overflow */
        {-0x7FFFFFF0, -100},    /* Near underflow */
        {0x40000000, 2},        /* Exact boundary */
        {-0x40000000, 2},       /* Negative boundary */
        {0x1000, 0x1000},       /* Safe multiplication */
    };
    
    for (int i = 0; i < 5; i++) {
        results[idx++] = sat_add(test_cases[i][0], test_cases[i][1]);
        results[idx++] = sat_mul(test_cases[i][0], test_cases[i][1]);
    }
    
    /* Fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.75r;
    _Fract fsum = f1 + f2;
    results[idx++] = *(uint32_t*)&fsum;
    #endif
    
    return compute_checksum(results, idx);
}

/* Test 4: Bit-field range analysis */
__attribute__((noinline))
uint32_t test_bitfield_ranges(void) {
    uint32_t results[16] = {0};
    int idx = 0;
    
    /* Struct with various bit-fields */
    struct BitFields {
        unsigned int small : 3;    /* 0-7 */
        signed int signed_small : 4; /* -8 to 7 */
        unsigned int medium : 10;  /* 0-1023 */
        unsigned int large : 31;   /* 0-2147483647 */
    } bf;
    
    /* Assignments that test boundaries */
    bf.small = 7;      /* Max for 3 bits */
    bf.signed_small = -8; /* Min for signed 4 bits */
    bf.medium = 1023;  /* Max for 10 bits */
    bf.large = 0x7FFFFFFF >> 1; /* Large value */
    
    /* Comparisons against bit-field capacities */
    if (bf.small == 7) results[idx++] = 1;
    if (bf.signed_small < 0) results[idx++] = 2;
    if (bf.medium > 1000) results[idx++] = 3;
    if (bf.large < 0x40000000) results[idx++] = 4;
    
    /* Union with overlapping fields */
    union Overlap {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } parts;
        uint32_t whole;
    } u;
    
    u.whole = 0x12345678;
    if (u.parts.low == 0x5678) results[idx++] = 5;
    if (u.parts.high == 0x1234) results[idx++] = 6;
    
    /* Bit-field in conditional */
    unsigned int value = 15;
    bf.small = value & 0x7;  /* Constrained to 3 bits */
    if (bf.small > 6) results[idx++] = 7;
    
    return compute_checksum(results, idx);
}

/* Test 5: Overflow builtins with range analysis */
__attribute__((noinline))
uint32_t test_overflow_builtins(void) {
    uint32_t results[16] = {0};
    int idx = 0;
    int overflow;
    
    /* Variables with partially known ranges */
    int32_t x = 100;
    int32_t y = 200;
    
    /* Range-restricting conditions */
    if (x > 0 && x < 1000) {
        if (y > 0 && y < 1000) {
            /* Compiler knows both are positive and bounded */
            overflow = __builtin_add_overflow(x, y, &results[idx++]);
            results[idx++] = overflow;
        }
    }
    
    /* Near boundary values */
    int32_t near_max = 0x7FFFFFF0;
    int32_t increment = 100;
    
    overflow = __builtin_add_overflow(near_max, increment, &results[idx++]);
    results[idx++] = overflow;
    
    /* Multiplication with range constraints */
    int32_t a = 1000;
    int32_t b = 2000;
    
    if (a < 10000 && b < 10000) {
        overflow = __builtin_mul_overflow(a, b, &results[idx++]);
        results[idx++] = overflow;
    }
    
    /* Chained operations */
    int32_t base = 0x40000000;
    for (int i = 0; i < 8; i++) {
        int32_t temp;
        overflow = __builtin_add_overflow(base, i * 0x1000000, &temp);
        results[idx++ % 16] = temp;
        results[idx++ % 16] = overflow;
    }
    
    /* Unsigned overflow checks */
    uint32_t u1 = 0xFFFFFF00;
    uint32_t u2 = 0x00000100;
    uint32_t u_result;
    
    overflow = __builtin_add_overflow(u1, u2, &u_result);
    results[idx++ % 16] = u_result;
    results[idx++ % 16] = overflow;
    
    return compute_checksum(results, 16);
}

/* Main function that runs all tests */
int main(void) {
    uint32_t checksums[5];
    
    checksums[0] = test_narrowing_conversions();
    checksums[1] = test_loop_range_analysis();
    checksums[2] = test_saturation_arithmetic();
    checksums[3] = test_bitfield_ranges();
    checksums[4] = test_overflow_builtins();
    
    /* Final checksum to prevent optimization */
    uint32_t final = 0;
    for (int i = 0; i < 5; i++) {
        final ^= checksums[i];
        printf("Test %d checksum: 0x%08x\n", i, checksums[i]);
    }
    
    sink = final;  /* Use result to prevent dead code elimination */
    return (final != 0) ? 0 : 1;
}
