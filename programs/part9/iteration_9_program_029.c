#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum function to prevent optimization */
static inline unsigned int checksum(unsigned int x) {
    return x * 1103515245 + 12345;
}

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline))
unsigned int test_narrowing_conversions(void) {
    unsigned int hash = 0;
    
    /* Test 1: Narrowing from 64-bit to 32-bit with boundary values */
    uint64_t wide_val = 0xFFFFFFFFULL;
    int32_t narrow1 = (int32_t)wide_val;  /* Should be -1 */
    int32_t narrow2 = (int32_t)(wide_val >> 1);  /* Should be 0x7FFFFFFF */
    int32_t narrow3 = (int32_t)(wide_val + 1);  /* Overflow in 64-bit */
    
    hash = checksum(hash + narrow1);
    hash = checksum(hash + narrow2);
    hash = checksum(hash + narrow3);
    
    /* Test 2: Shifts that may overflow */
    int32_t shift_val = 0x40000000;
    int32_t shifted1 = shift_val << 1;  /* Becomes 0x80000000 */
    int32_t shifted2 = shift_val << 2;  /* Overflow in signed */
    int32_t shifted3 = shift_val >> 31;  /* Boundary shift */
    
    hash = checksum(hash + shifted1);
    hash = checksum(hash + shifted2);
    hash = checksum(hash + shifted3);
    
    /* Test 3: Comparisons at type limits */
    int64_t large_neg = -0x8000000000000000LL;
    int64_t large_pos = 0x7FFFFFFFFFFFFFFFLL;
    
    if (narrow1 > (int32_t)(large_pos >> 32)) hash = checksum(hash + 1);
    if (narrow2 < (int32_t)(large_neg >> 32)) hash = checksum(hash + 2);
    
    return hash;
}

__attribute__((noinline))
unsigned int test_loop_range_analysis(void) {
    unsigned int hash = 0;
    
    /* Complex loop bounds with bitwise operations */
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    uint32_t c = 0x3FF;
    
    /* Outer loop with mask operation */
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += (c & 0x1FF)) {
        /* Inner loop with dependent bounds */
        for (uint32_t j = (i ^ 0x555) & 0x3FF; 
             j < ((i | 0x1FF) + 0x100); 
             j += (i & 0x3F) + 1) {
            hash = checksum(hash + j);
            
            /* Early exit based on bit pattern */
            if ((j & 0xF00) == 0xF00) {
                break;
            }
        }
        
        /* Loop variant with shifting bound */
        int32_t k;
        for (k = (int32_t)(i << 2); 
             k > (int32_t)((i & 0x7F) - 0x40); 
             k -= (i & 0x1F) + 1) {
            hash = checksum(hash + (unsigned int)k);
            
            /* Conditional that depends on range analysis */
            if (k > 0x3FFFFFFF || k < -0x40000000) {
                hash = checksum(hash + 0xDEAD);
            }
        }
        
        if (i > 0xF000) {
            break;
        }
    }
    
    /* Another loop with signed/unsigned mixing */
    int32_t start = -1000;
    uint32_t limit = 2000;
    
    for (int32_t idx = start; (uint32_t)idx < limit; idx += (idx & 0x7F) + 1) {
        hash = checksum(hash + (unsigned int)idx);
        
        /* Nested condition with range implications */
        if (idx > 0) {
            for (uint32_t m = 0; m < (uint32_t)(idx & 0xFF); m++) {
                hash = checksum(hash + m);
            }
        }
    }
    
    return hash;
}

__attribute__((noinline))
unsigned int test_saturation_arithmetic(void) {
    unsigned int hash = 0;
    
    /* Manual saturation arithmetic */
    int32_t sat_min = -0x80000000;
    int32_t sat_max = 0x7FFFFFFF;
    
    /* Test saturation addition */
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + (int64_t)b;
        if (result > sat_max) return sat_max;
        if (result < sat_min) return sat_min;
        return (int32_t)result;
    }
    
    /* Test saturation multiplication */
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * (int64_t)b;
        if (result > sat_max) return sat_max;
        if (result < sat_min) return sat_min;
        return (int32_t)result;
    }
    
    /* Boundary test cases */
    int32_t test_cases[][2] = {
        {sat_max, 1},
        {sat_min, -1},
        {sat_max / 2, 2},
        {sat_min / 2, 2},
        {0x40000000, 0x40000000},
        {-0x40000000, -0x40000000}
    };
    
    for (int i = 0; i < 6; i++) {
        int32_t sum = sat_add(test_cases[i][0], test_cases[i][1]);
        int32_t prod = sat_mul(test_cases[i][0], test_cases[i][1]);
        
        hash = checksum(hash + (unsigned int)sum);
        hash = checksum(hash + (unsigned int)prod);
        
        /* Conditional that tests saturation logic */
        if (sum == sat_max || sum == sat_min) {
            hash = checksum(hash + 0xBAD);
        }
        if (prod == sat_max || prod == sat_min) {
            hash = checksum(hash + 0xF00D);
        }
    }
    
    /* Fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.75r;
    _Accum a1 = 10.5k;
    _Accum a2 = 20.25k;
    
    /* Fixed-point operations that may saturate */
    _Fract f_sum = f1 + f2;
    _Accum a_prod = a1 * a2;
    
    hash = checksum(hash + *(unsigned int*)&f_sum);
    hash = checksum(hash + *(unsigned int*)&a_prod);
    #endif
    
    return hash;
}

__attribute__((noinline))
unsigned int test_bitfield_ranges(void) {
    unsigned int hash = 0;
    
    /* Struct with various bit-fields */
    struct BitFieldStruct {
        signed int small_signed : 5;
        unsigned int small_unsigned : 4;
        signed int medium_signed : 12;
        unsigned int medium_unsigned : 11;
        signed int large_signed : 20;
        unsigned int padding : 20;
    } bfs;
    
    /* Union to test bit-field extraction */
    union BitFieldUnion {
        struct BitFieldStruct bf;
        uint64_t raw;
    } u;
    
    memset(&bfs, 0, sizeof(bfs));
    
    /* Assign boundary values to bit-fields */
    bfs.small_signed = 0xF;      /* Should be -1 in 5-bit signed */
    bfs.small_unsigned = 0xF;    /* Should be 15 in 4-bit unsigned */
    bfs.medium_signed = 0x7FF;   /* Max positive for 12-bit signed */
    bfs.medium_unsigned = 0x7FF; /* Max for 11-bit unsigned */
    bfs.large_signed = 0x7FFFF;  /* Large positive value */
    
    u.bf = bfs;
    hash = checksum(hash + (unsigned int)u.raw);
    hash = checksum(hash + (unsigned int)(u.raw >> 32));
    
    /* Comparisons that test bit-field ranges */
    if (bfs.small_signed > 0) {
        hash = checksum(hash + 1);
    }
    if (bfs.small_unsigned < 20) {  /* Always true for 4-bit */
        hash = checksum(hash + 2);
    }
    if (bfs.medium_signed == 0x7FF) {
        hash = checksum(hash + 3);
    }
    if (bfs.large_signed > 0x40000) {  /* Test against value outside range */
        hash = checksum(hash + 4);
    }
    
    /* Bit-field in conditional expressions */
    int result = (bfs.small_signed < bfs.medium_signed) ? 
                 bfs.small_unsigned : bfs.medium_unsigned;
    hash = checksum(hash + result);
    
    /* Bit-field arithmetic */
    bfs.medium_signed = bfs.small_signed * 8;  /* May overflow 12-bit */
    bfs.large_signed = bfs.medium_unsigned << 10;  /* Shift within 20-bit */
    
    u.bf = bfs;
    hash = checksum(hash + (unsigned int)u.raw);
    
    return hash;
}

__attribute__((noinline))
unsigned int test_overflow_builtins(void) {
    unsigned int hash = 0;
    
    /* Test overflow builtins with range-constrained values */
    int32_t a = 0x40000000;
    int32_t b = 0x40000000;
    int32_t result;
    int overflow;
    
    /* Multiplication that will overflow */
    overflow = __builtin_mul_overflow(a, b, &result);
    hash = checksum(hash + result);
    hash = checksum(hash + overflow);
    
    /* Addition with boundary values */
    int32_t c = 0x7FFFFFFF;
    int32_t d = 1;
    overflow = __builtin_add_overflow(c, d, &result);
    hash = checksum(hash + result);
    hash = checksum(hash + overflow);
    
    /* Subtraction with minimum value */
    int32_t e = -0x80000000;
    int32_t f = 1;
    overflow = __builtin_sub_overflow(e, f, &result);
    hash = checksum(hash + result);
    hash = checksum(hash + overflow);
    
    /* Test in loops with varying ranges */
    for (int32_t i = -1000; i < 1000; i += 97) {
        for (int32_t j = -500; j < 500; j += 73) {
            int32_t sum, prod;
            int ovf1, ovf2;
            
            ovf1 = __builtin_add_overflow(i, j, &sum);
            ovf2 = __builtin_mul_overflow(i, j, &prod);
            
            hash = checksum(hash + sum);
            hash = checksum(hash + prod);
            hash = checksum(hash + ovf1);
            hash = checksum(hash + ovf2);
            
            /* Conditional based on overflow results */
            if (ovf1 || ovf2) {
                hash = checksum(hash + 0xCAFE);
            }
        }
    }
    
    /* Test with values derived from bit operations */
    uint32_t base = 0x87654321;
    for (int i = 0; i < 32; i += 4) {
        int32_t x = (base >> i) & 0xF;
        int32_t y = (base >> (31 - i)) & 0xF;
        
        overflow = __builtin_add_overflow(x * 0x10000000, 
                                         y * 0x10000000, 
                                         &result);
        hash = checksum(hash + result);
        hash = checksum(hash + overflow);
    }
    
    return hash;
}

__attribute__((noinline))
unsigned int test_edge_case_conditions(void) {
    unsigned int hash = 0;
    
    /* Edge case comparisons that trigger range analysis */
    int32_t x, y;
    
    /* Constrained ranges from earlier conditions */
    x = 100;
    if (x > 50 && x < 150) {
        /* x is known to be in [51, 149] */
        y = x * 2;
        
        /* This comparison should be analyzable */
        if (y > 300 || y < 100) {
            hash = checksum(hash + 1);
        }
        
        /* Comparison against constant near limit */
        if (y > INT32_MAX - 100) {
            hash = checksum(hash + 2);
        }
    }
    
    /* Modulo-constrained value */
    uint32_t mod_val = 0x12345678;
    uint32_t constrained = mod_val % 1000;  /* In [0, 999] */
    
    if (constrained > 500) {
        hash = checksum(hash + constrained);
        
        /* Further constraint */
        if (constrained < 600) {
            uint32_t scaled = constrained * 0x100000;
            if (scaled > 0x80000000) {
                hash = checksum(hash + 3);
            }
        }
    }
    
    /* Chain of constraints */
    int32_t val = -500;
    if (val < 0) {
        val = -val;  /* Now in [0, 500] */
        if (val < 256) {
            val = val << 2;  /* Now in [0, 1020] */
            if (val > 512) {
                hash = checksum(hash + val);
            }
        }
    }
    
    /* Comparison with extreme constants */
    int64_t big_val = 0x7FFFFFFFFFFFFFFFLL;
    if ((int32_t)big_val > INT32_MAX - 10) {
        hash = checksum(hash + 4);
    }
    
    if ((int32_t)(big_val >> 32) < INT32_MIN + 10) {
        hash = checksum(hash + 5);
    }
    
    return hash;
}

int main(void) {
    unsigned int final_hash = 0;
    
    sink = 0;
    
    final_hash = checksum(final_hash + test_narrowing_conversions());
    final_hash = checksum(final_hash + test_loop_range_analysis());
    final_hash = checksum(final_hash + test_saturation_arithmetic());
    final_hash = checksum(final_hash + test_bitfield_ranges());
    final_hash = checksum(final_hash + test_overflow_builtins());
    final_hash = checksum(final_hash + test_edge_case_conditions());
    
    /* Use the result to prevent optimization */
    sink = final_hash;
    
    printf("Result: %u\n", final_hash);
    
    return 0;
}
