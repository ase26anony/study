#include <stdint.h>
#include <stdio.h>
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

/* ========== 1. Integer Range Boundary Tests ========== */
__attribute__((noinline))
unsigned int test_narrowing_conversions(void) {
    unsigned int hash = 0;
    uint64_t wide_values[] = {
        0x7FFFFFFFULL,      /* INT_MAX */
        0x80000000ULL,      /* INT_MIN as unsigned */
        0xFFFFFFFFULL,      /* -1 in 32-bit */
        0x100000000ULL,     /* Just beyond 32-bit */
        0x7FFFFFFFFFFFFFFFULL, /* LLONG_MAX for 64-bit */
    };
    
    /* Narrowing conversions with explicit casts */
    for (size_t i = 0; i < sizeof(wide_values)/sizeof(wide_values[0]); i++) {
        uint64_t wide = wide_values[i];
        
        /* These should trigger range analysis */
        int32_t narrow1 = (int32_t)wide;
        uint32_t narrow2 = (uint32_t)wide;
        int16_t narrow3 = (int16_t)wide;
        
        /* Comparisons at boundaries */
        int in_range_32 = (wide <= 0x7FFFFFFFULL);
        int fits_unsigned = (wide <= 0xFFFFFFFFULL);
        
        hash ^= narrow1;
        hash = (hash << 5) | (hash >> 27);
        hash ^= narrow2;
        hash = (hash << 5) | (hash >> 27);
        hash ^= narrow3;
        hash = (hash << 5) | (hash >> 27);
        hash ^= in_range_32;
        hash = (hash << 5) | (hash >> 27);
        hash ^= fits_unsigned;
    }
    
    /* Shifts that may overflow */
    int32_t x = 0x40000000;
    int32_t y = x << 1;  /* Potential overflow */
    int32_t z = x << 2;  /* Definite overflow for signed */
    
    hash ^= y ^ z;
    return hash;
}

/* ========== 2. Loop Bound Analysis ========== */
__attribute__((noinline))
unsigned int test_loop_range_analysis(void) {
    unsigned int hash = 0;
    
    /* Complex loop conditions with bitwise operations */
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    uint32_t c = 17;
    
    /* Outer loop with mask operation */
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += c) {
        /* Inner loop with dependent bounds */
        uint32_t inner_start = i & 0xFF;
        uint32_t inner_end = (i >> 8) & 0x7F;
        
        for (uint32_t j = inner_start; j < inner_end + 10; j++) {
            /* XOR pattern to create checksum */
            hash ^= (i << 16) | j;
            hash = (hash << 3) | (hash >> 29);
        }
        
        /* Additional condition with XOR */
        if ((i ^ 0x555) > 0x1000) {
            hash += i * 3;
        }
    }
    
    /* Nested loops with shifting bounds */
    int32_t base = 100;
    for (int32_t k = -base; k < base; k += 3) {
        /* Bound depends on k with sign extension considerations */
        int32_t limit = (k < 0) ? -k : k;
        limit = limit & 0x3F;  /* Mask to 6 bits */
        
        for (int32_t m = 0; m < limit * 2; m++) {
            hash ^= (k << 8) | (m & 0xFF);
        }
    }
    
    return hash;
}

/* ========== 3. Saturation Arithmetic ========== */
__attribute__((noinline))
unsigned int test_saturation_arithmetic(void) {
    unsigned int hash = 0;
    
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
    int32_t test_cases[][2] = {
        {0x7FFFFFFF, 1},      /* Would overflow */
        {0x40000000, 2},      /* Would overflow */
        {-0x80000000, -1},    /* Would underflow */
        {0x3FFFFFFF, 2},      /* Safe */
        {1000, 2000},         /* Safe */
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        int32_t sum = sat_add(test_cases[i][0], test_cases[i][1]);
        int32_t prod = sat_mul(test_cases[i][0], test_cases[i][1]);
        
        hash ^= sum;
        hash = (hash << 7) | (hash >> 25);
        hash ^= prod;
        hash = (hash << 7) | (hash >> 25);
    }
    
    /* Fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.7r;
    _Accum a1 = 100.0k;
    _Accum a2 = 200.0k;
    
    /* Operations that might saturate */
    _Fract fsum = f1 + f2;
    _Accum asum = a1 + a2;
    
    hash ^= *(unsigned int*)&fsum;
    hash ^= *(unsigned int*)&asum;
    #endif
    
    return hash;
}

/* ========== 4. Bit-Field Range Analysis ========== */
__attribute__((noinline))
unsigned int test_bitfield_ranges(void) {
    unsigned int hash = 0;
    
    /* Struct with various bit-fields */
    struct BitFieldStruct {
        signed int small_signed : 5;    /* -16 to 15 */
        unsigned int small_unsigned : 6; /* 0 to 63 */
        signed int medium_signed : 12;   /* -2048 to 2047 */
        unsigned int medium_unsigned : 13; /* 0 to 8191 */
        int normal_int;
    } bfs;
    
    /* Initialize */
    memset(&bfs, 0, sizeof(bfs));
    
    /* Assign values near boundaries */
    bfs.small_signed = 15;      /* Max positive for 5-bit signed */
    bfs.small_unsigned = 63;    /* Max for 6-bit unsigned */
    bfs.medium_signed = -2048;  /* Min for 12-bit signed */
    bfs.medium_unsigned = 8191; /* Max for 13-bit unsigned */
    bfs.normal_int = 0x12345678;
    
    /* Conditional checks that require range analysis */
    if (bfs.small_signed > 10) {
        hash ^= 0x1111;
    }
    if (bfs.small_unsigned < 70) {  /* Always true due to bit-field width */
        hash ^= 0x2222;
    }
    if (bfs.medium_signed == -2048) {
        hash ^= 0x3333;
    }
    if (bfs.medium_unsigned >= 8000) {
        hash ^= 0x4444;
    }
    
    /* Union with bit-fields for type punning */
    union BitFieldUnion {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } parts;
        uint32_t whole;
    } u;
    
    u.whole = 0xDEADBEEF;
    if (u.parts.low > 0x7FFF) {
        hash ^= u.parts.low;
    }
    if (u.parts.high <= 0xBEEF) {
        hash ^= u.parts.high;
    }
    
    return hash;
}

/* ========== 5. Overflow Builtins ========== */
__attribute__((noinline))
unsigned int test_overflow_builtins(void) {
    unsigned int hash = 0;
    
    int32_t values[] = {
        0x7FFFFFF0,  /* Near INT_MAX */
        0x1000,
        -0x7FFFFFF0, /* Near INT_MIN */
        0x200,
        0x40000000,
    };
    
    /* Test overflow builtins with partially known ranges */
    for (size_t i = 0; i < sizeof(values)/sizeof(values[0]) - 1; i++) {
        int32_t a = values[i];
        int32_t b = values[i + 1];
        int32_t result;
        int overflow;
        
        /* Addition with overflow check */
        overflow = __builtin_add_overflow(a, b, &result);
        hash ^= result;
        hash = (hash << 11) | (hash >> 21);
        hash ^= overflow;
        
        /* Multiplication with overflow check */
        overflow = __builtin_mul_overflow(a, b, &result);
        hash ^= result;
        hash = (hash << 11) | (hash >> 21);
        hash ^= overflow;
        
        /* Subtraction with overflow check */
        overflow = __builtin_sub_overflow(a, b, &result);
        hash ^= result;
        hash = (hash << 11) | (hash >> 21);
        hash ^= overflow;
    }
    
    /* Nested overflow checks in conditional */
    int32_t x = 1000000;
    int32_t y = 2000000;
    int32_t z = 3000000;
    
    int of1, of2;
    int32_t r1, r2;
    
    of1 = __builtin_add_overflow(x, y, &r1);
    if (!of1) {
        of2 = __builtin_add_overflow(r1, z, &r2);
        if (!of2) {
            hash ^= r2;
        } else {
            hash ^= 0x55555555;
        }
    } else {
        hash ^= 0xAAAAAAAA;
    }
    
    return hash;
}

/* ========== 6. Edge Case Conditionals ========== */
__attribute__((noinline))
unsigned int test_edge_conditions(void) {
    unsigned int hash = 0;
    
    /* Variables with constrained ranges */
    int32_t x = 100;
    int32_t y = 200;
    
    /* Early range restriction */
    if (x > 50 && x < 150) {
        /* x is known to be in [51, 149] */
        if (x > 100) {
            /* Further refinement */
            y = x * 2;
            if (y > 250) {
                hash ^= 0x11111111;
            }
        }
    }
    
    /* Comparisons at extreme edges */
    uint64_t big = 0xFFFFFFFFFFFFFFFFULL;
    if (big > 0xFFFFFFFFULL) {
        hash ^= 0x22222222;
    }
    
    /* Modulo creates known range */
    for (int i = 0; i < 1000; i++) {
        int mod = i % 256;  /* Known to be 0-255 */
        if (mod > 300) {    /* Always false, dead code potential */
            hash ^= 0xDEAD;
        }
        if (mod < 0) {      /* Always false */
            hash ^= 0xBEEF;
        }
    }
    
    return hash;
}

/* ========== Main Function ========== */
int main(void) {
    unsigned int final_hash = 0;
    
    /* Run all tests */
    final_hash ^= test_narrowing_conversions();
    final_hash = (final_hash << 13) | (final_hash >> 19);
    
    final_hash ^= test_loop_range_analysis();
    final_hash = (final_hash << 13) | (final_hash >> 19);
    
    final_hash ^= test_saturation_arithmetic();
    final_hash = (final_hash << 13) | (final_hash >> 19);
    
    final_hash ^= test_bitfield_ranges();
    final_hash = (final_hash << 13) | (final_hash >> 19);
    
    final_hash ^= test_overflow_builtins();
    final_hash = (final_hash << 13) | (final_hash >> 19);
    
    final_hash ^= test_edge_conditions();
    
    /* Use result to prevent optimization */
    sink = final_hash;
    
    printf("Hash: %u\n", final_hash);
    return 0;
}
