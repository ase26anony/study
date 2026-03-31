#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum function to prevent optimization */
static inline uint32_t checksum(uint32_t seed, uint32_t value) {
    return seed * 31 + value;
}

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline))
uint32_t test_narrowing_conversions(void) {
    uint32_t hash = 0;
    
    /* Wide to narrow conversions with boundary values */
    int64_t wide_vals[] = {
        INT64_MAX, INT64_MIN, 
        (int64_t)INT32_MAX + 1, (int64_t)INT32_MIN - 1,
        0x7FFFFFFF00000000LL, 0x80000000FFFFFFFFLL
    };
    
    for (size_t i = 0; i < sizeof(wide_vals)/sizeof(wide_vals[0]); i++) {
        /* Explicit narrowing casts that require range analysis */
        int32_t narrow1 = (int32_t)wide_vals[i];
        uint32_t narrow2 = (uint32_t)wide_vals[i];
        
        /* Comparisons against type limits */
        if (wide_vals[i] > INT32_MAX) {
            hash = checksum(hash, 1);
        }
        if (wide_vals[i] < INT32_MIN) {
            hash = checksum(hash, 2);
        }
        if ((uint64_t)wide_vals[i] > UINT32_MAX) {
            hash = checksum(hash, 3);
        }
        
        sink = narrow1 + narrow2;
    }
    
    /* Shifts that may overflow */
    uint32_t x = 0x80000000;
    uint32_t y = x >> 1;
    uint32_t z = x << 1;  /* Potential overflow */
    
    if (z < x) {  /* Overflow detection */
        hash = checksum(hash, 4);
    }
    
    return hash;
}

__attribute__((noinline))
uint32_t test_loop_range_analysis(void) {
    uint32_t hash = 0;
    
    /* Complex loop bounds with bitwise operations */
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    uint32_t c = 17;
    
    /* Loop with bounds derived from bitwise ops */
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += c) {
        hash = checksum(hash, i);
        
        /* Nested loop with dependent bounds */
        for (uint32_t j = i & 0xFF; j < 1000; j += (i % 13) + 1) {
            hash = checksum(hash, j);
            
            /* Conditional that depends on loop variables */
            if ((i ^ j) > 0x80000000) {
                hash = checksum(hash, 0xDEAD);
            }
        }
    }
    
    /* Loop with wrap-around detection */
    uint32_t counter = 0;
    for (int k = 0; k < 100; k++) {
        counter += 0x40000000;  /* May overflow in 32-bit */
        if (counter < 0x40000000) {  /* Overflow check */
            hash = checksum(hash, 0xBEEF);
        }
    }
    
    return hash;
}

__attribute__((noinline))
uint32_t test_saturation_arithmetic(void) {
    uint32_t hash = 0;
    
    /* Manual saturation arithmetic */
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + (int64_t)b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * (int64_t)b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    /* Test with boundary values */
    struct {
        int32_t a, b;
    } tests[] = {
        {INT32_MAX, 1},
        {INT32_MIN, -1},
        {INT32_MAX, INT32_MAX},
        {INT32_MIN, INT32_MIN},
        {0x40000000, 2},
        {-0x40000000, -2}
    };
    
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        int32_t sum = sat_add(tests[i].a, tests[i].b);
        int32_t prod = sat_mul(tests[i].a, tests[i].b);
        
        hash = checksum(hash, sum);
        hash = checksum(hash, prod);
        
        /* Check if saturation occurred */
        if (sum == INT32_MAX || sum == INT32_MIN) {
            hash = checksum(hash, 0xSAT);
        }
    }
    
    return hash;
}

__attribute__((noinline))
uint32_t test_bitfield_ranges(void) {
    uint32_t hash = 0;
    
    /* Struct with various bit-fields */
    struct {
        unsigned int a : 5;   /* 0-31 */
        signed int b : 7;     /* -64 to 63 */
        unsigned int c : 12;  /* 0-4095 */
        signed int d : 20;    /* -524288 to 524287 */
    } bits;
    
    /* Assign boundary values */
    bits.a = 31;      /* Max for 5 bits */
    bits.b = -64;     /* Min for signed 7 bits */
    bits.c = 4095;    /* Max for 12 bits */
    bits.d = 524287;  /* Max for signed 20 bits */
    
    /* Comparisons that test bit-field range understanding */
    if (bits.a == 31) {
        hash = checksum(hash, 1);
    }
    if (bits.b < 0) {
        hash = checksum(hash, 2);
    }
    if (bits.c > 4000) {
        hash = checksum(hash, 3);
    }
    if (bits.d > 500000) {
        hash = checksum(hash, 4);
    }
    
    /* Union to test type-punning with bit-fields */
    union {
        struct {
            unsigned int x : 10;
            unsigned int y : 10;
            unsigned int z : 10;
        } bits;
        uint32_t word;
    } u;
    
    u.word = 0xFFFFFFFF;
    if (u.bits.x == 0x3FF) {  /* Max 10-bit value */
        hash = checksum(hash, 5);
    }
    
    return hash;
}

__attribute__((noinline))
uint32_t test_overflow_builtins(void) {
    uint32_t hash = 0;
    
    /* Use overflow builtins with partially known ranges */
    int32_t x = 1000000;
    int32_t y = 2000000;
    int32_t result;
    
    /* Basic overflow checks */
    if (__builtin_add_overflow(x, y, &result)) {
        hash = checksum(hash, 0xADD);
    }
    
    if (__builtin_mul_overflow(x, y, &result)) {
        hash = checksum(hash, 0xMUL);
    }
    
    /* Overflow checks in loops with constrained ranges */
    for (int32_t i = 1; i < 100; i++) {
        int32_t a = i * 100000;
        int32_t b = (100 - i) * 100000;
        int32_t sum;
        
        if (__builtin_add_overflow(a, b, &sum)) {
            hash = checksum(hash, i);
        }
    }
    
    /* Chain operations that might overflow */
    int32_t val = 1;
    for (int i = 0; i < 10; i++) {
        int32_t tmp;
        if (__builtin_mul_overflow(val, 3, &tmp)) {
            hash = checksum(hash, 0xCHAIN);
            break;
        }
        val = tmp;
    }
    
    /* Overflow with bitwise constrained values */
    uint32_t mask = 0x0FFFFFFF;  /* 28 bits set */
    for (uint32_t v = mask - 10; v < mask + 10; v++) {
        uint32_t r;
        if (__builtin_add_overflow(v, 100, &r)) {
            hash = checksum(hash, v & 0xFF);
        }
    }
    
    return hash;
}

__attribute__((noinline))
uint32_t test_edge_case_conditions(void) {
    uint32_t hash = 0;
    
    /* Conditions at type boundaries */
    int32_t vals[] = {INT32_MAX, INT32_MIN, 0, -100, 100};
    
    for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); i++) {
        /* Comparisons against extreme values */
        if (vals[i] > INT32_MAX - 10) {
            hash = checksum(hash, 1);
        }
        if (vals[i] < INT32_MIN + 10) {
            hash = checksum(hash, 2);
        }
        if (vals[i] >= 0 && vals[i] <= 100) {
            hash = checksum(hash, 3);
        }
    }
    
    /* Range-restricted variables */
    uint32_t x = 500;
    if (x > 0 && x < 1000) {
        /* x is known to be in [1, 999] */
        uint32_t y = x * 2;
        if (y > 1500) {  /* This should be knowable as false */
            hash = checksum(hash, 4);
        }
    }
    
    /* Modulo-constrained values */
    for (int i = 0; i < 256; i++) {
        uint32_t m = i % 37;  /* m is in [0, 36] */
        if (m > 40) {  /* Always false */
            hash = checksum(hash, 5);
        }
    }
    
    return hash;
}

int main(void) {
    uint32_t final_hash = 0;
    
    printf("Running integer range analysis tests...\n");
    
    final_hash = checksum(final_hash, test_narrowing_conversions());
    final_hash = checksum(final_hash, test_loop_range_analysis());
    final_hash = checksum(final_hash, test_saturation_arithmetic());
    final_hash = checksum(final_hash, test_bitfield_ranges());
    final_hash = checksum(final_hash, test_overflow_builtins());
    final_hash = checksum(final_hash, test_edge_case_conditions());
    
    printf("Final hash: %u\n", final_hash);
    printf("Sink value: %d\n", sink);
    
    return 0;
}
