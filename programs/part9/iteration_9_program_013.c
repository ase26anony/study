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
static unsigned test_narrowing_conversions(void) {
    unsigned int hash = 0;
    uint64_t wide_values[] = {
        0xFFFFFFFFFFFFFFFFULL,  /* Max uint64_t */
        0x8000000000000000ULL,  /* Min int64_t */
        0x7FFFFFFFFFFFFFFFULL,  /* Max int64_t */
        0x00000000FFFFFFFFULL,  /* Fits in uint32_t */
        0x0000000080000000ULL,  /* Fits in int32_t (negative) */
    };
    
    /* Test narrowing with explicit casts */
    for (size_t i = 0; i < sizeof(wide_values)/sizeof(wide_values[0]); i++) {
        uint32_t narrow1 = (uint32_t)wide_values[i];
        int32_t narrow2 = (int32_t)wide_values[i];
        int16_t narrow3 = (int16_t)wide_values[i];
        
        /* Comparisons at type boundaries */
        int result1 = narrow1 > 0x7FFFFFFF;
        int result2 = narrow2 < -0x7FFFFFFF;
        int result3 = narrow3 == 0x7FFF || narrow3 == -0x8000;
        
        hash = hash * 31 + result1;
        hash = hash * 31 + result2;
        hash = hash * 31 + result3;
    }
    
    /* Shift operations that may overflow */
    int32_t shift_test = 0x40000000;
    for (int shift = 0; shift < 34; shift++) {
        int64_t shifted = (int64_t)shift_test << shift;
        int in_range = (shifted >= -0x80000000LL && shifted <= 0x7FFFFFFFLL);
        hash = hash * 31 + in_range;
    }
    
    return hash;
}

/* ========== 2. Loop Bound Analysis ========== */
__attribute__((noinline))
static unsigned test_loop_range_analysis(void) {
    unsigned int hash = 0;
    
    /* Complex loop bounds with bitwise operations */
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    uint32_t c = 17;
    
    /* Outer loop with mask operation */
    for (uint32_t i = a & 0x0000FFFF; i < (b | 0x00007FFF); i += c) {
        if (i > 0x80000000) {
            /* Inner loop with dependent bounds */
            for (uint32_t j = i ^ 0x55555555; j < (i & 0x0FFFFFFF); j += 3) {
                hash = hash * 31 + j;
            }
        }
    }
    
    /* Nested loops with shifting bounds */
    for (int32_t x = -1000; x < 1000; x += 97) {
        int32_t lower = x & 0x3FF;
        int32_t upper = (x | 0x1FF) + 512;
        
        /* Loop where bounds analysis is complex */
        for (int32_t y = lower; y < upper; y += (x & 0x1F) + 1) {
            if (y > 0x7FFFFF00 || y < -0x7FFFFF00) {
                hash = hash * 31 + (y & 0xFF);
            }
        }
    }
    
    return hash;
}

/* ========== 3. Saturation Arithmetic ========== */
__attribute__((noinline))
static unsigned test_saturation_arithmetic(void) {
    unsigned int hash = 0;
    
    /* Manual saturation arithmetic */
    int32_t sat_min = -0x80000000;
    int32_t sat_max = 0x7FFFFFFF;
    
    int32_t test_values[] = {
        0x3FFFFFFF, 0x40000000, 0x7FFFFFFF,
        -0x3FFFFFFF, -0x40000000, -0x80000000
    };
    
    for (size_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        for (size_t j = 0; j < sizeof(test_values)/sizeof(test_values[0]); j++) {
            int64_t result64 = (int64_t)test_values[i] + (int64_t)test_values[j];
            
            /* Saturation logic - exactly the kind of boundary check we want */
            int32_t saturated;
            if (result64 > sat_max) {
                saturated = sat_max;
            } else if (result64 < sat_min) {
                saturated = sat_min;
            } else {
                saturated = (int32_t)result64;
            }
            
            hash = hash * 31 + saturated;
        }
    }
    
    /* Fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.75r;
    _Fract fsum = f1 + f2;
    hash = hash * 31 + *(unsigned*)&fsum;
    #endif
    
    return hash;
}

/* ========== 4. Bit-Field Ranges ========== */
__attribute__((noinline))
static unsigned test_bitfield_ranges(void) {
    unsigned int hash = 0;
    
    /* Struct with various bit-fields */
    struct {
        unsigned int a : 5;   /* 0-31 */
        signed int b : 7;     /* -64 to 63 */
        unsigned int c : 12;  /* 0-4095 */
        signed int d : 20;    /* -524288 to 524287 */
    } bits;
    
    /* Assign values at or near boundaries */
    unsigned test_cases[][4] = {
        {31, 63, 4095, 524287},
        {0, -64, 0, -524288},
        {16, 0, 2048, 0},
        {31, -32, 4095, 262144}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        bits.a = test_cases[i][0];
        bits.b = test_cases[i][1];
        bits.c = test_cases[i][2];
        bits.d = test_cases[i][3];
        
        /* Comparisons that require range analysis */
        int check1 = bits.a == 31;          /* Max of 5 bits */
        int check2 = bits.b < -32;          /* In lower half of range */
        int check3 = bits.c > 2047;         /* Above midpoint */
        int check4 = bits.d == -524288;     /* Min of 20-bit signed */
        
        hash = hash * 31 + check1 + check2 * 2 + check3 * 4 + check4 * 8;
    }
    
    /* Union with bit-field and integer */
    union {
        struct {
            unsigned low : 16;
            unsigned high : 16;
        } parts;
        uint32_t whole;
    } converter;
    
    converter.whole = 0x87654321;
    int range_check = (converter.parts.low > 0x7FFF) || 
                     (converter.parts.high < 0x8000);
    hash = hash * 31 + range_check;
    
    return hash;
}

/* ========== 5. Overflow Builtins ========== */
__attribute__((noinline))
static unsigned test_overflow_builtins(void) {
    unsigned int hash = 0;
    
    int32_t values[] = {
        0x3FFFFFFF, 0x40000000, 0x7FFFFFFF,
        -0x3FFFFFFF, -0x40000000, -0x80000000
    };
    
    for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); i++) {
        for (size_t j = 0; j < sizeof(values)/sizeof(values[0]); j++) {
            int32_t result;
            int overflow;
            
            /* Test addition overflow */
            overflow = __builtin_add_overflow(values[i], values[j], &result);
            hash = hash * 31 + overflow;
            hash = hash * 31 + result;
            
            /* Test multiplication overflow */
            if (values[i] != 0 && values[j] != 0) {
                overflow = __builtin_mul_overflow(values[i], values[j], &result);
                hash = hash * 31 + overflow;
                hash = hash * 31 + (result & 0xFF);
            }
        }
    }
    
    /* Overflow in loops with constrained ranges */
    uint32_t x = 0xFFFFFF00;
    for (int i = 0; i < 300; i++) {
        uint32_t old_x = x;
        int ovf = __builtin_add_overflow(x, 1, &x);
        
        /* Conditional based on overflow detection */
        if (ovf || x < old_x) {
            hash = hash * 31 + i;
        }
    }
    
    return hash;
}

/* ========== Main Function ========== */
int main(void) {
    unsigned int final_hash = 0;
    
    printf("Running integer range analysis tests...\n");
    
    final_hash ^= test_narrowing_conversions();
    printf("  Narrowing conversions: %08x\n", final_hash);
    
    final_hash ^= test_loop_range_analysis();
    printf("  Loop range analysis:   %08x\n", final_hash);
    
    final_hash ^= test_saturation_arithmetic();
    printf("  Saturation arithmetic: %08x\n", final_hash);
    
    final_hash ^= test_bitfield_ranges();
    printf("  Bit-field ranges:      %08x\n", final_hash);
    
    final_hash ^= test_overflow_builtins();
    printf("  Overflow builtins:     %08x\n", final_hash);
    
    printf("Final hash: %08x\n", final_hash);
    
    /* Use result to prevent optimization */
    sink = final_hash;
    
    return final_hash != 0 ? 0 : 1;
}
