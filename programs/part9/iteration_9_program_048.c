#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum function to prevent optimization */
__attribute__((noinline)) 
static uint32_t compute_checksum(const void* data, size_t size) {
    uint32_t hash = 0x811c9dc5;
    const uint8_t* bytes = (const uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        hash ^= bytes[i];
        hash *= 0x01000193;
    }
    return hash;
}

/* ========== Test 1: Narrowing Conversions ========== */
__attribute__((noinline))
static uint32_t test_narrowing_conversions(void) {
    uint32_t checksum = 0;
    uint64_t results[16];
    int idx = 0;
    
    /* Constants at boundary values */
    int64_t large_val = 0x7FFFFFFFFFFFFFFFLL;
    int64_t small_val = 0x8000000000000000LL;
    
    /* Test 1: Direct narrowing with boundary values */
    results[idx++] = (int32_t)large_val;      /* Should be -1 */
    results[idx++] = (int32_t)small_val;      /* Implementation-defined */
    results[idx++] = (uint32_t)(large_val >> 32);
    results[idx++] = (uint32_t)(small_val >> 32);
    
    /* Test 2: Arithmetic then narrowing */
    int64_t a = 0x123456789ABCDEF0LL;
    int64_t b = 0x0FEDCBA987654321LL;
    results[idx++] = (int32_t)(a + b);
    results[idx++] = (int32_t)(a - b);
    results[idx++] = (uint32_t)(a * 2);
    results[idx++] = (uint32_t)(b * 3);
    
    /* Test 3: Shifts that may overflow when narrowed */
    uint64_t x = 0x8000000000000000ULL;
    results[idx++] = (uint32_t)(x >> 31);
    results[idx++] = (uint32_t)(x >> 32);
    results[idx++] = (uint32_t)(x >> 33);
    
    /* Test 4: Comparisons after narrowing */
    int32_t narrow1 = (int32_t)large_val;
    int32_t narrow2 = (int32_t)(large_val - 1);
    results[idx++] = narrow1 > narrow2;
    results[idx++] = narrow1 < 0;
    results[idx++] = narrow2 > 0;
    
    /* Test 5: Boundary case with masking */
    uint64_t masked = 0xFFFFFFFF00000000ULL;
    results[idx++] = (uint32_t)masked;
    results[idx++] = (uint32_t)(masked | 0x80000000);
    
    checksum = compute_checksum(results, sizeof(results));
    sink = checksum;
    return checksum;
}

/* ========== Test 2: Loop Range Analysis ========== */
__attribute__((noinline))
static uint32_t test_loop_range_analysis(void) {
    uint32_t checksum = 0;
    uint32_t results[32] = {0};
    int idx = 0;
    
    /* Complex loop bounds with bitwise operations */
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    uint32_t c = 0x55555555;
    
    /* Loop 1: Lower bound from masking, upper bound from OR */
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += (c & 0xFF) + 1) {
        if (i < 32) results[i % 32] ^= i;
        if (i > 1000) break; /* Prevent infinite loops */
    }
    
    /* Loop 2: Nested loops with dependent bounds */
    for (int32_t outer = -100; outer < 100; outer += 37) {
        int32_t start = outer & 0x3F;
        int32_t end = (outer | 0x1F) + 50;
        
        /* Inner loop with range-dependent increment */
        for (int32_t inner = start; inner < end; inner += (outer & 0x7) + 1) {
            if (inner >= 0 && inner < 32) {
                results[inner] += outer ^ inner;
            }
            if (inner > 200) break;
        }
    }
    
    /* Loop 3: Bitwise increment pattern */
    uint32_t mask = 0x00FFFFFF;
    for (uint32_t i = 0; i < mask; i = (i + 1) & mask) {
        results[i % 32] += i & 0xFF;
        if (i > 1000000) break;
    }
    
    /* Loop 4: Reverse counting with shift */
    for (int32_t j = 1000; j > 0; j -= (j & 0x3F) + 1) {
        int32_t k = j >> 2;
        if (k >= 0 && k < 32) {
            results[k] ^= j;
        }
    }
    
    checksum = compute_checksum(results, sizeof(results));
    sink = checksum;
    return checksum;
}

/* ========== Test 3: Saturation Arithmetic ========== */
__attribute__((noinline))
static uint32_t test_saturation_arithmetic(void) {
    uint32_t checksum = 0;
    int32_t results[16];
    int idx = 0;
    
    /* Manual saturation functions */
    static int32_t sat_add(int32_t a, int32_t b) {
        int32_t sum = a + b;
        if ((b > 0) && (a > INT32_MAX - b)) return INT32_MAX;
        if ((b < 0) && (a < INT32_MIN - b)) return INT32_MIN;
        return sum;
    }
    
    static int32_t sat_mul(int32_t a, int32_t b) {
        int64_t product = (int64_t)a * (int64_t)b;
        if (product > INT32_MAX) return INT32_MAX;
        if (product < INT32_MIN) return INT32_MIN;
        return (int32_t)product;
    }
    
    /* Test saturation at boundaries */
    results[idx++] = sat_add(INT32_MAX, 1);
    results[idx++] = sat_add(INT32_MAX, -1);
    results[idx++] = sat_add(INT32_MIN, -1);
    results[idx++] = sat_add(INT32_MIN, 1);
    
    results[idx++] = sat_mul(INT32_MAX, 2);
    results[idx++] = sat_mul(INT32_MAX, 1);
    results[idx++] = sat_mul(INT32_MIN, 2);
    results[idx++] = sat_mul(INT32_MIN, -1);
    
    /* Test with values near boundaries */
    results[idx++] = sat_add(INT32_MAX - 100, 200);
    results[idx++] = sat_add(INT32_MIN + 100, -200);
    results[idx++] = sat_mul(46340, 46340);  /* sqrt(2^31-1) approx */
    results[idx++] = sat_mul(-46340, 46340);
    
    /* Chain of saturating operations */
    int32_t val = 1000;
    for (int i = 0; i < 10; i++) {
        val = sat_add(val, INT32_MAX / 20);
        results[idx++] = val;
    }
    
    checksum = compute_checksum(results, sizeof(results));
    sink = checksum;
    return checksum;
}

/* ========== Test 4: Bitfield Ranges ========== */
__attribute__((noinline))
static uint32_t test_bitfield_ranges(void) {
    uint32_t checksum = 0;
    uint32_t results[16] = {0};
    int idx = 0;
    
    /* Struct with various bitfield sizes */
    struct BitfieldStruct {
        signed int small : 4;
        unsigned int medium : 10;
        signed int large : 20;
        unsigned int full : 32;
    } bs;
    
    /* Union to test bitfield vs regular int */
    union BitfieldUnion {
        struct {
            signed int a : 5;
            unsigned int b : 7;
            signed int c : 12;
        } bits;
        uint32_t word;
    } bu;
    
    /* Test 1: Assign boundary values to bitfields */
    bs.small = 7;      /* Max for signed 4-bit */
    bs.small = -8;     /* Min for signed 4-bit */
    results[idx++] = bs.small;
    
    bs.medium = 0x3FF; /* Max for unsigned 10-bit */
    results[idx++] = bs.medium;
    
    bs.large = 0x7FFFF; /* Near max for signed 20-bit */
    bs.large = -0x80000; /* Near min for signed 20-bit */
    results[idx++] = bs.large;
    
    /* Test 2: Comparisons with bitfield limits */
    int test_val = 15;
    bs.small = 3;
    results[idx++] = (bs.small > 2);
    results[idx++] = (bs.small < -7);
    results[idx++] = (bs.medium >= 0x3FE);
    results[idx++] = (bs.large <= 0x7FFE);
    
    /* Test 3: Bitfield in conditional expressions */
    for (int i = -10; i < 10; i++) {
        bs.small = i;
        if (bs.small > 3 || bs.small < -4) {
            results[idx % 16] ^= i;
        }
        idx++;
    }
    
    /* Test 4: Union bitfield manipulation */
    bu.word = 0;
    bu.bits.a = -8;  /* Min for signed 5-bit */
    bu.bits.b = 0x7F; /* Max for unsigned 7-bit */
    bu.bits.c = 0x7FF; /* Max for signed 12-bit */
    results[15] = bu.word;
    
    checksum = compute_checksum(results, sizeof(results));
    sink = checksum;
    return checksum;
}

/* ========== Test 5: Overflow Builtins ========== */
__attribute__((noinline))
static uint32_t test_overflow_builtins(void) {
    uint32_t checksum = 0;
    uint32_t results[24];
    int idx = 0;
    int overflow;
    
    /* Test 1: Basic overflow checks */
    int32_t a = INT32_MAX;
    int32_t b = 1;
    results[idx++] = __builtin_add_overflow(a, b, &a);
    results[idx++] = a;
    
    a = INT32_MIN;
    b = -1;
    results[idx++] = __builtin_sub_overflow(a, b, &a);
    results[idx++] = a;
    
    a = INT32_MAX / 2 + 1;
    b = 2;
    results[idx++] = __builtin_mul_overflow(a, b, &a);
    results[idx++] = a;
    
    /* Test 2: Overflow in loops */
    uint32_t sum = 0;
    for (uint32_t i = 0; i < 1000; i++) {
        uint32_t old_sum = sum;
        overflow = __builtin_add_overflow(sum, i * 1000, &sum);
        results[idx++] = overflow;
        if (overflow) sum = old_sum;
    }
    results[idx++] = sum;
    
    /* Test 3: Conditional overflow checks */
    int32_t x = 100, y = 200;
    if (x > 0 && y > INT32_MAX - x) {
        results[idx++] = 1;
    } else {
        results[idx++] = __builtin_add_overflow(x, y, &x);
    }
    results[idx++] = x;
    
    x = -100, y = INT32_MIN;
    if (y < 0 && x < INT32_MIN - y) {
        results[idx++] = 1;
    } else {
        results[idx++] = __builtin_add_overflow(x, y, &x);
    }
    results[idx++] = x;
    
    /* Test 4: Multiplication with known ranges */
    for (int i = -10; i <= 10; i++) {
        for (int j = -10; j <= 10; j++) {
            int32_t prod;
            overflow = __builtin_mul_overflow(i * 100000, j * 100000, &prod);
            results[idx % 24] ^= overflow | prod;
            idx++;
        }
    }
    
    checksum = compute_checksum(results, sizeof(results));
    sink = checksum;
    return checksum;
}

/* ========== Test 6: Edge Case Conditionals ========== */
__attribute__((noinline))
static uint32_t test_edge_conditionals(void) {
    uint32_t checksum = 0;
    uint32_t results[32] = {0};
    int idx = 0;
    
    /* Extreme boundary comparisons */
    int64_t var = 0;
    
    /* These comparisons should trigger range analysis */
    if (var > INT64_MAX - 10) results[idx++] = 1;
    if (var < INT64_MIN + 10) results[idx++] = 2;
    
    /* Modulo-constrained ranges */
    uint32_t m = 1000;
    for (uint32_t i = 0; i < 10000; i++) {
        uint32_t constrained = i % m;
        if (constrained > UINT32_MAX - 100) results[idx % 32]++;
        if (constrained < 100) results[(idx + 1) % 32]++;
        idx++;
    }
    
    /* Chain of comparisons narrowing range */
    int32_t x = 500;
    if (x > 0) {
        if (x < 1000) {
            if (x > 100) {
                if (x < 900) {
                    results[0] = x;
                }
            }
        }
    }
    
    /* Bitwise range constraints */
    uint32_t y = 0x87654321;
    uint32_t masked = y & 0xFFFF;
    if (masked > 0xFF00) results[1] = masked;
    if (masked < 0x00FF) results[2] = masked;
    
    /* Shift-based range testing */
    for (int shift = 0; shift < 32; shift++) {
        uint32_t shifted = 1U << shift;
        if (shifted > 0x80000000U) results[3]++;
        if (shifted < 0x000000FFU) results[4]++;
    }
    
    checksum = compute_checksum(results, sizeof(results));
    sink = checksum;
    return checksum;
}

/* ========== Main Function ========== */
int main(void) {
    uint32_t final_checksum = 0;
    
    printf("Starting integer range analysis tests...\n");
    
    final_checksum ^= test_narrowing_conversions();
    printf("Test 1 complete\n");
    
    final_checksum ^= test_loop_range_analysis();
    printf("Test 2 complete\n");
    
    final_checksum ^= test_saturation_arithmetic();
    printf("Test 3 complete\n");
    
    final_checksum ^= test_bitfield_ranges();
    printf("Test 4 complete\n");
    
    final_checksum ^= test_overflow_builtins();
    printf("Test 5 complete\n");
    
    final_checksum ^= test_edge_conditionals();
    printf("Test 6 complete\n");
    
    printf("Final checksum: 0x%08X\n", final_checksum);
    
    /* Use results to prevent optimization */
    sink = final_checksum;
    
    return (final_checksum != 0) ? 0 : 1;
}
