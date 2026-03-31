#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum function to prevent optimization */
__attribute__((noinline)) 
uint64_t compute_checksum(uint64_t val) {
    return val ^ (val >> 32);
}

/* Test 1: Narrowing conversions with boundary values */
__attribute__((noinline))
uint64_t test_narrowing_conversions(void) {
    uint64_t checksum = 0;
    
    /* Test with values at type boundaries */
    int64_t large_vals[] = {
        INT64_MAX,
        INT64_MIN,
        (int64_t)UINT32_MAX,
        (int64_t)UINT32_MAX + 1,
        (int64_t)INT32_MAX,
        (int64_t)INT32_MIN,
        0x7FFFFFFF00000000LL,
        0x8000000000000000LL
    };
    
    for (size_t i = 0; i < sizeof(large_vals)/sizeof(large_vals[0]); i++) {
        /* Narrowing conversions that require range analysis */
        int32_t narrow1 = (int32_t)large_vals[i];
        uint32_t narrow2 = (uint32_t)large_vals[i];
        int16_t narrow3 = (int16_t)large_vals[i];
        
        checksum ^= compute_checksum((uint64_t)narrow1);
        checksum ^= compute_checksum((uint64_t)narrow2);
        checksum ^= compute_checksum((uint64_t)narrow3);
        
        /* Comparisons against boundaries */
        if (large_vals[i] > INT32_MAX) {
            checksum += 1;
        }
        if (large_vals[i] < INT32_MIN) {
            checksum += 2;
        }
        if ((uint64_t)large_vals[i] > UINT32_MAX) {
            checksum += 4;
        }
    }
    
    return checksum;
}

/* Test 2: Complex loop bound analysis */
__attribute__((noinline))
uint64_t test_loop_range_analysis(void) {
    uint64_t checksum = 0;
    
    /* Outer loop with bitwise-derived bounds */
    for (int32_t outer = 100; outer < 500; outer += 37) {
        /* Complex bound calculation using bitwise ops */
        uint32_t mask = 0xFFF;
        uint32_t start = outer & mask;
        uint32_t end = (outer | 0x7FF) & 0xFFFF;
        
        /* Inner loop with dependent bounds */
        for (uint32_t inner = start; inner < end; inner += (outer & 0x3F) + 1) {
            /* Mix of signed and unsigned comparisons */
            if ((int32_t)inner > outer) {
                checksum += inner * 3;
            } else {
                checksum += inner * 7;
            }
            
            /* Nested condition with range-dependent check */
            if (inner > 0x8000 && inner < 0x9000) {
                checksum ^= (inner << 16);
            }
        }
        
        /* Another loop with shifting bounds */
        int64_t base = (int64_t)outer * 1000000LL;
        for (int64_t j = base; j < base + 1000; j += 17) {
            /* Comparison that requires double_int range analysis */
            if (j > INT32_MAX || j < INT32_MIN) {
                checksum += j & 0xFFFF;
            }
        }
    }
    
    /* Loop with wrap-around analysis */
    uint32_t counter = 0;
    for (int i = 0; i < 1000; i++) {
        counter = (counter + 1) & 0x1F;  /* Constrained to 5 bits */
        checksum += counter;
        
        /* Condition testing boundary values */
        if (counter == 0) checksum += 0x1000;
        if (counter == 0x1F) checksum += 0x2000;
    }
    
    return checksum;
}

/* Test 3: Saturation arithmetic */
__attribute__((noinline))
uint64_t test_saturation_arithmetic(void) {
    uint64_t checksum = 0;
    
    /* Manual saturation implementation */
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + (int64_t)b;
        
        /* These comparisons trigger the uncovered boundary checks */
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * (int64_t)b;
        
        /* Boundary comparisons for multiplication */
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    /* Test cases near boundaries */
    int32_t test_cases[] = {
        INT32_MAX, INT32_MIN, 0,
        INT32_MAX - 100, INT32_MIN + 100,
        10000, -10000, 65535, -65535
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        for (size_t j = 0; j < sizeof(test_cases)/sizeof(test_cases[0]); j++) {
            int32_t sum = sat_add(test_cases[i], test_cases[j]);
            int32_t prod = sat_mul(test_cases[i], test_cases[j]);
            
            checksum ^= compute_checksum((uint64_t)sum);
            checksum ^= compute_checksum((uint64_t)prod);
        }
    }
    
    /* Fixed-point arithmetic if supported */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.75r;
    _Fract f3 = f1 + f2;
    checksum ^= *(uint32_t*)&f3;
    #endif
    
    return checksum;
}

/* Test 4: Bit-field range analysis */
__attribute__((noinline))
uint64_t test_bitfield_ranges(void) {
    uint64_t checksum = 0;
    
    /* Struct with various bit-fields */
    struct BitFields {
        unsigned int a : 3;   /* 0-7 */
        signed int b : 5;     /* -16 to 15 */
        unsigned int c : 12;  /* 0-4095 */
        signed int d : 20;    /* -524288 to 524287 */
    } bf;
    
    /* Assign values and test boundaries */
    for (unsigned int i = 0; i < 20; i++) {
        bf.a = i & 0x7;      /* Constrained to 3 bits */
        bf.b = (i - 10) & 0x1F; /* Constrained to 5 bits, signed */
        bf.c = i * 100;      /* May exceed 12 bits */
        bf.d = i * 10000;    /* May exceed 20 bits */
        
        /* Comparisons that require bit-field range analysis */
        if (bf.a == 7) checksum += 1;
        if (bf.a == 0) checksum += 2;
        
        if (bf.b > 10) checksum += 4;
        if (bf.b < -10) checksum += 8;
        
        if (bf.c > 2047) checksum += 16;
        if (bf.c < 100) checksum += 32;
        
        if (bf.d > 262144) checksum += 64;
        if (bf.d < -262144) checksum += 128;
    }
    
    /* Union with bit-fields and regular integers */
    union BitUnion {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } bits;
        uint32_t full;
    } bu;
    
    bu.full = 0x87654321;
    if (bu.bits.low > 0x7FFF) checksum += 0x100;
    if (bu.bits.high < 0x8000) checksum += 0x200;
    
    return checksum;
}

/* Test 5: Overflow builtins with range analysis */
__attribute__((noinline))
uint64_t test_overflow_builtins(void) {
    uint64_t checksum = 0;
    
    int32_t vals[] = {
        0, 1, -1, 100, -100,
        INT32_MAX, INT32_MIN,
        INT32_MAX / 2, INT32_MIN / 2
    };
    
    /* Test overflow detection with partially known ranges */
    for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); i++) {
        for (size_t j = 0; j < sizeof(vals)/sizeof(vals[0]); j++) {
            int32_t result;
            int overflow;
            
            /* Addition overflow */
            overflow = __builtin_add_overflow(vals[i], vals[j], &result);
            checksum ^= compute_checksum((uint64_t)result);
            if (overflow) checksum += 0x1000;
            
            /* Multiplication overflow */
            overflow = __builtin_mul_overflow(vals[i], vals[j], &result);
            checksum ^= compute_checksum((uint64_t)result);
            if (overflow) checksum += 0x2000;
            
            /* Subtraction overflow */
            overflow = __builtin_sub_overflow(vals[i], vals[j], &result);
            checksum ^= compute_checksum((uint64_t)result);
            if (overflow) checksum += 0x4000;
        }
    }
    
    /* Overflow checks in loops with constrained ranges */
    for (int32_t x = -1000; x < 1000; x += 77) {
        /* Constrain range further with condition */
        int32_t y = (x > 0) ? (x & 0xFF) : (x | 0x7F);
        
        int32_t sum;
        if (__builtin_add_overflow(x, y, &sum)) {
            checksum += x;
        } else {
            checksum += sum;
        }
        
        /* Chain of operations */
        int32_t temp = x * 3;
        if (!__builtin_add_overflow(temp, y, &sum)) {
            if (!__builtin_mul_overflow(sum, 2, &temp)) {
                checksum ^= temp;
            }
        }
    }
    
    return checksum;
}

/* Test 6: Additional boundary condition tests */
__attribute__((noinline))
uint64_t test_boundary_conditions(void) {
    uint64_t checksum = 0;
    
    /* Direct tests of boundary comparisons */
    for (int64_t i = -10; i <= 10; i++) {
        int64_t val = INT64_MAX + i;
        
        /* These should trigger the uncovered sgt/ugt comparisons */
        if (val > INT64_MAX - 100) {
            checksum += 1;
        }
        if ((uint64_t)val > UINT64_MAX - 100) {
            checksum += 2;
        }
        if (val < INT64_MIN + 100) {
            checksum += 4;
        }
    }
    
    /* Shift operations with overflow checking */
    for (uint32_t i = 0; i < 40; i++) {
        uint64_t shifted = 1ULL << i;
        
        /* Comparisons against shifted boundaries */
        if (shifted > 0xFFFFFFFFULL) {
            checksum += shifted >> 32;
        }
        if (shifted < 0x1000) {
            checksum += i;
        }
    }
    
    return checksum;
}

int main(void) {
    uint64_t final_checksum = 0;
    
    printf("Running integer range analysis tests...\n");
    
    final_checksum ^= test_narrowing_conversions();
    printf("  test_narrowing_conversions: %016llx\n", (unsigned long long)final_checksum);
    
    final_checksum ^= test_loop_range_analysis();
    printf("  test_loop_range_analysis:   %016llx\n", (unsigned long long)final_checksum);
    
    final_checksum ^= test_saturation_arithmetic();
    printf("  test_saturation_arithmetic: %016llx\n", (unsigned long long)final_checksum);
    
    final_checksum ^= test_bitfield_ranges();
    printf("  test_bitfield_ranges:       %016llx\n", (unsigned long long)final_checksum);
    
    final_checksum ^= test_overflow_builtins();
    printf("  test_overflow_builtins:     %016llx\n", (unsigned long long)final_checksum);
    
    final_checksum ^= test_boundary_conditions();
    printf("  test_boundary_conditions:   %016llx\n", (unsigned long long)final_checksum);
    
    printf("Final checksum: %016llx\n", (unsigned long long)final_checksum);
    
    /* Use sink to prevent optimization */
    sink = (final_checksum > 0);
    
    return sink ? 0 : 1;
}
