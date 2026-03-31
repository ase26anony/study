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

/* Test 1: Narrowing conversions with boundary values */
__attribute__((noinline))
uint32_t test_narrowing_conversions(void) {
    uint32_t hash = 0;
    
    /* Constants at type boundaries */
    int64_t large_positive = 0x7FFFFFFF;
    int64_t large_negative = -0x80000000LL;
    uint64_t large_unsigned = 0xFFFFFFFFULL;
    
    /* Narrowing conversions that require range analysis */
    int32_t n1 = (int32_t)large_positive;      /* Exact boundary */
    int32_t n2 = (int32_t)large_negative;      /* Negative boundary */
    uint32_t n3 = (uint32_t)large_unsigned;    /* Unsigned boundary */
    
    /* Operations that might overflow */
    int64_t sum = large_positive + 1;
    int32_t n4 = (int32_t)sum;                 /* Potential overflow */
    
    /* Comparisons against boundaries */
    if (n1 > 0x7FFFFFFE) hash ^= 1;
    if (n2 < -0x7FFFFFFF) hash ^= 2;
    if (n3 == 0xFFFFFFFF) hash ^= 4;
    
    /* Shift operations with boundary values */
    int32_t shifted = n1 << 1;
    if (shifted < 0) hash ^= 8;                /* Sign check after shift */
    
    sink = n4;
    return mix(hash ^ (uint32_t)n1 ^ (uint32_t)n2 ^ n3);
}

/* Test 2: Complex loop bound analysis */
__attribute__((noinline))
uint32_t test_loop_range_analysis(void) {
    uint32_t hash = 0;
    
    /* Variables with constrained ranges */
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    int32_t c = 100;
    
    /* Loop with bitwise-derived bounds */
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += (c & 0x3F)) {
        hash = mix(hash ^ i);
        
        /* Nested loop with dependent bounds */
        for (int32_t j = (i & 0xFF) - 128; j < (int32_t)(i >> 8) + 64; j++) {
            if (j > 0 && j < 100) {
                hash = mix(hash ^ (uint32_t)j);
            }
        }
        
        /* Break condition based on range analysis */
        if (i > 0xF0000000) break;
    }
    
    /* Another loop with signed/unsigned mixing */
    int32_t start = -1000;
    uint32_t limit = 500;
    for (int32_t k = start; (uint32_t)k < limit; k += 73) {
        hash = mix(hash ^ (uint32_t)k);
    }
    
    sink = hash;
    return hash;
}

/* Test 3: Saturation arithmetic */
__attribute__((noinline))
uint32_t test_saturation_arithmetic(void) {
    uint32_t hash = 0;
    
    /* Manual saturation implementation */
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
    int32_t vals[] = {0x7FFFFFF0, -0x7FFFFFF0, 100, -100, 0x40000000};
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int32_t s1 = sat_add(vals[i], vals[j]);
            int32_t s2 = sat_mul(vals[i], vals[j]);
            hash = mix(hash ^ (uint32_t)s1 ^ (uint32_t)s2);
        }
    }
    
    /* Fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.9r;
    _Fract f3 = f1 + f2;  /* May saturate */
    hash = mix(hash ^ *(uint32_t*)&f3);
    #endif
    
    sink = hash;
    return hash;
}

/* Test 4: Bit-field range analysis */
__attribute__((noinline))
uint32_t test_bitfield_ranges(void) {
    uint32_t hash = 0;
    
    /* Struct with various bit-fields */
    struct BitFields {
        signed int a : 5;    /* -16 to 15 */
        unsigned int b : 7;  /* 0 to 127 */
        signed int c : 12;   /* -2048 to 2047 */
        unsigned int d : 3;  /* 0 to 7 */
    } bf;
    
    /* Union to test bit-field storage */
    union BitUnion {
        struct BitFields fields;
        uint32_t raw;
    } u;
    
    /* Assign boundary values */
    u.fields.a = 15;         /* Max positive for 5-bit signed */
    u.fields.b = 127;        /* Max for 7-bit unsigned */
    u.fields.c = -2048;      /* Min for 12-bit signed */
    u.fields.d = 7;          /* Max for 3-bit unsigned */
    
    hash = mix(u.raw);
    
    /* Comparisons that require bit-field range understanding */
    if (u.fields.a == 15) hash ^= 0x11111111;
    if (u.fields.b > 100) hash ^= 0x22222222;
    if (u.fields.c < -2000) hash ^= 0x44444444;
    if (u.fields.d != 0) hash ^= 0x88888888;
    
    /* Operations that might overflow bit-field */
    int32_t temp = u.fields.a * 10;  /* Definitely exceeds 5 bits */
    u.fields.a = temp & 0x1F;        /* Explicit mask */
    
    hash = mix(hash ^ u.raw ^ (uint32_t)temp);
    
    sink = u.raw;
    return hash;
}

/* Test 5: Overflow builtins with range analysis */
__attribute__((noinline))
uint32_t test_overflow_builtins(void) {
    uint32_t hash = 0;
    
    int32_t a = 0x70000000;
    int32_t b = 0x10000000;
    int32_t result;
    int overflow;
    
    /* Basic overflow checks */
    overflow = __builtin_add_overflow(a, b, &result);
    hash = mix(hash ^ (uint32_t)result ^ (overflow << 16));
    
    overflow = __builtin_mul_overflow(a, 2, &result);
    hash = mix(hash ^ (uint32_t)result ^ (overflow << 17));
    
    overflow = __builtin_sub_overflow(-0x70000000, b, &result);
    hash = mix(hash ^ (uint32_t)result ^ (overflow << 18));
    
    /* Overflow checks in loops with constrained ranges */
    for (int32_t i = 0x70000000; i < 0x70000100; i += 0x10) {
        for (int32_t j = 0x10000000; j < 0x10000100; j += 0x10) {
            int ov;
            int32_t res;
            
            ov = __builtin_add_overflow(i, j, &res);
            if (!ov) {
                hash = mix(hash ^ (uint32_t)res);
            }
            
            ov = __builtin_mul_overflow(i, 3, &res);
            hash = mix(hash ^ (uint32_t)ov);
        }
    }
    
    /* Unsigned overflow checks */
    uint32_t u1 = 0xFFFFFFFF;
    uint32_t u2 = 1;
    uint32_t ures;
    
    overflow = __builtin_add_overflow(u1, u2, &ures);
    hash = mix(hash ^ ures ^ (overflow << 24));
    
    sink = result;
    return hash;
}

/* Test 6: Additional boundary condition tests */
__attribute__((noinline))
uint32_t test_boundary_conditions(void) {
    uint32_t hash = 0;
    
    /* Edge case comparisons */
    int64_t values[] = {
        0x7FFFFFFFFFFFFFFFLL,  /* INT64_MAX */
        -0x8000000000000000LL, /* INT64_MIN */
        0xFFFFFFFFFFFFFFFFLL,  /* UINT64_MAX */
        0,
        0x7FFFFFFF,            /* INT32_MAX */
        -0x80000000            /* INT32_MIN */
    };
    
    for (int i = 0; i < 6; i++) {
        /* Comparisons that trigger range analysis */
        if (values[i] > 0x7FFFFFFFFFFFFFFFLL - 10) hash ^= 1 << i;
        if (values[i] < -0x7FFFFFFFFFFFFFFFLL + 10) hash ^= 1 << (i + 6);
        
        /* Narrowing with explicit checks */
        int32_t narrowed = (int32_t)values[i];
        if (narrowed == 0x7FFFFFFF) hash ^= 1 << (i + 12);
        if (narrowed == -0x80000000) hash ^= 1 << (i + 18);
    }
    
    /* Shift operations at bit-width boundaries */
    uint64_t x = 0x8000000000000000ULL;
    for (int shift = 0; shift < 64; shift += 8) {
        uint64_t shifted = x >> shift;
        hash = mix(hash ^ (uint32_t)(shifted >> 32) ^ (uint32_t)shifted);
    }
    
    sink = hash;
    return hash;
}

int main(void) {
    uint32_t final_hash = 0;
    
    final_hash ^= test_narrowing_conversions();
    final_hash ^= test_loop_range_analysis();
    final_hash ^= test_saturation_arithmetic();
    final_hash ^= test_bitfield_ranges();
    final_hash ^= test_overflow_builtins();
    final_hash ^= test_boundary_conditions();
    
    /* Use result to prevent optimization */
    printf("Result: %u\n", final_hash);
    
    return (int)(final_hash & 0xFF);
}
