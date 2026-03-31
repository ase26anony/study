#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent dead code elimination */
#define KEEP(expr) do { volatile auto __v = (expr); (void)__v; } while(0)

/* Checksum to prevent optimization */
static uint64_t checksum = 0;

/* Test 1: Narrowing conversions with boundary values */
__attribute__((noinline))
static uint64_t test_narrowing_conversions(void) {
    uint64_t local_sum = 0;
    
    /* Constants at type boundaries */
    int64_t large_positive = 0x7FFFFFFFFFFFFFFFLL;
    int64_t large_negative = 0x8000000000000000LL;
    uint64_t max_unsigned = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Narrowing conversions that require range analysis */
    int32_t narrow1 = (int32_t)large_positive;  /* May overflow */
    int32_t narrow2 = (int32_t)large_negative;  /* Sign extension issues */
    uint32_t narrow3 = (uint32_t)max_unsigned;  /* Truncation */
    
    /* Arithmetic that creates values near boundaries */
    int64_t calc1 = large_positive - 100;
    int64_t calc2 = large_negative + 100;
    int32_t narrow4 = (int32_t)calc1;
    int32_t narrow5 = (int32_t)calc2;
    
    /* Shifts that may overflow the 32-bit range */
    int64_t shifted1 = 1LL << 35;
    int64_t shifted2 = 1LL << 60;
    int32_t narrow6 = (int32_t)shifted1;
    int32_t narrow7 = (int32_t)shifted2;
    
    /* Comparisons against boundaries (triggers range analysis) */
    if (narrow1 > 0x7FFFFFFF - 1000) {
        local_sum += 1;
    }
    if (narrow2 < -0x7FFFFFFF + 1000) {
        local_sum += 2;
    }
    if ((uint32_t)narrow3 > 0xFFFFFF00U) {
        local_sum += 4;
    }
    
    KEEP(narrow4); KEEP(narrow5); KEEP(narrow6); KEEP(narrow7);
    return local_sum;
}

/* Test 2: Complex loop bound analysis */
__attribute__((noinline))
static uint64_t test_loop_range_analysis(void) {
    uint64_t local_sum = 0;
    int32_t a = 1000, b = 2000, c = 100;
    
    /* Loop with bitwise operation in bound */
    for (int32_t i = a & 0xFFF; i < (b | 0x7FF); i += c) {
        local_sum += i;
        
        /* Nested loop with dependent bounds */
        for (int32_t j = (i & 0x3F); j < 1000; j += 50) {
            local_sum += j;
            
            /* Complex condition using bitwise ops */
            if ((j ^ i) > 500 && (j | i) < 1500) {
                local_sum += 10000;
            }
        }
    }
    
    /* Another loop with shifting in bounds */
    uint32_t base = 0x80000000U;
    for (uint32_t k = base >> 16; k < base >> 8; k += 256) {
        local_sum += k;
        
        /* Condition that tests boundary values */
        if (k > 0x7FFF0000U && k < 0x80010000U) {
            local_sum += k * 2;
        }
    }
    
    /* Loop with modulo operation affecting range */
    for (int32_t m = 0; m < 10000; m++) {
        int32_t constrained = m % 2048;  /* Range is now 0-2047 */
        if (constrained > 2000) {
            local_sum += constrained * 3;
        }
    }
    
    return local_sum;
}

/* Test 3: Saturation arithmetic */
__attribute__((noinline))
static uint64_t test_saturation_arithmetic(void) {
    uint64_t local_sum = 0;
    
    /* Manual saturation implementation */
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + (int64_t)b;
        if (result > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (result < -0x7FFFFFFF - 1) return -0x7FFFFFFF - 1;
        return (int32_t)result;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * (int64_t)b;
        if (result > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (result < -0x7FFFFFFF - 1) return -0x7FFFFFFF - 1;
        return (int32_t)result;
    }
    
    /* Test cases near boundaries */
    int32_t test_values[] = {
        0x70000000, 0x10000000,  // Will saturate on addition
        -0x70000000, -0x10000000, // Will saturate negative
        0x4000, 0x4000,          // Safe multiplication
        0x10000, 0x10000,        // May overflow multiplication
    };
    
    for (size_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]) - 1; i += 2) {
        int32_t saturated_add = sat_add(test_values[i], test_values[i+1]);
        int32_t saturated_mul = sat_mul(test_values[i], test_values[i+1]);
        
        local_sum += saturated_add;
        local_sum += saturated_mul;
        
        /* Conditions that test boundary comparisons */
        if (saturated_add == 0x7FFFFFFF) {
            local_sum += 0x1000;
        }
        if (saturated_add == -0x7FFFFFFF - 1) {
            local_sum += 0x2000;
        }
    }
    
    return local_sum;
}

/* Test 4: Bit-field range analysis */
__attribute__((noinline))
static uint64_t test_bitfield_ranges(void) {
    uint64_t local_sum = 0;
    
    /* Struct with various bit-field sizes */
    struct BitFields {
        unsigned int small : 4;    /* 0-15 */
        signed int signed5 : 5;    /* -16 to 15 */
        unsigned int medium : 10;  /* 0-1023 */
        signed int signed12 : 12;  /* -2048 to 2047 */
        unsigned int large : 30;   /* 0-1073741823 */
    } bf;
    
    /* Assign values at or near bit-field boundaries */
    bf.small = 15;      /* Max for 4-bit unsigned */
    bf.signed5 = -16;   /* Min for 5-bit signed */
    bf.medium = 1023;   /* Max for 10-bit unsigned */
    bf.signed12 = 2047; /* Max for 12-bit signed */
    bf.large = 0x3FFFFFFF; /* Max for 30-bit unsigned */
    
    /* Comparisons that should trigger range analysis */
    if (bf.small == 15) {
        local_sum += 1;
    }
    if (bf.signed5 < 0) {
        local_sum += 2;
    }
    if (bf.medium > 1000) {
        local_sum += 4;
    }
    if (bf.signed12 >= 2000 && bf.signed12 <= 2047) {
        local_sum += 8;
    }
    if (bf.large > 0x3FFFFFF0U) {
        local_sum += 16;
    }
    
    /* Union with overlapping bit-fields */
    union Overlap {
        struct {
            unsigned int lower : 16;
            unsigned int upper : 16;
        } parts;
        uint32_t whole;
    } u;
    
    u.whole = 0x87654321;
    if (u.parts.lower > 0x8000 && u.parts.upper < 0x9000) {
        local_sum += u.parts.lower + u.parts.upper;
    }
    
    return local_sum;
}

/* Test 5: Overflow builtins with range analysis */
__attribute__((noinline))
static uint64_t test_overflow_builtins(void) {
    uint64_t local_sum = 0;
    
    /* Variables with constrained ranges */
    int32_t constrained_a = 1000;
    int32_t constrained_b = 2000;
    
    /* Range-restricting conditions */
    if (constrained_a > 500 && constrained_a < 1500) {
        if (constrained_b > 1000 && constrained_b < 3000) {
            int overflow;
            
            /* These should not overflow given the ranges */
            int32_t sum = __builtin_add_overflow(constrained_a, constrained_b, &overflow);
            if (!overflow) {
                local_sum += sum;
            }
            
            int32_t product = __builtin_mul_overflow(constrained_a, constrained_b / 2, &overflow);
            if (!overflow) {
                local_sum += product;
            }
        }
    }
    
    /* Test with boundary values */
    int32_t boundary_values[] = {
        0x7FFFFFF0,  /* Near INT_MAX */
        0x100,
        -0x7FFFFFF0, /* Near INT_MIN */
        -0x100,
    };
    
    for (size_t i = 0; i < sizeof(boundary_values)/sizeof(boundary_values[0]) - 1; i += 2) {
        int overflow1, overflow2;
        
        /* These may overflow */
        int32_t sum = __builtin_add_overflow(boundary_values[i], boundary_values[i+1], &overflow1);
        int32_t mul = __builtin_mul_overflow(boundary_values[i], 2, &overflow2);
        
        local_sum += overflow1 ? 1 : 0;
        local_sum += overflow2 ? 2 : 0;
        local_sum += sum;
        local_sum += mul;
    }
    
    /* Loop with overflow checking */
    for (int32_t i = 0; i < 100; i++) {
        int overflow;
        int32_t value = i * 100000;
        int32_t doubled = __builtin_mul_overflow(value, 2, &overflow);
        
        if (!overflow) {
            local_sum += doubled;
        } else {
            local_sum += i;
        }
    }
    
    return local_sum;
}

/* Test 6: Additional edge cases for range analysis */
__attribute__((noinline))
static uint64_t test_edge_cases(void) {
    uint64_t local_sum = 0;
    
    /* Zero extension cases */
    uint16_t u16 = 0xFFFF;
    uint32_t u32 = u16;  /* Zero extends */
    uint64_t u64 = u32;
    
    if (u64 > 0x00000000FFFFFFFFULL) {
        local_sum += 1;
    }
    
    /* Sign extension with shifting */
    int32_t signed_val = -100;
    int64_t extended = (int64_t)signed_val;
    int64_t shifted = extended << 20;
    
    if (shifted < 0 && shifted > -0x10000000000LL) {
        local_sum += 2;
    }
    
    /* Comparisons that should use the uncovered logic */
    int64_t test_val = 0x7FFFFFFFFFFFFFFFLL;
    if (test_val > 0x7FFFFFFFFFFFFFF0LL) {
        local_sum += 4;
    }
    
    /* Bitmask operations that constrain ranges */
    uint32_t masked = 0x12345678 & 0x0000FFFF;
    if (masked <= 0xFFFF) {
        local_sum += 8;
    }
    
    return local_sum;
}

int main(void) {
    checksum = 0;
    
    checksum ^= test_narrowing_conversions();
    checksum ^= test_loop_range_analysis();
    checksum ^= test_saturation_arithmetic();
    checksum ^= test_bitfield_ranges();
    checksum ^= test_overflow_builtins();
    checksum ^= test_edge_cases();
    
    /* Use checksum to prevent optimization */
    volatile uint64_t result = checksum;
    printf("Checksum: %llu\n", (unsigned long long)result);
    
    return (int)(result & 0xFFFFFFFF);
}
