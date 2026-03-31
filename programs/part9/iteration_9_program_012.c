#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum function to prevent optimization */
static inline int checksum(int val) {
    return val ^ (val >> 16);
}

/* GCC fixed-point types if available */
#ifdef __STDC_IEC_559__
typedef _Fract gcc_fract;
typedef _Accum gcc_accum;
#endif

/* ========== Test 1: Narrowing Conversions ========== */
__attribute__((noinline))
int test_narrowing_conversions(void) {
    int result = 0;
    
    /* Test 1a: Constants at boundary edges */
    int64_t large_const = 0x7FFFFFFFFFFFFFFFLL;  /* INT64_MAX */
    int32_t narrow1 = (int32_t)large_const;      /* Should trigger range analysis */
    result ^= narrow1;
    
    /* Test 1b: Variable with known range */
    uint64_t x = 0xFFFFFFFF00000000ULL;
    for (int i = 0; i < 4; i++) {
        uint32_t y = (uint32_t)(x >> (i * 8));  /* Different shift amounts */
        result += y;
    }
    
    /* Test 1c: Arithmetic before narrowing */
    int64_t a = 1000, b = 2000;
    int64_t sum = a * b;  /* 2,000,000 - fits in 32-bit */
    int32_t narrow_sum = (int32_t)sum;
    result ^= narrow_sum;
    
    /* Test 1d: Boundary case with sign extension */
    int16_t s16 = -32768;  /* INT16_MIN */
    int32_t s32 = s16;      /* Sign extends */
    int8_t s8 = (int8_t)s32; /* Narrowing with sign change analysis */
    result += s8;
    
    return checksum(result);
}

/* ========== Test 2: Loop Range Analysis ========== */
__attribute__((noinline))
int test_loop_range_analysis(void) {
    int result = 0;
    
    /* Test 2a: Complex loop bounds with bitwise ops */
    uint32_t base = 0x12345678;
    uint32_t mask = 0x00000FFF;
    
    for (uint32_t i = base & mask;  /* i in [0, 0xFFF] */
         i < (base | 0x000007FF);   /* Upper bound analysis needed */
         i += 17) {                 /* Non-power-of-2 step */
        result ^= i;
    }
    
    /* Test 2b: Nested loops with dependent bounds */
    int outer_max = 100;
    for (int j = 0; j < outer_max; j++) {
        /* Inner loop bound depends on outer, with bitwise constraint */
        int inner_bound = (j & 0x3F) + 50;  /* Range: [50, 113] */
        for (int k = j; k < inner_bound; k++) {
            result += k * 3;
        }
    }
    
    /* Test 2c: Loop with shifting bound */
    uint64_t start = 1ULL << 31;  /* 2^31 */
    uint64_t end = 1ULL << 33;    /* 2^33 */
    for (uint64_t v = start; v < end; v += (1ULL << 30)) {
        result += (int)v;  /* Narrowing in loop body */
    }
    
    /* Test 2d: Reverse loop with underflow check */
    for (int32_t r = 1000; r > -1000; r -= 7) {
        result ^= r;
    }
    
    return checksum(result);
}

/* ========== Test 3: Saturation Arithmetic ========== */
__attribute__((noinline))
int test_saturation_arithmetic(void) {
    int result = 0;
    
    /* Manual saturation implementation */
    static int32_t saturating_add(int32_t a, int32_t b) {
        int64_t sum = (int64_t)a + (int64_t)b;
        
        /* These comparisons should trigger the uncovered logic */
        if (sum > 0x7FFFFFFF) return 0x7FFFFFFF;      /* INT32_MAX */
        if (sum < (int64_t)0x80000000) return 0x80000000; /* INT32_MIN */
        
        return (int32_t)sum;
    }
    
    /* Test saturation at boundaries */
    result += saturating_add(0x70000000, 0x10000000);  /* Should saturate to MAX */
    result += saturating_add(0x80000000, -1);          /* Should saturate to MIN */
    result += saturating_add(100, 200);                /* Normal case */
    
    /* Test with variables that have known ranges */
    int32_t x = 500, y = 600;
    for (int i = 0; i < 10; i++) {
        x = saturating_add(x, y);
        y = saturating_add(y, -300);
        result ^= x + y;
    }
    
#ifdef __STDC_IEC_559__
    /* GCC fixed-point types if available */
    gcc_accum acc = 0.5k;
    for (int i = 0; i < 5; i++) {
        acc = acc * 0.9k;  /* Fixed-point multiplication */
        result += (int)(acc * 1000);
    }
#endif
    
    return checksum(result);
}

/* ========== Test 4: Bit-Field Ranges ========== */
__attribute__((noinline))
int test_bitfield_ranges(void) {
    int result = 0;
    
    /* Struct with various bit-fields */
    struct bitfields {
        unsigned int a : 3;   /* 0-7 */
        signed int b : 5;     /* -16 to 15 */
        unsigned int c : 12;  /* 0-4095 */
        signed int d : 20;    /* -524288 to 524287 */
    } bf;
    
    /* Assign boundary values */
    bf.a = 7;     /* Max for 3 bits */
    bf.b = -16;   /* Min for 5 signed bits */
    bf.c = 4095;  /* Max for 12 bits */
    bf.d = 524287; /* Max for 20 signed bits */
    
    /* Comparisons that require range analysis */
    if (bf.a == 7) result ^= 1;
    if (bf.b < 0) result ^= 2;
    if (bf.c > 4000) result ^= 4;
    if (bf.d >= 500000) result ^= 8;
    
    /* Union with bit-field overlay */
    union overlay {
        uint32_t full;
        struct {
            uint32_t low : 16;
            uint32_t high : 16;
        } parts;
    } u;
    
    u.full = 0x87654321;
    /* These comparisons need to understand bit-field ranges */
    if (u.parts.low > 0x8000) result ^= 16;
    if (u.parts.high < 0x9000) result ^= 32;
    
    /* Bit-field in loop */
    for (bf.a = 0; bf.a < 8; bf.a++) {
        result += bf.a * 3;
    }
    
    return checksum(result);
}

/* ========== Test 5: Overflow Builtins ========== */
__attribute__((noinline))
int test_overflow_builtins(void) {
    int result = 0;
    int overflow;
    
    /* Test 5a: Basic overflow checks */
    int32_t a = 0x70000000;
    int32_t b = 0x10000000;
    
    if (__builtin_add_overflow(a, b, &a)) {
        result ^= 0x100;  /* Overflow occurred */
    }
    
    /* Test 5b: Overflow in loops */
    uint64_t prod = 1;
    for (int i = 1; i < 20; i++) {
        uint64_t tmp;
        if (__builtin_mul_overflow(prod, i, &tmp)) {
            result ^= i;  /* Record when overflow happens */
        } else {
            prod = tmp;
        }
    }
    result += (int)prod;
    
    /* Test 5c: Overflow with range-constrained variables */
    int32_t x = 100;
    int32_t y = 200;
    
    /* Constrain ranges through conditions */
    if (x > 50 && x < 150) {
        if (y > 100 && y < 300) {
            int32_t sum;
            overflow = __builtin_add_overflow(x, y, &sum);
            result ^= (overflow << 8) | (sum & 0xFF);
        }
    }
    
    /* Test 5d: Sub overflow with negative numbers */
    int32_t neg = -0x70000000;
    int32_t pos = 0x10000000;
    if (__builtin_sub_overflow(neg, pos, &neg)) {
        result ^= 0x200;
    }
    
    return checksum(result);
}

/* ========== Test 6: Additional Range Stress Tests ========== */
__attribute__((noinline))
int test_range_stress(void) {
    int result = 0;
    
    /* Test 6a: Shift operations with range analysis */
    uint32_t val = 0x80000000;
    for (int shift = 0; shift < 32; shift++) {
        uint32_t shifted = val >> shift;
        /* Comparison that needs to understand shift produces [0, 0x80000000] */
        if (shifted > (0x80000000 >> (shift + 1))) {
            result ^= (1 << (shift & 0x1F));
        }
    }
    
    /* Test 6b: Modulo operations creating known ranges */
    for (int i = -1000; i < 1000; i++) {
        int mod = i % 37;  /* Range: [-36, 36] */
        if (mod == 0) result++;
        if (mod < -30) result ^= i;
    }
    
    /* Test 6c: Multi-dimensional range */
    int arr[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j;
            /* Access with bounds that should be provable */
            if (i >= 0 && i < 10 && j >= 0 && j < 10) {
                result += arr[i][j];
            }
        }
    }
    
    return checksum(result);
}

/* ========== Main Function ========== */
int main(void) {
    int final_result = 0;
    
    printf("Starting integer range analysis tests...\n");
    
    /* Run all tests */
    final_result ^= test_narrowing_conversions();
    printf("Test 1 complete\n");
    
    final_result ^= test_loop_range_analysis();
    printf("Test 2 complete\n");
    
    final_result ^= test_saturation_arithmetic();
    printf("Test 3 complete\n");
    
    final_result ^= test_bitfield_ranges();
    printf("Test 4 complete\n");
    
    final_result ^= test_overflow_builtins();
    printf("Test 5 complete\n");
    
    final_result ^= test_range_stress();
    printf("Test 6 complete\n");
    
    /* Use result to prevent optimization */
    sink = final_result;
    
    printf("All tests completed. Result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
