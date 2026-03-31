#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to maintain function boundaries for coverage */
#define NOINLINE __attribute__((noinline))

/* Fixed-point types if available */
#ifdef __STDC_IEC_559__
#include <stdfix.h>
#endif

/* Checksum to prevent dead code elimination */
static uint64_t checksum = 0;

NOINLINE static void update_checksum(uint64_t val) {
    checksum ^= val;
    checksum = (checksum << 1) | (checksum >> 63);
}

/* 1. Integer Range Boundary Tests */
NOINLINE static uint64_t test_narrowing_conversions(void) {
    uint64_t local_sum = 0;
    
    /* Explicit bit-width types */
    int64_t wide_val = 0x7FFFFFFFFFFFFFFFLL;
    int32_t narrow_val;
    uint32_t u_narrow_val;
    
    /* Narrowing conversions that require range checking */
    narrow_val = (int32_t)wide_val;  /* Should trigger range analysis */
    local_sum += narrow_val;
    
    wide_val = 0xFFFFFFFF00000000LL;
    u_narrow_val = (uint32_t)wide_val;
    local_sum += u_narrow_val;
    
    /* Shifts that may overflow */
    int32_t shift_val = 0x40000000;
    shift_val <<= 2;  /* Potential overflow */
    local_sum += shift_val;
    
    /* Comparisons at type limits */
    int64_t cmp_val = 0x8000000000000000LL;
    if (cmp_val > INT64_MAX - 100) {
        local_sum += 1;
    }
    
    /* Complex narrowing with arithmetic */
    for (int64_t i = -1000; i < 1000; i++) {
        int32_t narrowed = (int32_t)(i * 123456789);
        if (narrowed > 0) local_sum += narrowed;
    }
    
    update_checksum(local_sum);
    return local_sum;
}

/* 2. Loop Bound Analysis with Complex Conditions */
NOINLINE static uint64_t test_loop_range_analysis(void) {
    uint64_t local_sum = 0;
    volatile int a = 0x1234, b = 0x5678, c = 3;
    
    /* Loop with bitwise operations in bounds */
    for (int i = a & 0xFFF; i < (b | 0x7FF); i += c) {
        local_sum += i;
        
        /* Nested loop with dependent bounds */
        for (int j = (i & 0x3F); j < 100; j += (c & 0x7)) {
            local_sum += j * i;
        }
    }
    
    /* Loop with shifting bounds */
    int shift_bound = 64;
    for (int i = 0; i < (1 << (shift_bound & 0x1F)); i++) {
        local_sum += i;
    }
    
    /* Complex loop condition with XOR */
    int x = 0xABCD, y = 0xDCBA;
    for (int i = x ^ 0xFF; i < (y | 0xFF); i = (i * 2) & 0xFFFF) {
        local_sum += i;
        if (i > 0x7FFF) break;
    }
    
    update_checksum(local_sum);
    return local_sum;
}

/* 3. Fixed-Point and Saturation Arithmetic */
NOINLINE static uint64_t test_saturation_arithmetic(void) {
    uint64_t local_sum = 0;
    
    /* Manual saturation arithmetic */
    int32_t sat_min = -0x7FFFFFFF;
    int32_t sat_max = 0x7FFFFFFF;
    
    int32_t test_vals[] = {100, -200, 0x40000000, -0x40000000, 0};
    
    for (size_t i = 0; i < sizeof(test_vals)/sizeof(test_vals[0]); i++) {
        int32_t val = test_vals[i];
        int32_t saturated;
        
        /* Saturation logic that directly tests boundary comparisons */
        if (val > sat_max) {
            saturated = sat_max;
        } else if (val < sat_min) {
            saturated = sat_min;
        } else {
            saturated = val;
        }
        local_sum += saturated;
        
        /* Multiply with saturation check */
        int32_t mult = val * 2;
        if (mult > sat_max) mult = sat_max;
        if (mult < sat_min) mult = sat_min;
        local_sum += mult;
    }
    
#ifdef __STDC_IEC_559__
    /* GCC fixed-point types if available */
    _Accum acc_val = 0.5k;
    _Fract frac_val = 0.5r;
    
    for (int i = 0; i < 10; i++) {
        acc_val += 0.1k;
        frac_val += 0.05r;
        local_sum += (uint64_t)(acc_val * 1000);
        local_sum += (uint64_t)(frac_val * 1000);
    }
#endif
    
    update_checksum(local_sum);
    return local_sum;
}

/* 4. Conditional Code with Value-Dependent Dead Branches */
NOINLINE static uint64_t test_conditional_ranges(void) {
    uint64_t local_sum = 0;
    
    /* Variables with constrained ranges */
    int32_t x = 100;
    int32_t y = -50;
    
    /* Range-restricting conditions */
    if (x > 0 && x < 1000) {
        y = x * 2;
        if (y > 150) {
            local_sum += y;
        }
    }
    
    /* Edge of type range comparisons */
    uint32_t uval = 0xFFFFFF00;
    if (uval > UINT32_MAX - 255) {
        local_sum += 1;
    }
    
    int64_t lval = 0x7FFFFFFFFFFFFFFFLL;
    if (lval > INT64_MAX - 10) {
        local_sum += 2;
    }
    
    /* Modulo-constrained variable */
    int32_t mod_val = 12345;
    mod_val = mod_val % 1000;  /* Now 0-999 */
    
    if (mod_val > 500) {
        local_sum += mod_val * 2;
    } else {
        local_sum += mod_val;
    }
    
    /* Complex conditional chain */
    int32_t a = 100, b = 200, c = 300;
    if (a < b && b < c && c < 400) {
        int32_t d = a + b + c;
        if (d > 500 && d < 1000) {
            local_sum += d;
        }
    }
    
    update_checksum(local_sum);
    return local_sum;
}

/* 5. Structs with Bit-Fields and Unions */
NOINLINE static uint64_t test_bitfield_ranges(void) {
    uint64_t local_sum = 0;
    
    /* Struct with various bit-fields */
    struct bitfields {
        unsigned int small : 4;    /* 0-15 */
        signed int signed_small : 5;  /* -16 to 15 */
        unsigned int medium : 10;  /* 0-1023 */
        unsigned int large : 20;   /* 0-1048575 */
    } bf;
    
    union overlay {
        struct bitfields bf;
        uint32_t raw;
    } u;
    
    /* Assign values at bit-field boundaries */
    bf.small = 15;      /* Max for 4 bits */
    bf.signed_small = -16; /* Min for 5 signed bits */
    bf.medium = 1023;   /* Max for 10 bits */
    bf.large = 524287;  /* Mid-range for 20 bits */
    
    u.bf = bf;
    local_sum += u.raw;
    
    /* Comparisons against bit-field capacity */
    if (bf.small == 15) {
        local_sum += 1;
    }
    
    if (bf.signed_small < 0) {
        local_sum += 2;
    }
    
    if (bf.medium > 500 && bf.medium < 1024) {
        local_sum += bf.medium;
    }
    
    /* Overflow into adjacent bit-fields */
    bf.small = 20;  /* Should wrap to 4 */
    local_sum += bf.small;
    
    update_checksum(local_sum);
    return local_sum;
}

/* 6. Compiler Builtins for Overflow Detection */
NOINLINE static uint64_t test_overflow_builtins(void) {
    uint64_t local_sum = 0;
    int overflow;
    
    /* Basic overflow checks */
    int32_t x = 0x70000000;
    int32_t y = 0x10000000;
    int32_t result;
    
    overflow = __builtin_add_overflow(x, y, &result);
    local_sum += overflow;
    local_sum += result;
    
    overflow = __builtin_mul_overflow(x, 2, &result);
    local_sum += overflow;
    local_sum += result;
    
    /* Overflow checks in loops */
    for (int i = 0; i < 100; i++) {
        int32_t a = i * 1000000;
        int32_t b = i * 500000;
        overflow = __builtin_add_overflow(a, b, &result);
        local_sum += overflow;
        if (!overflow) {
            local_sum += result;
        }
    }
    
    /* Range-constrained overflow checks */
    uint32_t ux = 0xFFFFFF00;
    uint32_t uy = 0x00000100;
    uint32_t uresult;
    
    if (ux > 0xF0000000) {
        overflow = __builtin_add_overflow(ux, uy, &uresult);
        local_sum += overflow;
        local_sum += uresult;
    }
    
    /* Chain of operations with overflow checks */
    int32_t val = 1;
    for (int i = 0; i < 10; i++) {
        overflow = __builtin_mul_overflow(val, 3, &val);
        local_sum += overflow;
        if (overflow) break;
    }
    
    update_checksum(local_sum);
    return local_sum;
}

int main(void) {
    uint64_t total = 0;
    
    printf("Running integer range analysis tests...\n");
    
    total += test_narrowing_conversions();
    total += test_loop_range_analysis();
    total += test_saturation_arithmetic();
    total += test_conditional_ranges();
    total += test_bitfield_ranges();
    total += test_overflow_builtins();
    
    printf("Total checksum: %lu\n", (unsigned long)total);
    printf("Global checksum: %lu\n", (unsigned long)checksum);
    
    return 0;
}
