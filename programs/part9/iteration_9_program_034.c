#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum function to prevent optimization */
static inline unsigned checksum(unsigned x) {
    return x * 1103515245 + 12345;
}

/* Test functions marked noinline for coverage tracking */
__attribute__((noinline))
unsigned test_narrowing_conversions(void) {
    unsigned hash = 0;
    
    /* Test 1: Narrowing with constants at boundaries */
    int64_t wide1 = INT64_MAX;
    int32_t narrow1 = (int32_t)wide1;  /* Should trigger range analysis */
    hash ^= checksum(narrow1);
    
    /* Test 2: Narrowing after arithmetic */
    uint64_t wide2 = UINT64_MAX - 100;
    uint32_t narrow2 = (uint32_t)wide2;
    hash ^= checksum(narrow2);
    
    /* Test 3: Chain of narrowing conversions */
    int64_t a = 0x7FFFFFFF00000000LL;
    int32_t b = (int32_t)a;
    int16_t c = (int16_t)b;
    int8_t d = (int8_t)c;
    hash ^= checksum(d);
    
    /* Test 4: Narrowing with shift operations */
    uint64_t shifted = 1ULL << 40;
    uint32_t narrowed_shift = (uint32_t)(shifted >> 8);
    hash ^= checksum(narrowed_shift);
    
    /* Test 5: Comparison after narrowing */
    int64_t val = 0x80000000LL;
    int32_t narrowed = (int32_t)val;
    if (narrowed > INT32_MAX - 10) {
        hash ^= 0xDEADBEEF;
    }
    
    return hash;
}

__attribute__((noinline))
unsigned test_loop_range_analysis(void) {
    unsigned hash = 0;
    
    /* Test 1: Loop with bitmasked bounds */
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += 37) {
        hash ^= checksum(i);
        if (i > 0x80000000) {
            hash ^= 0xCAFEBABE;
        }
    }
    
    /* Test 2: Nested loops with dependent bounds */
    int32_t outer_limit = 100;
    for (int32_t j = 0; j < outer_limit; j++) {
        int32_t inner_start = j & 0x3F;
        int32_t inner_end = (j * 2) | 0x1FF;
        
        for (int32_t k = inner_start; k < inner_end && k < 1000; k += (j % 7) + 1) {
            hash ^= checksum(k * j);
        }
    }
    
    /* Test 3: Loop with XOR-based bound */
    uint64_t mask = 0xFFFFFFFF00000000ULL;
    for (uint64_t x = 0; x < (mask ^ 0x0F0F0F0F0F0F0F0FULL); x += 0x100000001) {
        if (x < 1000000) {
            hash ^= checksum(x);
        } else {
            break;
        }
    }
    
    /* Test 4: Loop with shifting bound */
    uint32_t base = 0x1000;
    for (uint32_t i = 0; i < (base << 4); i += 0x10) {
        hash ^= checksum(i >> 2);
    }
    
    return hash;
}

__attribute__((noinline))
unsigned test_saturation_arithmetic(void) {
    unsigned hash = 0;
    
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
    
    /* Test saturation at boundaries */
    hash ^= checksum(sat_add(INT32_MAX, 1));
    hash ^= checksum(sat_add(INT32_MIN, -1));
    hash ^= checksum(sat_mul(INT32_MAX / 2, 3));
    hash ^= checksum(sat_mul(INT32_MIN / 2, 3));
    
    /* Test with values near boundaries */
    int32_t vals[] = {INT32_MAX - 10, INT32_MIN + 10, 0x7FFFFFF0, 0x8000000F};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            hash ^= checksum(sat_add(vals[i], vals[j]));
        }
    }
    
    /* GCC fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.7r;
    _Fract f3 = f1 + f2;
    sink = *(int*)&f3;  /* Use value to prevent elimination */
    #endif
    
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
    bf.b = -16;    /* Min for 5 signed bits */
    bf.c = 4095;   /* Max for 12 bits */
    bf.d = 524287; /* Max for 20 signed bits */
    
    hash ^= checksum(bf.a);
    hash ^= checksum(bf.b);
    hash ^= checksum(bf.c);
    hash ^= checksum(bf.d);
    
    /* Comparisons that test range analysis */
    if (bf.a > 6) hash ^= 0x11111111;
    if (bf.b < -15) hash ^= 0x22222222;
    if (bf.c == 4095) hash ^= 0x33333333;
    if (bf.d >= 524286) hash ^= 0x44444444;
    
    /* Union with bit-field overlay */
    union Overlay {
        struct BitFields bits;
        uint32_t word;
    } u;
    
    u.word = 0xFFFFFFFF;
    hash ^= checksum(u.bits.a);
    hash ^= checksum(u.bits.b);
    
    /* Bit-field in loop condition */
    struct BitFields bf2 = {0};
    for (bf2.a = 0; bf2.a < 8; bf2.a++) {
        hash ^= checksum(bf2.a * 0x100);
    }
    
    return hash;
}

__attribute__((noinline))
unsigned test_overflow_builtins(void) {
    unsigned hash = 0;
    
    /* Test overflow detection with various ranges */
    int32_t a = INT32_MAX / 2;
    int32_t b = INT32_MAX / 2 + 1;
    int32_t result;
    
    /* These should trigger overflow analysis */
    if (__builtin_add_overflow(a, b, &result)) {
        hash ^= 0xADDADDED;
    }
    
    if (__builtin_mul_overflow(a, 3, &result)) {
        hash ^= 0xMULMULED;
    }
    
    /* Test with constrained ranges */
    uint32_t x = 1000;
    uint32_t y = 2000;
    
    /* Add constraints through conditionals */
    if (x < 1500 && y < 2500) {
        uint32_t sum;
        if (!__builtin_add_overflow(x, y, &sum)) {
            hash ^= checksum(sum);
        }
    }
    
    /* Chain of overflow checks */
    int64_t large = INT64_MAX;
    int32_t small;
    if (!__builtin_sub_overflow(large, 1000, &large)) {
        if (!__builtin_add_overflow(large, 500, &large)) {
            if (__builtin_sadd_overflow((int32_t)large, 100, &small)) {
                hash ^= 0xCHAINED;
            }
        }
    }
    
    /* Overflow in loop */
    uint32_t accum = 0;
    for (int i = 0; i < 100; i++) {
        uint32_t old = accum;
        if (__builtin_add_overflow(accum, 0x10000000, &accum)) {
            hash ^= checksum(old);
            accum = 0;
        }
    }
    hash ^= checksum(accum);
    
    return hash;
}

__attribute__((noinline))
unsigned test_edge_case_comparisons(void) {
    unsigned hash = 0;
    
    /* Comparisons at type boundaries */
    int64_t extreme = INT64_MAX;
    if (extreme > INT64_MAX - 10) {
        hash ^= 0xEDGE1;
    }
    
    uint32_t uextreme = UINT32_MAX;
    if (uextreme == UINT32_MAX - 1) {
        hash ^= 0xEDGE2;
    } else if (uextreme >= UINT32_MAX) {
        hash ^= 0xEDGE3;
    }
    
    /* Value-dependent dead branches */
    int32_t x = 100;
    if (x > INT32_MAX - 50) {  /* Always false, should be analyzed */
        hash ^= 0xDEAD;
    }
    
    /* Complex boundary with bit operations */
    uint64_t val = 0xFFFFFFFFFFFFFFFFULL;
    uint64_t mask = 0x7FFFFFFFFFFFFFFFULL;
    if ((val & mask) == mask) {
        hash ^= 0xMASKED;
    }
    
    /* Shift and compare */
    int32_t shifted = 1 << 30;
    if (shifted > (INT32_MAX >> 1)) {
        hash ^= 0xSHIFTED;
    }
    
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
    sink = final_hash;
    
    printf("Hash: %u\n", final_hash);
    return 0;
}
