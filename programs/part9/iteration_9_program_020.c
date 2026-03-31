#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent dead code elimination */
#define KEEP(expr) do { volatile auto __v = (expr); (void)__v; } while(0)

/* Checksum/hash functions to prevent optimization */
static uint32_t __attribute__((noinline)) checksum32(const void* data, size_t len) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t hash = 0x811c9dc5;
    for (size_t i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= 0x01000193;
    }
    return hash;
}

/* Test 1: Narrowing conversions with boundary values */
static uint32_t __attribute__((noinline)) test_narrowing_conversions(void) {
    uint32_t hash = 0;
    uint64_t wide_vals[8];
    
    /* Constants at type boundaries */
    wide_vals[0] = 0xFFFFFFFFFFFFFFFFULL;  /* UINT64_MAX */
    wide_vals[1] = 0x7FFFFFFFFFFFFFFFULL;  /* INT64_MAX */
    wide_vals[2] = 0x8000000000000000ULL;  /* INT64_MIN as unsigned */
    wide_vals[3] = 0x00000000FFFFFFFFULL;  /* Fits in 32-bit */
    
    /* Variables with arithmetic that may overflow */
    for (int i = 0; i < 4; i++) {
        wide_vals[4 + i] = wide_vals[i] + (i * 0x100000000ULL);
    }
    
    /* Narrowing conversions that require range analysis */
    for (int i = 0; i < 8; i++) {
        uint32_t narrow1 = (uint32_t)wide_vals[i];  /* Explicit cast */
        int32_t narrow2 = (int32_t)wide_vals[i];    /* Signed conversion */
        
        /* Comparisons that test boundary logic */
        if (wide_vals[i] > UINT32_MAX) {
            narrow1 = 0xDEADBEEF;
        }
        if ((int64_t)wide_vals[i] > INT32_MAX || (int64_t)wide_vals[i] < INT32_MIN) {
            narrow2 = 0x7FFFFFFF;
        }
        
        hash ^= narrow1;
        hash = (hash << 5) | (hash >> 27);
        hash += narrow2;
    }
    
    /* Shift operations that may overflow */
    uint64_t shift_test = 0x1ULL << 63;
    uint32_t shifted_narrow = (uint32_t)(shift_test >> 32);
    hash ^= shifted_narrow;
    
    return hash;
}

/* Test 2: Complex loop bound analysis */
static uint32_t __attribute__((noinline)) test_loop_range_analysis(void) {
    uint32_t hash = 0;
    int32_t a = 1000, b = 2000, c = 7;
    
    /* Loop with bitwise operation in bound */
    for (int32_t i = a & 0xFFF; i < (b | 0x7FF); i += c) {
        hash += i * 3;
        
        /* Nested loop with dependent bounds */
        for (int32_t j = (i & 0x3F); j < 100; j += (c & 0x3)) {
            hash ^= j;
            
            /* Complex condition using bitwise ops */
            if ((j ^ 0x55) > (i & 0xFF)) {
                hash += 0x12345678;
            }
        }
        
        /* Break condition based on overflow check */
        if (i > 0x7FFFFFFF - 1000) {  /* Near INT32_MAX */
            break;
        }
    }
    
    /* Another loop with shifting in bounds */
    uint64_t start = 0x100000000ULL;  /* 2^32 */
    uint64_t end = 0x100000100ULL;    /* 2^32 + 256 */
    
    for (uint64_t k = start >> 16; k < end >> 16; k++) {
        hash += (uint32_t)k;
        
        /* Condition that requires range analysis */
        if (k > (UINT64_MAX >> 32)) {
            hash ^= 0xABCDEF01;
        }
    }
    
    return hash;
}

/* Test 3: Saturation arithmetic */
static int32_t __attribute__((noinline)) saturate_add(int32_t a, int32_t b) {
    int32_t sum;
    
    /* Manual saturation using comparisons at boundaries */
    if (a > 0 && b > INT32_MAX - a) {
        return INT32_MAX;
    }
    if (a < 0 && b < INT32_MIN - a) {
        return INT32_MIN;
    }
    
    sum = a + b;
    
    /* Additional boundary check */
    if (sum > 0x3FFFFFFF) {  /* 1/4 of int32 range */
        sum = 0x3FFFFFFF;
    }
    if (sum < -0x3FFFFFFF) {
        sum = -0x3FFFFFFF;
    }
    
    return sum;
}

static uint32_t __attribute__((noinline)) test_saturation_arithmetic(void) {
    uint32_t hash = 0;
    int32_t vals[8] = {
        INT32_MAX, INT32_MIN, 0,
        0x7FFFFFFF, 0x80000000,
        1000000000, -1000000000,
        0x3FFFFFFF
    };
    
    /* Test saturation with boundary values */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int32_t result = saturate_add(vals[i], vals[j]);
            hash ^= result;
            hash = (hash << 3) | (hash >> 29);
        }
    }
    
    /* Fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.75r;
    _Fract fsum = f1 + f2;
    hash += *(uint32_t*)&fsum;
    #endif
    
    return hash;
}

/* Test 4: Bit-field range analysis */
struct BitFieldStruct {
    unsigned int a : 3;   /* 0-7 */
    signed int b : 5;     /* -16 to 15 */
    unsigned int c : 12;  /* 0-4095 */
    signed int d : 20;    /* -524288 to 524287 */
};

static uint32_t __attribute__((noinline)) test_bitfield_ranges(void) {
    uint32_t hash = 0;
    struct BitFieldStruct bfs;
    
    /* Assign values at bit-field boundaries */
    bfs.a = 7;      /* Max for 3 bits */
    bfs.b = -16;    /* Min for 5-bit signed */
    bfs.c = 4095;   /* Max for 12 bits */
    bfs.d = 524287; /* Max for 20-bit signed */
    
    hash = bfs.a ^ (bfs.b << 3) ^ (bfs.c << 8) ^ (bfs.d << 20);
    
    /* Comparisons that test bit-field ranges */
    if (bfs.a > 6) {           /* Boundary check */
        hash += 0x11111111;
    }
    if (bfs.b < -15) {         /* Another boundary */
        hash ^= 0x22222222;
    }
    if (bfs.c == 4095) {       /* Exact max */
        hash |= 0x44444444;
    }
    if (bfs.d >= 524287) {     /* At max */
        hash &= 0x88888888;
    }
    
    /* Overflow in bit-field assignment */
    unsigned int overflow_test = 0xFFF;  /* 4095 */
    bfs.c = overflow_test;               /* Should fit */
    bfs.a = overflow_test;               /* Should truncate */
    
    hash += bfs.c;
    hash ^= bfs.a << 16;
    
    return hash;
}

/* Test 5: Overflow builtins with range analysis */
static uint32_t __attribute__((noinline)) test_overflow_builtins(void) {
    uint32_t hash = 0;
    int32_t a = 1000000000;
    int32_t b = 2000000000;
    int32_t result;
    int overflow;
    
    /* Basic overflow checks */
    overflow = __builtin_add_overflow(a, b, &result);
    hash ^= result;
    hash += overflow * 0x1000;
    
    overflow = __builtin_mul_overflow(a, 3, &result);
    hash ^= result;
    hash += overflow * 0x2000;
    
    /* Overflow checks in loops with constrained ranges */
    for (int32_t i = -1000; i < 1000; i += 100) {
        for (int32_t j = -1000; j < 1000; j += 100) {
            if (i > 0 && j > INT32_MAX / i) {
                /* Range analysis should detect potential overflow */
                overflow = __builtin_mul_overflow(i, j, &result);
                hash += overflow;
            } else {
                overflow = __builtin_add_overflow(i, j, &result);
                hash ^= overflow;
            }
            hash = hash * 0x5A827999 + result;
        }
    }
    
    /* Test with values near boundaries */
    int32_t boundary_vals[] = {
        INT32_MAX, INT32_MIN, 
        INT32_MAX - 100, INT32_MIN + 100,
        0x7FFFFFFF, 0x80000000
    };
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            overflow = __builtin_add_overflow(boundary_vals[i], boundary_vals[j], &result);
            hash = (hash << 1) | (hash >> 31);
            hash ^= result + overflow;
        }
    }
    
    return hash;
}

/* Main function that runs all tests */
int main(void) {
    uint32_t final_hash = 0x12345678;
    
    /* Run all tests and combine results */
    final_hash ^= test_narrowing_conversions();
    final_hash = (final_hash << 5) | (final_hash >> 27);
    
    final_hash += test_loop_range_analysis();
    final_hash = (final_hash << 13) | (final_hash >> 19);
    
    final_hash ^= test_saturation_arithmetic();
    final_hash = (final_hash << 7) | (final_hash >> 25);
    
    final_hash += test_bitfield_ranges();
    final_hash = (final_hash << 11) | (final_hash >> 21);
    
    final_hash ^= test_overflow_builtins();
    final_hash = (final_hash << 17) | (final_hash >> 15);
    
    /* Prevent optimization of final result */
    KEEP(final_hash);
    
    printf("Test hash: 0x%08X\n", final_hash);
    return (final_hash == 0) ? 1 : 0;
}
