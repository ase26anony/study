#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Prevent inlining for better coverage tracking */
__attribute__((noinline, cold))
unsigned test_narrowing_conversions(void) {
    unsigned hash = 0;
    
    /* Test 1: Narrowing with constants at boundaries */
    int64_t wide1 = INT64_MAX;
    int32_t narrow1 = (int32_t)wide1;  /* Should trigger range analysis */
    hash ^= (unsigned)narrow1;
    
    /* Test 2: Narrowing after arithmetic */
    uint64_t wide2 = UINT64_MAX - 100;
    uint32_t narrow2 = (uint32_t)(wide2 >> 16);
    hash ^= narrow2;
    
    /* Test 3: Multiple narrowing steps */
    int64_t x = 0x7FFFFFFFFFFFFFFFLL;
    int32_t y = (int32_t)x;
    int16_t z = (int16_t)y;
    int8_t w = (int8_t)z;
    hash ^= (unsigned)w;
    
    /* Test 4: Narrowing with sign extension implications */
    int32_t a = -1000;
    int64_t b = (int64_t)a;
    int32_t c = (int32_t)(b * 2);  /* Range analysis needed */
    hash ^= (unsigned)c;
    
    sink = hash;
    return hash;
}

__attribute__((noinline, cold))
unsigned test_loop_range_analysis(void) {
    unsigned hash = 0;
    int i, j, k;
    
    /* Test 1: Loop with bitmasked bounds */
    uint32_t start = 0x12345678;
    uint32_t end = 0x9ABCDEF0;
    
    for (i = start & 0xFFFF; i < (end | 0x7FFF); i += 257) {
        hash += i * 3;
        
        /* Nested loop with dependent bounds */
        for (j = (i & 0xFF); j < 1000; j += (i & 0x1F) + 1) {
            hash ^= j;
            
            /* Triple nested with complex condition */
            for (k = (j ^ 0x55); k < (i | 0x3FF); k += (j & 0xF) + 1) {
                if (k > (INT_MAX - 100) && k < (INT_MAX - 50)) {
                    hash += k * 7;
                }
            }
        }
    }
    
    /* Test 2: Loop with shifting bounds */
    int64_t base = 1LL << 40;
    for (i = 0; i < 100; i++) {
        int64_t bound = base >> (i & 0x1F);
        if (bound < INT_MAX) {
            for (j = 0; j < (int)bound; j += (i & 0x3) + 1) {
                hash += j * 11;
            }
        }
    }
    
    /* Test 3: Loop with XOR pattern creating range constraints */
    uint32_t mask = 0xAAAAAAAA;
    for (i = 0; i < 1000; i++) {
        uint32_t val = i ^ mask;
        if (val > 0x80000000 && val < 0xFFFFFFFF) {
            hash += (val & 0xFFFF);
        }
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline, cold))
unsigned test_saturation_arithmetic(void) {
    unsigned hash = 0;
    
    /* Manual saturation arithmetic */
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + (int64_t)b;
        if (result > INT_MAX) return INT_MAX;
        if (result < INT_MIN) return INT_MIN;
        return (int32_t)result;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * (int64_t)b;
        if (result > INT_MAX) return INT_MAX;
        if (result < INT_MIN) return INT_MIN;
        return (int32_t)result;
    }
    
    /* Test boundary cases */
    hash ^= (unsigned)sat_add(INT_MAX, 1);
    hash ^= (unsigned)sat_add(INT_MIN, -1);
    hash ^= (unsigned)sat_add(INT_MAX / 2, INT_MAX / 2 + 1);
    
    hash ^= (unsigned)sat_mul(INT_MAX, 2);
    hash ^= (unsigned)sat_mul(INT_MIN, 2);
    hash ^= (unsigned)sat_mul(46340, 46341);  /* Just over INT_MAX sqrt */
    
    /* Test with GCC fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.7r;
    _Fract f3 = f1 + f2;
    hash ^= *(unsigned*)&f3;
    #endif
    
    sink = hash;
    return hash;
}

__attribute__((noinline, cold))
unsigned test_bitfield_ranges(void) {
    unsigned hash = 0;
    
    /* Struct with various bit-field sizes */
    struct bitfields {
        unsigned int a : 3;   /* 0-7 */
        signed int b : 5;     /* -16 to 15 */
        unsigned int c : 12;  /* 0-4095 */
        signed int d : 20;    /* -524288 to 524287 */
        unsigned int e : 1;   /* 0-1 */
    } bf;
    
    /* Assign values at boundaries */
    bf.a = 7;      /* Max for 3 bits */
    bf.b = -16;    /* Min for 5 signed bits */
    bf.c = 4095;   /* Max for 12 bits */
    bf.d = 524287; /* Max for 20 signed bits */
    bf.e = 1;      /* Max for 1 bit */
    
    /* Comparisons that require range analysis */
    if (bf.a == 7) hash += 1;
    if (bf.b < 0) hash += 2;
    if (bf.c > 4000) hash += 4;
    if (bf.d != -524288) hash += 8;
    if (bf.e != 0) hash += 16;
    
    /* Union with overlapping bit-fields */
    union overlay {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } parts;
        uint32_t whole;
    } u;
    
    u.whole = 0xDEADBEEF;
    if (u.parts.low > 0x8000) hash += 32;
    if (u.parts.high < 0xFFFF) hash += 64;
    
    /* Nested bit-field struct */
    struct nested {
        struct {
            unsigned x : 4;
            unsigned y : 4;
        } inner;
        unsigned z : 8;
    } n;
    
    n.inner.x = 15;
    n.inner.y = 0;
    n.z = 255;
    
    if (n.inner.x == 15 && n.z == 255) hash += 128;
    
    sink = hash;
    return hash;
}

__attribute__((noinline, cold))
unsigned test_overflow_builtins(void) {
    unsigned hash = 0;
    int overflow;
    
    /* Test 1: Builtins with constants at boundaries */
    int32_t r1, r2, r3;
    
    overflow = __builtin_add_overflow(INT_MAX, 1, &r1);
    hash ^= (overflow << 0) ^ (unsigned)r1;
    
    overflow = __builtin_sub_overflow(INT_MIN, 1, &r2);
    hash ^= (overflow << 1) ^ (unsigned)r2;
    
    overflow = __builtin_mul_overflow(46341, 46341, &r3);
    hash ^= (overflow << 2) ^ (unsigned)r3;
    
    /* Test 2: Builtins in loops with range-constrained values */
    int32_t accum = 0;
    for (int i = 0; i < 100; i++) {
        int32_t val = (i & 0x3FF) * 1000;
        if (__builtin_add_overflow(accum, val, &accum)) {
            accum = INT_MAX;
        }
        hash += accum;
    }
    
    /* Test 3: Builtins with values from earlier range analysis */
    uint32_t x = 0xFFFFFFFF;
    uint32_t y = 0x00000001;
    uint32_t sum;
    
    if (!__builtin_add_overflow(x, y, &sum)) {
        hash ^= sum;
    }
    
    /* Test 4: Chain of overflow checks */
    int32_t a = 1000000;
    int32_t b = 2000000;
    int32_t c = 3000000;
    int32_t tmp, result;
    
    if (!__builtin_add_overflow(a, b, &tmp) &&
        !__builtin_add_overflow(tmp, c, &result)) {
        hash ^= result;
    } else {
        hash ^= 0xDEADBEEF;
    }
    
    /* Test 5: Overflow with bitwise constrained values */
    uint32_t mask = 0x00FFFFFF;  /* Upper 8 bits always 0 */
    for (int i = 0; i < 50; i++) {
        uint32_t val1 = (i * 1000) & mask;
        uint32_t val2 = (i * 2000) & mask;
        uint32_t mul_result;
        
        if (__builtin_mul_overflow(val1, val2, &mul_result)) {
            hash += i;
        }
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline, cold))
unsigned test_edge_case_comparisons(void) {
    unsigned hash = 0;
    
    /* Direct comparisons at type boundaries */
    int64_t big = INT64_MAX;
    int32_t medium = INT32_MAX;
    int16_t small = INT16_MAX;
    
    if (big > (INT64_MAX - 1000)) hash += 1;
    if (medium == INT32_MAX) hash += 2;
    if (small < (INT16_MIN + 100)) hash += 4;
    
    /* Comparisons after extensions */
    uint32_t u32 = 0x80000000;  /* Exactly at signed boundary */
    int64_t extended = (int64_t)(int32_t)u32;
    if (extended < 0) hash += 8;
    
    /* Complex boundary condition */
    int32_t x = 100;
    int32_t y = 200;
    if (x > (INT_MAX - y - 10)) {
        hash += 16;
    }
    
    /* Multiple comparisons chained */
    int32_t val = 0x7FFFFFF0;
    if (val > INT_MAX - 100 && val < INT_MAX - 50) {
        hash += 32;
    }
    
    /* Comparison with shifted boundaries */
    uint64_t shifted = 1ULL << 63;
    if (shifted > (UINT64_MAX >> 1)) {
        hash += 64;
    }
    
    sink = hash;
    return hash;
}

int main(void) {
    unsigned final_hash = 0;
    
    final_hash ^= test_narrowing_conversions();
    final_hash ^= test_loop_range_analysis();
    final_hash ^= test_saturation_arithmetic();
    final_hash ^= test_bitfield_ranges();
    final_hash ^= test_overflow_builtins();
    final_hash ^= test_edge_case_comparisons();
    
    /* Use the result to prevent optimization */
    printf("Result hash: %u\n", final_hash);
    
    return (int)(final_hash & 0xFF);
}
