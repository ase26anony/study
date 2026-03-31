#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum function to prevent optimization */
static inline uint32_t mix(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

__attribute__((noinline))
uint32_t test_narrowing_conversions(void) {
    uint32_t hash = 0;
    
    /* Test 1: Narrowing with constants at boundaries */
    int64_t wide1 = 0x7FFFFFFF;      /* INT_MAX for 32-bit */
    int64_t wide2 = 0x80000000;      /* INT_MIN for 32-bit */
    int64_t wide3 = 0xFFFFFFFFFFFFULL;
    
    int32_t narrow1 = (int32_t)wide1;  /* Should fit */
    int32_t narrow2 = (int32_t)wide2;  /* Should fit (wrapped) */
    int32_t narrow3 = (int32_t)wide3;  /* Will be truncated */
    
    hash = mix(hash + narrow1);
    hash = mix(hash + narrow2);
    hash = mix(hash + narrow3);
    
    /* Test 2: Narrowing with arithmetic results */
    uint64_t a = 0x123456789ABCDEF0ULL;
    uint64_t b = 0xFEDCBA9876543210ULL;
    uint32_t c = (uint32_t)(a + b);    /* Overflow in 64-bit, then narrow */
    uint32_t d = (uint32_t)(a - b);    /* Underflow, then narrow */
    
    hash = mix(hash + c);
    hash = mix(hash + d);
    
    /* Test 3: Chain of narrowing conversions */
    int64_t val = 1000;
    for (int i = 0; i < 10; i++) {
        int32_t tmp = (int32_t)val;
        int16_t tmp2 = (int16_t)tmp;
        int8_t tmp3 = (int8_t)tmp2;
        hash = mix(hash + tmp3);
        val = val * 3 + 7;
    }
    
    return hash;
}

__attribute__((noinline))
uint32_t test_loop_range_analysis(void) {
    uint32_t hash = 0;
    
    /* Complex loop bounds with bitwise operations */
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    
    /* Loop 1: Masked bounds */
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += 37) {
        hash = mix(hash + i);
        if (i > 0x800) break;  /* Additional condition */
    }
    
    /* Loop 2: Nested with dependent bounds */
    int32_t outer_max = 50;
    for (int32_t j = -100; j < outer_max; j += 3) {
        /* Inner loop bound depends on outer */
        int32_t inner_bound = (j & 0x3F) + 10;
        for (int32_t k = 0; k < inner_bound; k += 2) {
            hash = mix(hash + (j * k));
        }
    }
    
    /* Loop 3: XOR-based bounds */
    uint64_t x = 0x1000;
    uint64_t y = 0x3000;
    for (uint64_t i = x ^ 0xFF; i < (y ^ 0x7FF); i += 129) {
        hash = mix(hash + (uint32_t)i);
    }
    
    /* Loop 4: Shift-based bounds */
    int32_t base = 256;
    for (int32_t i = base >> 2; i < base << 2; i += 5) {
        /* Conditional that depends on i's range */
        if (i > 0 && i < 1000) {
            hash = mix(hash + i);
        }
    }
    
    return hash;
}

__attribute__((noinline))
uint32_t test_saturation_arithmetic(void) {
    uint32_t hash = 0;
    
    /* Manual saturation arithmetic */
    #define SAT_ADD(a, b, max_val) \
        (((b) > 0) && ((a) > (max_val) - (b))) ? (max_val) : ((a) + (b))
    
    #define SAT_SUB(a, b, min_val) \
        (((b) > 0) && ((a) < (min_val) + (b))) ? (min_val) : ((a) - (b))
    
    int32_t vals[] = {100, 500, 1000, 5000, 10000};
    const int32_t MAX_VAL = 2000;
    const int32_t MIN_VAL = -1000;
    
    /* Test saturation addition */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int32_t result = SAT_ADD(vals[i], vals[j], MAX_VAL);
            hash = mix(hash + result);
        }
    }
    
    /* Test saturation subtraction */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int32_t result = SAT_SUB(vals[i], vals[j], MIN_VAL);
            hash = mix(hash + result);
        }
    }
    
    /* Edge cases at boundaries */
    int32_t edge_cases[] = {INT32_MAX, INT32_MIN, INT32_MAX - 10, INT32_MIN + 10};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int32_t sum;
            if (edge_cases[j] > 0 && edge_cases[i] > INT32_MAX - edge_cases[j]) {
                sum = INT32_MAX;
            } else if (edge_cases[j] < 0 && edge_cases[i] < INT32_MIN - edge_cases[j]) {
                sum = INT32_MIN;
            } else {
                sum = edge_cases[i] + edge_cases[j];
            }
            hash = mix(hash + sum);
        }
    }
    
    return hash;
}

__attribute__((noinline))
uint32_t test_bitfield_ranges(void) {
    uint32_t hash = 0;
    
    /* Struct with various bit-fields */
    struct {
        unsigned int a : 3;   /* 0-7 */
        signed int b : 5;     /* -16 to 15 */
        unsigned int c : 10;  /* 0-1023 */
        signed int d : 12;    /* -2048 to 2047 */
    } bits;
    
    /* Assign values at boundaries */
    bits.a = 7;      /* Max for 3 bits */
    bits.b = -16;    /* Min for 5-bit signed */
    bits.c = 1023;   /* Max for 10 bits */
    bits.d = 2047;   /* Max for 12-bit signed */
    
    hash = mix(hash + bits.a);
    hash = mix(hash + bits.b);
    hash = mix(hash + bits.c);
    hash = mix(hash + bits.d);
    
    /* Conditional checks based on bit-field ranges */
    if (bits.a > 6) {          /* Only true for value 7 */
        hash = mix(hash + 1000);
    }
    
    if (bits.b < -15) {        /* Always false for 5-bit signed */
        hash = mix(hash + 2000);
    }
    
    if (bits.c >= 1000 && bits.c <= 1023) {
        hash = mix(hash + bits.c);
    }
    
    /* Union with bit-fields */
    union {
        struct {
            unsigned int x : 4;
            unsigned int y : 4;
            unsigned int z : 8;
        } bits;
        uint16_t value;
    } u;
    
    u.value = 0xFFFF;
    hash = mix(hash + u.bits.x);  /* Will be truncated to 4 bits */
    hash = mix(hash + u.bits.y);
    hash = mix(hash + u.bits.z);
    
    /* Test overflow in bit-field assignment */
    bits.a = 15;    /* Too large for 3 bits */
    bits.b = 20;    /* Too large for 5-bit signed */
    hash = mix(hash + bits.a);  /* Should be truncated */
    hash = mix(hash + bits.b);  /* Should be truncated */
    
    return hash;
}

__attribute__((noinline))
uint32_t test_overflow_builtins(void) {
    uint32_t hash = 0;
    
    /* Test overflow builtins with various inputs */
    int32_t x = 1000000;
    int32_t y = 2000000;
    int32_t result;
    int overflow;
    
    /* Basic overflow checks */
    overflow = __builtin_add_overflow(x, y, &result);
    hash = mix(hash + result);
    hash = mix(hash + overflow);
    
    overflow = __builtin_mul_overflow(x, y, &result);
    hash = mix(hash + result);
    hash = mix(hash + overflow);
    
    overflow = __builtin_sub_overflow(INT32_MIN, 1, &result);
    hash = mix(hash + result);
    hash = mix(hash + overflow);
    
    /* Overflow checks in loops */
    int32_t vals[] = {1, 10, 100, 1000, 10000, 100000};
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            overflow = __builtin_add_overflow(vals[i], vals[j], &result);
            hash = mix(hash + result);
            if (overflow) {
                hash = mix(hash + 0xDEAD);
            }
        }
    }
    
    /* Overflow with known-range values */
    int32_t a = 500;
    int32_t b;
    
    /* b's range is limited by condition */
    if (a > 0 && a < 1000) {
        b = a * 2;
        overflow = __builtin_add_overflow(a, b, &result);
        hash = mix(hash + result + overflow);
    }
    
    /* Chain of operations with overflow checks */
    int32_t accum = 1;
    for (int i = 1; i <= 20; i++) {
        int32_t tmp;
        overflow = __builtin_mul_overflow(accum, i, &tmp);
        if (!overflow) {
            accum = tmp;
        } else {
            accum = INT32_MAX;
        }
        hash = mix(hash + accum);
    }
    
    /* Unsigned overflow checks */
    uint32_t u1 = 0xFFFFFFFF;
    uint32_t u2 = 1;
    uint32_t uresult;
    overflow = __builtin_add_overflow(u1, u2, &uresult);
    hash = mix(hash + uresult + overflow);
    
    return hash;
}

__attribute__((noinline))
uint32_t test_edge_case_conditions(void) {
    uint32_t hash = 0;
    
    /* Conditions at type boundaries */
    int32_t x = 100;
    
    /* These conditions test boundary analysis */
    if (x > INT32_MAX - 10) {
        hash = mix(hash + 1);
    }
    
    if (x < INT32_MIN + 10) {
        hash = mix(hash + 2);
    }
    
    /* Range-restricted variable */
    int32_t y = x & 0xFF;  /* y is in [0, 255] */
    
    if (y > 200) {
        hash = mix(hash + y);
    }
    
    if (y < 50) {
        hash = mix(hash + y * 2);
    }
    
    /* Modulo creates known range */
    int32_t z = x % 100;  /* z is in [0, 99] */
    
    if (z >= 0 && z <= 99) {
        hash = mix(hash + z + 1000);
    }
    
    /* Complex condition with AND/OR */
    int32_t a = 500, b = 1500;
    
    if ((a > 0 && a < 1000) || (b > 1000 && b < 2000)) {
        hash = mix(hash + a + b);
    }
    
    /* Condition that's always true/false based on ranges */
    int32_t always_true = 50;
    int32_t always_false = 500;
    
    if (always_true >= 0 && always_true <= 100) {
        hash = mix(hash + 555);
    }
    
    if (always_false < 0 && always_false > 1000) {
        hash = mix(hash + 666);
    }
    
    /* Shift operations in conditions */
    uint32_t val = 0x80000000;
    if ((val >> 16) > 0x7FFF) {
        hash = mix(hash + 777);
    }
    
    if ((val << 1) == 0) {
        hash = mix(hash + 888);
    }
    
    return hash;
}

int main(void) {
    uint32_t final_hash = 0;
    
    printf("Starting integer range analysis tests...\n");
    
    final_hash ^= test_narrowing_conversions();
    printf("  test_narrowing_conversions complete\n");
    
    final_hash ^= test_loop_range_analysis();
    printf("  test_loop_range_analysis complete\n");
    
    final_hash ^= test_saturation_arithmetic();
    printf("  test_saturation_arithmetic complete\n");
    
    final_hash ^= test_bitfield_ranges();
    printf("  test_bitfield_ranges complete\n");
    
    final_hash ^= test_overflow_builtins();
    printf("  test_overflow_builtins complete\n");
    
    final_hash ^= test_edge_case_conditions();
    printf("  test_edge_case_conditions complete\n");
    
    /* Use sink to prevent optimization */
    sink = final_hash;
    
    printf("Final hash: %u\n", final_hash);
    printf("All tests completed.\n");
    
    return 0;
}
