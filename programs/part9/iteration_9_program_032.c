#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum function to prevent optimization */
static inline uint32_t mix(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

/* Test 1: Narrowing conversions with boundary values */
__attribute__((noinline))
uint32_t test_narrowing_conversions(void) {
    uint32_t hash = 0;
    
    /* Constants at type boundaries */
    int64_t large_vals[] = {
        INT64_MAX, INT64_MIN, 
        (int64_t)INT32_MAX + 1, 
        (int64_t)INT32_MIN - 1,
        (int64_t)UINT32_MAX,
        0x7FFFFFFF00000000LL
    };
    
    /* Narrowing conversions that require range analysis */
    for (size_t i = 0; i < sizeof(large_vals)/sizeof(large_vals[0]); i++) {
        int32_t narrowed = (int32_t)large_vals[i];
        uint32_t unarrowed = (uint32_t)large_vals[i];
        
        /* Comparisons that should trigger range checks */
        if (narrowed > INT32_MAX - 100) {
            hash = mix(hash ^ 0x11111111);
        }
        if (unarrowed < UINT32_MAX - 0xFF) {
            hash = mix(hash ^ 0x22222222);
        }
        
        /* Shifts that may overflow */
        int64_t shifted = large_vals[i] << 3;
        int32_t narrow_shifted = (int32_t)shifted;
        if (narrow_shifted > 0) {
            hash = mix(hash ^ 0x33333333);
        }
    }
    
    /* Complex narrowing with intermediate calculations */
    uint64_t a = 0xFFFFFFFFULL;
    uint64_t b = 0x80000000ULL;
    uint64_t c = a - b;
    uint32_t d = (uint32_t)c;
    
    if (d > 0x7FFFFFFF) {
        hash = mix(hash ^ 0x44444444);
    }
    
    sink = hash;
    return hash;
}

/* Test 2: Loop bound analysis with complex conditions */
__attribute__((noinline))
uint32_t test_loop_range_analysis(void) {
    uint32_t hash = 0;
    
    /* Outer loop with bitmasked bounds */
    for (int32_t i = 100; i < 500; i += 37) {
        /* Complex bound calculation using bitwise ops */
        int32_t lower = i & 0x3FF;  /* Mask to 10 bits */
        int32_t upper = (i | 0x7FF) & 0xFFF;  /* Complex upper bound */
        
        /* Inner loop with dependent bounds */
        for (int32_t j = lower; j < upper; j += (i & 0x1F) + 1) {
            /* XOR pattern to create complex value flow */
            int32_t k = j ^ (i << 3);
            
            /* Comparisons at boundary conditions */
            if (k > INT32_MAX - 1000) {
                hash = mix(hash ^ (uint32_t)j);
            }
            if (k < INT32_MIN + 1000) {
                hash = mix(hash ^ (uint32_t)(-j));
            }
            
            /* Nested condition with range-dependent branch */
            if ((j & 0xFF) < 128) {
                int32_t shifted = k << 2;
                if (shifted > 0 && shifted < INT32_MAX >> 2) {
                    hash = mix(hash ^ 0x55555555);
                }
            }
        }
    }
    
    /* Loop with wrap-around analysis */
    uint32_t counter = 0;
    for (uint32_t x = 0xFFFFFF00; x < 0xFFFFFFFF; x++) {
        counter++;
        /* This should trigger analysis of x near UINT32_MAX */
        if (x > 0xFFFFFFF0) {
            hash = mix(hash ^ x);
        }
    }
    hash = mix(hash ^ counter);
    
    sink = hash;
    return hash;
}

/* Test 3: Saturation arithmetic */
__attribute__((noinline))
uint32_t test_saturation_arithmetic(void) {
    uint32_t hash = 0;
    
    /* Manual saturation implementation */
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
    
    /* Test cases near boundaries */
    int32_t test_cases[][2] = {
        {INT32_MAX, 1},
        {INT32_MAX, -1},
        {INT32_MIN, -1},
        {INT32_MIN, 1},
        {1000000000, 1000000000},
        {-1000000000, -1000000000},
        {0x7FFFFFFF, 2},
        {0x80000000, 2}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        int32_t a = test_cases[i][0];
        int32_t b = test_cases[i][1];
        
        int32_t sum = sat_add(a, b);
        int32_t prod = sat_mul(a, b);
        
        /* Comparisons that should trigger boundary analysis */
        if (sum == INT32_MAX) {
            hash = mix(hash ^ 0x66666666);
        }
        if (sum == INT32_MIN) {
            hash = mix(hash ^ 0x77777777);
        }
        if (prod > INT32_MAX - 100) {
            hash = mix(hash ^ 0x88888888);
        }
    }
    
    /* Fixed-point style arithmetic (emulated) */
    int32_t fixed_mul(int32_t a, int32_t b, int shift) {
        int64_t result = (int64_t)a * (int64_t)b;
        result >>= shift;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    int32_t fp_result = fixed_mul(0x7FFFFFFF, 0x7FFFFFFF, 30);
    if (fp_result > 0) {
        hash = mix(hash ^ (uint32_t)fp_result);
    }
    
    sink = hash;
    return hash;
}

/* Test 4: Bit-field range analysis */
__attribute__((noinline))
uint32_t test_bitfield_ranges(void) {
    uint32_t hash = 0;
    
    /* Struct with various bit-fields */
    struct BitFieldStruct {
        unsigned int a : 3;   /* 0-7 */
        signed int b : 5;     /* -16 to 15 */
        unsigned int c : 12;  /* 0-4095 */
        signed int d : 20;    /* -524288 to 524287 */
        unsigned int e : 1;   /* 0-1 */
    } bfs;
    
    /* Test assignments at and beyond bit-field capacities */
    unsigned int test_values[] = {0, 1, 7, 8, 15, 16, 4095, 4096, 0xFFFFF};
    
    for (size_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        /* These assignments require range analysis */
        bfs.a = test_values[i] & 0x7;
        bfs.b = (test_values[i] & 0x1F) - 16;  /* Signed range */
        bfs.c = test_values[i] & 0xFFF;
        bfs.d = (test_values[i] & 0xFFFFF) - 0x80000;  /* Center around 0 */
        bfs.e = test_values[i] & 0x1;
        
        /* Comparisons that test bit-field boundaries */
        if (bfs.a == 7) {  /* Max for 3-bit unsigned */
            hash = mix(hash ^ 0x99999999);
        }
        if (bfs.b == -16 || bfs.b == 15) {  /* Min/Max for 5-bit signed */
            hash = mix(hash ^ 0xAAAAAAAA);
        }
        if (bfs.c > 4000) {  /* Near 12-bit max */
            hash = mix(hash ^ 0xBBBBBBBB);
        }
        if (bfs.d < -500000 || bfs.d > 500000) {  /* Near 20-bit signed bounds */
            hash = mix(hash ^ 0xCCCCCCCC);
        }
    }
    
    /* Union with bit-fields for type-punning */
    union BitFieldUnion {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } parts;
        uint32_t whole;
    } u;
    
    u.whole = 0x87654321;
    if (u.parts.high == 0x8765 && u.parts.low == 0x4321) {
        hash = mix(hash ^ u.whole);
    }
    
    /* Complex condition with bit-field extraction */
    uint32_t val = 0xABCD1234;
    unsigned int nibble = (val >> 28) & 0xF;  /* 4-bit extract */
    if (nibble > 0xA) {  /* Compare against 10 */
        hash = mix(hash ^ val);
    }
    
    sink = hash;
    return hash;
}

/* Test 5: Overflow builtins with range analysis */
__attribute__((noinline))
uint32_t test_overflow_builtins(void) {
    uint32_t hash = 0;
    
    /* Variables with constrained ranges */
    int32_t x = 1000000000;
    int32_t y = 2000000000;
    
    /* Overflow checks that depend on range analysis */
    int32_t result;
    if (__builtin_add_overflow(x, y, &result)) {
        hash = mix(hash ^ 0xDDDDDDDD);
    } else {
        hash = mix(hash ^ (uint32_t)result);
    }
    
    /* Multiplication with overflow check */
    int32_t a = 0x7FFFFFFF;
    int32_t b = 2;
    if (__builtin_mul_overflow(a, b, &result)) {
        hash = mix(hash ^ 0xEEEEEEEE);
    }
    
    /* Loop with overflow builtins */
    for (int32_t i = 0; i < 100; i++) {
        int32_t base = INT32_MAX - 50;
        int32_t increment = i * 2;
        
        if (!__builtin_add_overflow(base, increment, &result)) {
            if (result > INT32_MAX - 10) {
                hash = mix(hash ^ (uint32_t)i);
            }
        }
    }
    
    /* Unsigned overflow checks */
    uint32_t u1 = UINT32_MAX - 100;
    uint32_t u2 = 200;
    uint32_t uresult;
    if (__builtin_add_overflow(u1, u2, &uresult)) {
        hash = mix(hash ^ 0xFFFFFFFF);
    } else {
        hash = mix(hash ^ uresult);
    }
    
    /* Chained operations with overflow detection */
    int32_t acc = 1;
    for (int i = 1; i <= 20; i++) {
        int32_t old_acc = acc;
        if (__builtin_mul_overflow(acc, i, &acc)) {
            hash = mix(hash ^ (uint32_t)old_acc);
            break;
        }
    }
    hash = mix(hash ^ (uint32_t)acc);
    
    sink = hash;
    return hash;
}

/* Main function that runs all tests */
int main(void) {
    uint32_t final_hash = 0;
    
    final_hash ^= test_narrowing_conversions();
    final_hash ^= test_loop_range_analysis();
    final_hash ^= test_saturation_arithmetic();
    final_hash ^= test_bitfield_ranges();
    final_hash ^= test_overflow_builtins();
    
    /* Use result to prevent optimization */
    sink = final_hash;
    
    return (int)(final_hash & 0x7FFFFFFF);
}
