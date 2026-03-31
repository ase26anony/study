#include <stdint.h>
#include <stddef.h>
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

/* ========== Test 1: Narrowing Conversions ========== */
__attribute__((noinline))
unsigned int test_narrowing_conversions(void) {
    uint64_t results[16];
    int idx = 0;
    
    /* Constants at boundary values */
    int64_t large_positive = 0x7FFFFFFFFFFFFFFFLL;
    int64_t large_negative = 0x8000000000000000LL;
    
    /* Test 1: Direct narrowing with boundary values */
    int32_t narrow1 = (int32_t)large_positive;  /* Should trigger range analysis */
    int32_t narrow2 = (int32_t)large_negative;
    results[idx++] = narrow1;
    results[idx++] = narrow2;
    
    /* Test 2: Arithmetic before narrowing */
    uint64_t a = 0xFFFFFFFFFFFFFFF0ULL;
    uint64_t b = 0x0000000000000020ULL;
    uint32_t sum_narrow = (uint32_t)(a + b);  /* Overflow in 64-bit, then narrow */
    results[idx++] = sum_narrow;
    
    /* Test 3: Shift then narrow */
    int64_t shifted = 0x1LL << 48;
    int32_t shifted_narrow = (int32_t)(shifted >> 16);  /* Complex range analysis */
    results[idx++] = shifted_narrow;
    
    /* Test 4: Conditional narrowing based on comparison */
    int64_t var = 0x7FFFFFFF00000000LL;
    int32_t cond_narrow = (var > 0x7FFFFFFFLL) ? (int32_t)var : 0;
    results[idx++] = cond_narrow;
    
    /* Test 5: Loop with narrowing conversions */
    for (int64_t i = 0x7FFFFFFF80000000LL; i < 0x7FFFFFFF80000010LL; i++) {
        results[idx++] = (int32_t)i;  /* Each iteration requires range check */
        if (idx >= 16) break;
    }
    
    return checksum(results, sizeof(results));
}

/* ========== Test 2: Loop Range Analysis ========== */
__attribute__((noinline))
unsigned int test_loop_range_analysis(void) {
    unsigned int results[32] = {0};
    int idx = 0;
    
    /* Complex loop bounds with bitwise operations */
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    
    /* Outer loop: bounds depend on masked values */
    for (uint32_t i = a & 0x0000FFFF; i < (b | 0x00007FFF); i += 0x100) {
        /* Inner loop: bounds depend on outer index */
        for (uint32_t j = (i ^ 0xFF) & 0x3FF; j < ((i + 0x100) & 0x7FF); j += 0x40) {
            results[idx++ % 32] ^= j;
        }
        
        /* Another inner loop with shift-based bounds */
        uint32_t shift_bound = (i << 2) & 0xFFF;
        for (uint32_t k = i >> 4; k < shift_bound; k += (i & 0x3F) + 1) {
            results[idx++ % 32] += k;
        }
        
        if (idx >= 100) break; /* Prevent infinite loops */
    }
    
    /* Loop with signed bounds that may wrap */
    int32_t start = 0x70000000;
    int32_t end = 0x7FFFFFF0;
    for (int32_t i = start; i < end; i += 0x1000000) {
        results[idx++ % 32] |= (unsigned int)i;
    }
    
    /* Nested loops with multiplication in bounds */
    for (int m = 1; m < 8; m++) {
        for (int n = m * 16; n < m * 32; n += m) {
            results[idx++ % 32] += n;
            if (idx >= 32) goto done;
        }
    }
    
done:
    return checksum(results, sizeof(results));
}

/* ========== Test 3: Saturation Arithmetic ========== */
__attribute__((noinline))
unsigned int test_saturation_arithmetic(void) {
    int32_t results[16];
    int idx = 0;
    
    /* Manual saturation arithmetic */
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t sum = (int64_t)a + (int64_t)b;
        if (sum > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (sum < (int64_t)0x80000000) return 0x80000000;
        return (int32_t)sum;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t prod = (int64_t)a * (int64_t)b;
        if (prod > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (prod < (int64_t)0x80000000) return 0x80000000;
        return (int32_t)prod;
    }
    
    /* Test saturation at boundaries */
    results[idx++] = sat_add(0x70000000, 0x10000000);  /* Would overflow */
    results[idx++] = sat_add(0x80000000, -0x10000000); /* Would underflow */
    results[idx++] = sat_add(0x40000000, 0x40000000);  /* Exact boundary */
    
    results[idx++] = sat_mul(0x10000, 0x10000);        /* Would overflow */
    results[idx++] = sat_mul(0x8000, 0x8000);          /* Exact fit */
    results[idx++] = sat_mul(0xC0000000, 0x2);         /* Would underflow */
    
    /* Fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.7r;
    _Fract f3 = f1 + f2;  /* May saturate */
    results[idx++] = *(int32_t*)&f3;
    #endif
    
    /* Accumulator saturation */
    int32_t acc = 0;
    for (int i = 0; i < 100; i++) {
        int32_t old_acc = acc;
        acc = sat_add(acc, 0x1000000);
        if (acc == old_acc) break;  /* Saturated */
        results[idx++ % 16] = acc;
    }
    
    return checksum(results, sizeof(results));
}

/* ========== Test 4: Bit-Field Ranges ========== */
__attribute__((noinline))
unsigned int test_bitfield_ranges(void) {
    struct BitFields {
        unsigned int a : 3;   /* 0-7 */
        signed int b : 5;     /* -16 to 15 */
        unsigned int c : 12;  /* 0-4095 */
        signed int d : 20;    /* -524288 to 524287 */
        unsigned int e : 1;   /* 0-1 */
    } bf;
    
    unsigned int results[8] = {0};
    int idx = 0;
    
    /* Assign values at boundaries */
    bf.a = 7;      /* Max for 3 bits */
    bf.b = -16;    /* Min for 5 signed bits */
    bf.c = 4095;   /* Max for 12 bits */
    bf.d = 524287; /* Max for 20 signed bits */
    bf.e = 1;      /* Max for 1 bit */
    
    /* Comparisons that test range understanding */
    if (bf.a == 7) results[idx++] = 1;
    if (bf.b < 0) results[idx++] = 2;
    if (bf.c >= 4095) results[idx++] = 4;
    if (bf.d > 500000) results[idx++] = 8;
    if (bf.e != 0) results[idx++] = 16;
    
    /* Operations that may overflow bit-field */
    bf.a = bf.a + 1;  /* Should wrap to 0 */
    bf.b = bf.b - 1;  /* Should wrap to 15 */
    bf.c = bf.c * 2;  /* Should wrap */
    
    results[idx++] = bf.a;
    results[idx++] = bf.b;
    results[idx++] = bf.c;
    
    /* Union with overlapping bit-fields */
    union {
        struct {
            unsigned int x : 10;
            unsigned int y : 10;
            unsigned int z : 12;
        } bits;
        uint32_t word;
    } u;
    
    u.word = 0xFFFFFFFF;
    if (u.bits.x == 0x3FF) results[idx++] = 0x100;
    if (u.bits.y == 0x3FF) results[idx++] = 0x200;
    if (u.bits.z == 0xFFF) results[idx++] = 0x400;
    
    return checksum(results, sizeof(results));
}

/* ========== Test 5: Overflow Builtins ========== */
__attribute__((noinline))
unsigned int test_overflow_builtins(void) {
    unsigned int results[16];
    int idx = 0;
    int overflow;
    
    /* Test with constants at boundaries */
    int32_t x = 0x7FFFFFF0;
    int32_t y = 0x00000020;
    
    overflow = __builtin_add_overflow(x, y, &results[idx++]);
    results[idx++] = overflow;
    
    overflow = __builtin_mul_overflow(x, 2, &results[idx++]);
    results[idx++] = overflow;
    
    overflow = __builtin_sub_overflow(0x80000000, 1, &results[idx++]);
    results[idx++] = overflow;
    
    /* Test in loops with varying ranges */
    for (int32_t i = 0x70000000; i < 0x700000F0; i += 0x10) {
        int32_t sum;
        overflow = __builtin_add_overflow(i, 0x10000000, &sum);
        results[idx++ % 16] = sum;
        results[idx++ % 16] = overflow;
    }
    
    /* Chain of operations with overflow checks */
    int32_t val = 1000;
    for (int i = 0; i < 10; i++) {
        int32_t new_val;
        if (!__builtin_mul_overflow(val, 3, &new_val)) {
            val = new_val;
        } else {
            val = 0x7FFFFFFF;  /* Saturate on overflow */
        }
        results[idx++ % 16] = val;
    }
    
    /* Unsigned overflow checks */
    uint32_t u1 = 0xFFFFFF00;
    uint32_t u2 = 0x00000100;
    uint32_t u_result;
    
    overflow = __builtin_uadd_overflow(u1, u2, &u_result);
    results[idx++ % 16] = u_result;
    results[idx++ % 16] = overflow;
    
    return checksum(results, sizeof(results));
}

/* ========== Test 6: Conditional Dead Branches ========== */
__attribute__((noinline))
unsigned int test_conditional_dead_branches(void) {
    unsigned int results[8] = {0};
    int idx = 0;
    
    /* Variables with constrained ranges */
    int32_t x = 100;
    int32_t y = 200;
    
    /* First, constrain ranges */
    if (x > 0 && x < 1000) {
        if (y > 150 && y < 250) {
            /* Compiler now knows ranges: x in [1,999], y in [151,249] */
            
            /* These comparisons test boundary analysis */
            if (x + y > 1000) results[idx++] = 1;  /* Always true? */
            if (x * 2 < 500) results[idx++] = 2;   /* Possibly true */
            if (y - x > 0x7FFFFFF0) results[idx++] = 4; /* Always false */
            
            /* Modulo creates known range */
            int32_t z = x % 128;  /* z in [0,127] */
            if (z > 200) results[idx++] = 8;  /* Dead branch */
            if (z < 128) results[idx++] = 16; /* Always true */
        }
    }
    
    /* Edge case comparisons */
    int64_t big = 0x7FFFFFFFFFFFFFFFLL;
    if (big > 0x7FFFFFFFFFFFFFF0LL) results[idx++] = 32;
    
    int32_t neg = -0x80000000;
    if (neg == 0x80000000) results[idx++] = 64;  /* Tricky comparison */
    
    /* Range from shift operations */
    uint32_t shifted = 0x1U << 31;
    if (shifted > 0x80000000U) results[idx++] = 128;  /* Always false? */
    
    return checksum(results, sizeof(results));
}

/* ========== Main Function ========== */
int main(void) {
    unsigned int final_hash = 0;
    
    /* Run all tests */
    final_hash ^= test_narrowing_conversions();
    final_hash ^= test_loop_range_analysis();
    final_hash ^= test_saturation_arithmetic();
    final_hash ^= test_bitfield_ranges();
    final_hash ^= test_overflow_builtins();
    final_hash ^= test_conditional_dead_branches();
    
    /* Use sink to prevent optimization */
    sink = final_hash;
    
    return 0;
}
