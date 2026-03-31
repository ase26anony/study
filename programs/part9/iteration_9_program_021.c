#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Prevent inlining for better coverage tracking */
__attribute__((noinline,optimize("no-tree-vectorize")))
unsigned test_narrowing_conversions(void) {
    unsigned hash = 0;
    
    /* Test 1: Narrowing with constants at boundaries */
    int64_t wide1 = INT64_MAX;
    int32_t narrow1 = (int32_t)wide1;  /* Should trigger range analysis */
    hash ^= (unsigned)narrow1;
    
    uint64_t wide2 = UINT64_MAX;
    uint32_t narrow2 = (uint32_t)wide2;
    hash ^= narrow2;
    
    /* Test 2: Narrowing after arithmetic */
    int64_t a = 1000000000LL;
    int64_t b = 2000000000LL;
    int64_t sum = a + b;
    int32_t narrow_sum = (int32_t)sum;  /* May overflow 32-bit */
    hash ^= (unsigned)narrow_sum;
    
    /* Test 3: Shifts that may overflow */
    uint32_t x = 0x80000000U;
    uint32_t shifted = x << 3;  /* Definitely overflows 32-bit */
    uint16_t narrow_shift = (uint16_t)shifted;
    hash ^= narrow_shift;
    
    /* Test 4: Complex narrowing chain */
    int64_t val = -9223372036854775807LL;
    for (int i = 0; i < 4; i++) {
        val += 1000000000LL;
        int32_t temp = (int32_t)val;
        hash = hash * 31 + (unsigned)temp;
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline,optimize("O3")))
unsigned test_loop_range_analysis(void) {
    unsigned hash = 0;
    
    /* Test 1: Loop with bitmasked bounds */
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    
    for (uint32_t i = a & 0x0000FFFF; i < (b | 0x0000FFFF); i += 0x100) {
        hash += i;
        /* Nested loop with dependent bounds */
        for (uint32_t j = 0; j < (i & 0xFF); j++) {
            hash ^= j * 3;
        }
    }
    
    /* Test 2: Loop with shifting bounds */
    int64_t start = -100;
    int64_t end = 100;
    for (int64_t k = start; k < end; k++) {
        /* Complex condition involving bitwise ops */
        if ((k & 0x7F) == 0x7F || (k ^ 0x55) < 10) {
            hash = hash * 7 + (unsigned)k;
        }
        
        /* Bound that depends on k */
        uint32_t inner_bound = (k < 0) ? 10 : 20;
        for (uint32_t m = 0; m < inner_bound; m++) {
            hash += (m << (k & 0x3));
        }
    }
    
    /* Test 3: Loop with wrap-around detection */
    uint8_t counter = 250;
    for (int n = 0; n < 20; n++) {
        counter += 10;  /* Will wrap around */
        hash += counter;
        
        /* Comparison against boundary */
        if (counter > 240) {
            hash ^= 0xDEADBEEF;
        }
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline))
unsigned test_saturation_arithmetic(void) {
    unsigned hash = 0;
    
    /* Manual saturation implementation */
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + (int64_t)b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    /* Test saturation at boundaries */
    int32_t vals[] = {INT32_MAX - 10, 100, INT32_MIN + 10, -100};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int32_t saturated = sat_add(vals[i], vals[j]);
            hash = hash * 19 + (unsigned)saturated;
        }
    }
    
    /* Test with shifting */
    int32_t x = 0x40000000;
    for (int i = 0; i < 8; i++) {
        int32_t shifted = x << i;
        int32_t saturated = sat_add(shifted, shifted);
        hash ^= (unsigned)saturated;
    }
    
    /* Fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.7r;
    _Fract fsum = f1 + f2;
    hash += *(unsigned*)&fsum;
    #endif
    
    sink = hash;
    return hash;
}

__attribute__((noinline))
unsigned test_bitfield_ranges(void) {
    unsigned hash = 0;
    
    /* Struct with various bit-fields */
    struct BitFields {
        unsigned int a : 3;   /* 0-7 */
        signed int b : 5;     /* -16 to 15 */
        unsigned int c : 12;  /* 0-4095 */
        signed int d : 20;    /* -524288 to 524287 */
    } bf;
    
    /* Assign values at boundaries */
    bf.a = 7;      /* Max for 3 bits */
    bf.b = -16;    /* Min for 5-bit signed */
    bf.c = 4095;   /* Max for 12 bits */
    bf.d = 524287; /* Max for 20-bit signed */
    
    hash = bf.a + (bf.b << 3) + (bf.c << 8) + (bf.d << 20);
    
    /* Comparisons against bit-field capacity */
    if (bf.a == 7) hash ^= 0x1111;
    if (bf.b == -16) hash ^= 0x2222;
    if (bf.c >= 4000) hash ^= 0x3333;
    if (bf.d > 500000) hash ^= 0x4444;
    
    /* Union with overlapping bit-fields */
    union Overlap {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } parts;
        uint32_t whole;
    } u;
    
    u.whole = 0x87654321;
    if (u.parts.low > 0x8000) hash ^= 0x5555;
    if (u.parts.high < 0x9000) hash ^= 0x6666;
    
    /* Complex bit-field arithmetic */
    for (int i = 0; i < 10; i++) {
        bf.a = (bf.a + 1) & 0x7;  /* Keep in 3-bit range */
        bf.c = (bf.c * 3) % 4096; /* Keep in 12-bit range */
        hash = hash * 23 + bf.a + bf.c;
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline))
unsigned test_overflow_builtins(void) {
    unsigned hash = 0;
    
    /* Test overflow builtins with range-constrained values */
    int32_t x = 1000000;
    int32_t y = 2000000;
    int32_t result;
    
    /* Basic overflow checks */
    if (__builtin_add_overflow(x, y, &result)) {
        hash ^= 0xAAAA;
    } else {
        hash ^= (unsigned)result;
    }
    
    /* In loop with varying bounds */
    for (int32_t i = -1000; i < 1000; i += 100) {
        int32_t a = i * 10000;
        int32_t b = 50000;
        int32_t sum;
        
        if (__builtin_add_overflow(a, b, &sum)) {
            hash += 0xBBBB;
        } else {
            hash += sum;
        }
        
        /* Multiplication overflow */
        int32_t prod;
        if (__builtin_mul_overflow(a / 100, b, &prod)) {
            hash ^= 0xCCCC;
        }
    }
    
    /* Chain of operations with overflow checks */
    int64_t accum = 0;
    for (int i = 0; i < 50; i++) {
        int32_t a = i * 100000;
        int32_t b = 30000;
        int32_t tmp;
        
        if (!__builtin_add_overflow(a, b, &tmp)) {
            accum += tmp;
            if (__builtin_mul_overflow(tmp, 2, &tmp)) {
                hash += 0xDDDD;
            }
        }
    }
    
    hash ^= (unsigned)(accum >> 32);
    hash ^= (unsigned)accum;
    
    /* Overflow with bitwise constrained values */
    uint32_t mask = 0x00FFFFFF;
    for (uint32_t val = 0xFF000000; val != 0; val >>= 1) {
        uint32_t masked = val & mask;
        uint32_t add_result;
        if (__builtin_add_overflow(masked, 0x1000000, &add_result)) {
            hash = hash * 3 + 1;
        }
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline))
unsigned test_edge_case_comparisons(void) {
    unsigned hash = 0;
    
    /* Comparisons at type boundaries */
    int32_t max_val = INT32_MAX;
    int32_t min_val = INT32_MIN;
    
    /* These comparisons should trigger boundary analysis */
    if (max_val > INT32_MAX - 10) hash ^= 0x11111111;
    if (min_val < INT32_MIN + 10) hash ^= 0x22222222;
    
    /* Value-dependent dead branches */
    int32_t x = 100;
    if (x > 0 && x < 50) {
        /* This branch should be recognized as dead */
        hash += 0x3333;
    }
    
    /* Complex boundary condition */
    uint64_t big = UINT64_MAX;
    for (uint32_t i = 0; i < 10; i++) {
        big -= 1000000000000ULL;
        if (big < 1000000000000ULL) {
            hash = hash * 5 + i;
        }
    }
    
    /* Modulo-constrained ranges */
    int32_t constrained = 12345;
    constrained = constrained % 1000;  /* Now 0-999 */
    
    if (constrained > 500) {
        hash ^= 0x4444;
    }
    if (constrained < 100) {
        hash ^= 0x8888;
    }
    
    /* Shift with potential overflow */
    uint32_t base = 0x80000000;
    for (int shift = 0; shift < 5; shift++) {
        uint32_t shifted = base << shift;
        /* Comparison that depends on overflow behavior */
        if (shifted < base) {
            hash += 0x7777 << shift;
        }
    }
    
    sink = hash;
    return hash;
}

int main(void) {
    unsigned final_hash = 0;
    
    printf("Starting integer range analysis tests...\n");
    
    final_hash ^= test_narrowing_conversions();
    printf("Narrowing conversions test complete\n");
    
    final_hash ^= test_loop_range_analysis();
    printf("Loop range analysis test complete\n");
    
    final_hash ^= test_saturation_arithmetic();
    printf("Saturation arithmetic test complete\n");
    
    final_hash ^= test_bitfield_ranges();
    printf("Bitfield ranges test complete\n");
    
    final_hash ^= test_overflow_builtins();
    printf("Overflow builtins test complete\n");
    
    final_hash ^= test_edge_case_comparisons();
    printf("Edge case comparisons test complete\n");
    
    printf("Final hash: 0x%08X\n", final_hash);
    
    /* Use the result to prevent optimization */
    if (final_hash == 0x12345678) {
        printf("Impossible condition\n");
    }
    
    return 0;
}
