#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum function to prevent optimization */
static inline int mix(int a, int b) {
    return (a * 31) ^ b;
}

/* ========== Test 1: Narrowing Conversions ========== */
__attribute__((noinline))
int test_narrowing_conversions(void) {
    int checksum = 0;
    
    /* Constants at type boundaries */
    int64_t wide_vals[] = {
        INT64_MAX,
        INT64_MIN,
        INT32_MAX + 1LL,
        INT32_MIN - 1LL,
        UINT32_MAX,
        0x7FFFFFFF00000000LL,
        0x8000000000000000LL
    };
    
    /* Narrowing conversions that require range analysis */
    for (size_t i = 0; i < sizeof(wide_vals)/sizeof(wide_vals[0]); i++) {
        int32_t narrow = (int32_t)wide_vals[i];
        checksum = mix(checksum, narrow);
        
        /* Comparisons against boundaries */
        if (wide_vals[i] > INT32_MAX) {
            checksum += 1;
        }
        if (wide_vals[i] < INT32_MIN) {
            checksum += 2;
        }
        
        /* Shift operations that may overflow */
        int64_t shifted = wide_vals[i] << 3;
        int32_t narrow_shifted = (int32_t)shifted;
        checksum = mix(checksum, narrow_shifted);
    }
    
    /* Complex narrowing with intermediate calculations */
    uint64_t a = 0xFFFFFFFFULL;
    uint64_t b = 0x100000000ULL;
    uint32_t c = (uint32_t)(a * b / 1000);
    checksum = mix(checksum, c);
    
    sink = checksum;
    return checksum;
}

/* ========== Test 2: Loop Range Analysis ========== */
__attribute__((noinline))
int test_loop_range_analysis(void) {
    int checksum = 0;
    
    /* Outer loop with bitmasked bounds */
    for (int32_t i = 100 & 0xFFF; i < (500 | 0x7FF); i += 17) {
        checksum += i;
        
        /* Inner loop with dependent bounds */
        for (int32_t j = i & 0x3F; j < (i ^ 0x1FF); j += (i % 13) + 1) {
            checksum = mix(checksum, j);
            
            /* Complex condition that requires range analysis */
            if ((j > (INT32_MAX - 100)) || (j < (INT32_MIN + 100))) {
                checksum ^= 0x5555;
            }
        }
        
        /* Loop with shifting bound */
        int32_t k = i << 2;
        while (k > 0 && k < (i * 4)) {
            checksum = mix(checksum, k);
            k = (k * 3) >> 1;
            
            /* Boundary comparison at extreme values */
            if (k > 0x3FFFFFFF) {
                checksum += 0x1000;
            }
        }
    }
    
    /* Nested loops with modulo operations */
    for (uint32_t x = 0x80000000; x < 0x80001000; x += 0x100) {
        for (uint32_t y = x & 0xFFF; y < (x | 0xFFF); y += (x % 256) + 1) {
            checksum = mix(checksum, (int)y);
            
            /* Comparison that tests the uncovered logic */
            if (y > 0xFFFFFFFF - 1000) {
                checksum ^= 0xAAAA;
            }
        }
    }
    
    sink = checksum;
    return checksum;
}

/* ========== Test 3: Saturation Arithmetic ========== */
__attribute__((noinline))
int test_saturation_arithmetic(void) {
    int checksum = 0;
    
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
    
    /* Test saturation with boundary values */
    int32_t test_vals[] = {
        INT32_MAX, INT32_MIN, INT32_MAX - 100,
        INT32_MIN + 100, 0, 1000, -1000
    };
    
    for (size_t i = 0; i < sizeof(test_vals)/sizeof(test_vals[0]); i++) {
        for (size_t j = 0; j < sizeof(test_vals)/sizeof(test_vals[0]); j++) {
            int32_t sum = sat_add(test_vals[i], test_vals[j]);
            int32_t prod = sat_mul(test_vals[i], test_vals[j]);
            checksum = mix(checksum, sum);
            checksum = mix(checksum, prod);
        }
    }
    
    /* Fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.7r;
    _Fract f3 = f1 + f2;
    checksum = mix(checksum, *(int32_t*)&f3);
    #endif
    
    sink = checksum;
    return checksum;
}

/* ========== Test 4: Bitfield Ranges ========== */
__attribute__((noinline))
int test_bitfield_ranges(void) {
    int checksum = 0;
    
    /* Struct with various bitfields */
    struct BitfieldStruct {
        unsigned int a : 3;   /* 0-7 */
        signed int b : 5;     /* -16 to 15 */
        unsigned int c : 12;  /* 0-4095 */
        signed int d : 20;    /* -524288 to 524287 */
        unsigned int e : 1;   /* 0-1 */
    } bs;
    
    /* Union to test bitfield range analysis */
    union BitfieldUnion {
        struct BitfieldStruct bs;
        uint64_t raw;
    } bu;
    
    /* Assign boundary values to bitfields */
    bu.bs.a = 7;      /* Max for 3 bits */
    bu.bs.b = -16;    /* Min for 5-bit signed */
    bu.bs.c = 4095;   /* Max for 12 bits */
    bu.bs.d = 524287; /* Max for 20-bit signed */
    bu.bs.e = 1;      /* Max for 1 bit */
    
    checksum = mix(checksum, bu.raw & 0xFFFFFFFF);
    checksum = mix(checksum, bu.raw >> 32);
    
    /* Comparisons that require bitfield range analysis */
    if (bu.bs.a > 6) checksum += 1;
    if (bu.bs.b < -15) checksum += 2;
    if (bu.bs.c >= 4094) checksum += 4;
    if (bu.bs.d <= -524287) checksum += 8;
    if (bu.bs.e == 1) checksum += 16;
    
    /* Complex bitfield operations */
    for (unsigned int val = 0; val < 16; val++) {
        bu.bs.a = val & 0x7;  /* Automatically truncated to 3 bits */
        bu.bs.b = (val * 2) - 16;
        bu.bs.c = val * 256;
        
        /* Conditions that test the uncovered comparison logic */
        if (bu.bs.a == 7 && bu.bs.b == -2 && bu.bs.c > 2048) {
            checksum ^= 0x1234;
        }
    }
    
    sink = checksum;
    return checksum;
}

/* ========== Test 5: Overflow Builtins ========== */
__attribute__((noinline))
int test_overflow_builtins(void) {
    int checksum = 0;
    
    /* Test values at boundaries */
    int32_t boundary_vals[] = {
        INT32_MAX, INT32_MIN, INT32_MAX - 10,
        INT32_MIN + 10, 1000000000, -1000000000
    };
    
    /* Overflow detection with builtins */
    for (size_t i = 0; i < sizeof(boundary_vals)/sizeof(boundary_vals[0]); i++) {
        for (size_t j = 0; j < sizeof(boundary_vals)/sizeof(boundary_vals[0]); j++) {
            int32_t result;
            int overflow;
            
            /* Addition overflow */
            overflow = __builtin_add_overflow(boundary_vals[i], boundary_vals[j], &result);
            checksum = mix(checksum, result);
            if (overflow) checksum ^= 0x1;
            
            /* Multiplication overflow */
            overflow = __builtin_mul_overflow(boundary_vals[i], boundary_vals[j], &result);
            checksum = mix(checksum, result);
            if (overflow) checksum ^= 0x2;
            
            /* Subtraction overflow */
            overflow = __builtin_sub_overflow(boundary_vals[i], boundary_vals[j], &result);
            checksum = mix(checksum, result);
            if (overflow) checksum ^= 0x4;
        }
    }
    
    /* Overflow in loops with range-restricted values */
    for (int32_t x = INT32_MAX - 100; x < INT32_MAX; x += 10) {
        for (int32_t y = 2; y < 10; y++) {
            int32_t sum, diff, prod;
            
            /* These operations may or may not overflow depending on range analysis */
            if (__builtin_add_overflow(x, y, &sum)) {
                checksum += 0x10;
            } else {
                checksum = mix(checksum, sum);
            }
            
            if (__builtin_mul_overflow(x, y, &prod)) {
                checksum += 0x20;
            } else {
                checksum = mix(checksum, prod);
            }
            
            /* Complex condition that requires understanding value ranges */
            if (x > INT32_MAX - 50 && y > 5) {
                int32_t temp;
                if (!__builtin_add_overflow(x, y * 2, &temp)) {
                    checksum = mix(checksum, temp);
                }
            }
        }
    }
    
    sink = checksum;
    return checksum;
}

/* ========== Test 6: Complex Conditional Ranges ========== */
__attribute__((noinline))
int test_complex_conditionals(void) {
    int checksum = 0;
    
    /* Variables with constrained ranges */
    int32_t x = 1000;
    int32_t y = -1000;
    
    /* Chain of conditions that constrain value ranges */
    if (x > 500 && x < 1500) {
        checksum += x;
        /* x is now known to be in [501, 1499] */
        
        if (x % 2 == 0) {
            x = x * 2;  /* Could overflow? */
            checksum = mix(checksum, x);
        }
    }
    
    if (y < -500 && y > -1500) {
        checksum += y;
        /* y is now known to be in [-1499, -501] */
        
        if (y & 0x100) {
            y = y / 2;
            checksum = mix(checksum, y);
        }
    }
    
    /* Complex boundary comparisons */
    uint64_t big = 0xFFFFFFFF00000000ULL;
    for (uint32_t i = 0; i < 1000; i += 100) {
        uint64_t val = big + i;
        
        /* These comparisons directly test the uncovered double_int logic */
        if (val > UINT32_MAX) {
            checksum += 1;
        }
        if (val < 0x100000000ULL) {
            checksum += 2;
        }
        if (val >= 0xFFFFFFFFFFFFFFF0ULL) {
            checksum += 4;
        }
        
        /* Narrowing with explicit check */
        if (val <= UINT32_MAX) {
            uint32_t narrow = (uint32_t)val;
            checksum = mix(checksum, narrow);
        }
    }
    
    sink = checksum;
    return checksum;
}

/* ========== Main Function ========== */
int main(void) {
    int final_checksum = 0;
    
    final_checksum = mix(final_checksum, test_narrowing_conversions());
    final_checksum = mix(final_checksum, test_loop_range_analysis());
    final_checksum = mix(final_checksum, test_saturation_arithmetic());
    final_checksum = mix(final_checksum, test_bitfield_ranges());
    final_checksum = mix(final_checksum, test_overflow_builtins());
    final_checksum = mix(final_checksum, test_complex_conditionals());
    
    /* Use the result to prevent optimization */
    sink = final_checksum;
    
    return final_checksum & 0xFF;
}
