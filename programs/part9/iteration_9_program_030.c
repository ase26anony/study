#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum to prevent optimization */
__attribute__((noinline)) 
uint32_t compute_checksum(const void* data, size_t size) {
    uint32_t hash = 5381;
    const unsigned char* p = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        hash = ((hash << 5) + hash) + p[i];
    }
    return hash;
}

/* Test 1: Narrowing conversions with boundary values */
__attribute__((noinline))
uint32_t test_narrowing_conversions(void) {
    uint32_t checksum = 0;
    uint64_t buffer[8] = {0};
    int idx = 0;
    
    /* Constants at type boundaries */
    int64_t large_positive = 0x7FFFFFFFFFFFFFFFLL;
    int64_t large_negative = 0x8000000000000000LL;
    uint64_t max_unsigned = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Narrowing conversions that require range analysis */
    int32_t narrow1 = (int32_t)large_positive;  /* Should be -1 */
    int32_t narrow2 = (int32_t)large_negative;  /* Should be 0 */
    uint32_t narrow3 = (uint32_t)max_unsigned;  /* Should be 0xFFFFFFFF */
    
    buffer[idx++] = narrow1;
    buffer[idx++] = narrow2;
    buffer[idx++] = narrow3;
    
    /* Operations that may overflow when narrowed */
    int64_t a = 0x123456789ABCDEF0LL;
    int64_t b = 0xFEDCBA9876543210LL;
    int32_t sum_narrow = (int32_t)(a + b);  /* Overflow in narrowing */
    
    /* Shifts that exceed target type width */
    uint64_t shifted = 1ULL << 40;
    uint32_t shift_narrow = (uint32_t)(shifted >> 8);
    
    buffer[idx++] = sum_narrow;
    buffer[idx++] = shift_narrow;
    
    /* Comparisons at boundaries */
    int32_t x = 0x7FFFFFFF;
    int32_t y = 0x80000000;
    buffer[idx++] = (x > INT32_MAX - 10) ? 1 : 0;
    buffer[idx++] = (y < INT32_MIN + 10) ? 1 : 0;
    
    checksum = compute_checksum(buffer, sizeof(buffer));
    sink = checksum;
    return checksum;
}

/* Test 2: Complex loop bound analysis */
__attribute__((noinline))
uint32_t test_loop_range_analysis(void) {
    uint32_t checksum = 0;
    uint32_t buffer[256] = {0};
    int idx = 0;
    
    /* Outer loop with bitwise-derived bounds */
    for (int32_t i = 100; i < 200; i++) {
        /* Inner loop with complex bound calculation */
        uint32_t mask = 0xFFF;
        uint32_t start = i & mask;
        uint32_t end = (i | 0x7FF) & 0xFFF;
        
        /* Loop where bounds depend on outer variable */
        for (uint32_t j = start; j < end && j < 1000; j += (i & 0x3F) + 1) {
            buffer[idx++ % 256] = j ^ i;
        }
        
        /* Another loop with shifting bounds */
        int32_t k = i - 50;
        for (int32_t m = k * 2; m < k + 100; m += 3) {
            if (m >= 0 && m < 256) {
                buffer[m] = buffer[m] * 1103515245 + 12345;
            }
        }
    }
    
    /* Loop with wrap-around analysis */
    uint32_t counter = 0;
    for (int i = 0; i < 1000; i++) {
        counter = (counter + 1) & 0x3FF;  /* Constrained to 10 bits */
        buffer[counter % 256] += i;
        
        /* Condition that tests boundary */
        if (counter == 0x3FF) {
            buffer[0] = 1;
        }
    }
    
    checksum = compute_checksum(buffer, sizeof(buffer));
    sink = checksum;
    return checksum;
}

/* Test 3: Saturation arithmetic */
__attribute__((noinline))
uint32_t test_saturation_arithmetic(void) {
    uint32_t checksum = 0;
    int32_t buffer[16] = {0};
    int idx = 0;
    
    /* Manual saturation functions */
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    /* Test cases near boundaries */
    buffer[idx++] = sat_add(INT32_MAX - 100, 200);  /* Should saturate */
    buffer[idx++] = sat_add(INT32_MIN + 100, -200); /* Should saturate */
    buffer[idx++] = sat_add(1000, 2000);           /* Normal case */
    
    buffer[idx++] = sat_mul(INT32_MAX / 2, 3);     /* Should saturate */
    buffer[idx++] = sat_mul(INT32_MIN / 2, 3);     /* Should saturate */
    buffer[idx++] = sat_mul(1000, 2000);           /* Normal case */
    
    /* Fixed-point types if available */
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.7r;
    _Fract f3 = f1 + f2;  /* May saturate */
    buffer[idx++] = *(int32_t*)&f3;
    #endif
    
    /* Clamping to range */
    int32_t clamp(int32_t val, int32_t min, int32_t max) {
        if (val < min) return min;
        if (val > max) return max;
        return val;
    }
    
    buffer[idx++] = clamp(500, 0, 255);
    buffer[idx++] = clamp(-10, 0, 255);
    buffer[idx++] = clamp(300, 0, 255);
    
    checksum = compute_checksum(buffer, sizeof(buffer));
    sink = checksum;
    return checksum;
}

/* Test 4: Bit-field range analysis */
__attribute__((noinline))
uint32_t test_bitfield_ranges(void) {
    uint32_t checksum = 0;
    uint32_t buffer[8] = {0};
    int idx = 0;
    
    /* Struct with various bit-fields */
    struct BitFields {
        unsigned int a : 3;   /* 0-7 */
        signed int b : 5;     /* -16 to 15 */
        unsigned int c : 12;  /* 0-4095 */
        signed int d : 20;    /* -524288 to 524287 */
    } bf;
    
    /* Assignments that test boundaries */
    bf.a = 7;      /* Max for 3 bits */
    bf.b = -16;    /* Min for 5-bit signed */
    bf.c = 4095;   /* Max for 12 bits */
    bf.d = 524287; /* Max for 20-bit signed */
    
    buffer[idx++] = bf.a;
    buffer[idx++] = bf.b;
    buffer[idx++] = bf.c;
    buffer[idx++] = bf.d;
    
    /* Comparisons against bit-field capacity */
    unsigned int test_val = 10;
    if (test_val > 7) {  /* 7 is max for 3-bit field */
        bf.a = 7;
    } else {
        bf.a = test_val;
    }
    
    /* Union with bit-fields and integer */
    union BitUnion {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } parts;
        uint32_t whole;
    } u;
    
    u.whole = 0x12345678;
    buffer[idx++] = u.parts.low;
    buffer[idx++] = u.parts.high;
    
    /* Check if parts are within their bit-width */
    if (u.parts.low <= 0xFFFF && u.parts.high <= 0xFFFF) {
        buffer[idx++] = 1;
    }
    
    checksum = compute_checksum(buffer, sizeof(buffer));
    sink = checksum;
    return checksum;
}

/* Test 5: Overflow builtins with range analysis */
__attribute__((noinline))
uint32_t test_overflow_builtins(void) {
    uint32_t checksum = 0;
    uint32_t buffer[16] = {0};
    int idx = 0;
    int overflow;
    
    /* Basic overflow checks */
    int32_t a = INT32_MAX - 100;
    int32_t b = 200;
    overflow = __builtin_add_overflow(a, b, &a);
    buffer[idx++] = a;
    buffer[idx++] = overflow;
    
    /* Multiplication with range-constrained inputs */
    int32_t x = 1000000;
    int32_t y = 2000;
    overflow = __builtin_mul_overflow(x, y, &x);
    buffer[idx++] = x;
    buffer[idx++] = overflow;
    
    /* In loops with varying ranges */
    for (int32_t i = 1; i < 100; i *= 2) {
        int32_t result;
        overflow = __builtin_mul_overflow(i, i, &result);
        buffer[idx++ % 16] = result;
        buffer[idx++ % 16] = overflow;
    }
    
    /* Subtraction with negative boundaries */
    int32_t m = INT32_MIN + 100;
    int32_t n = 200;
    overflow = __builtin_sub_overflow(m, n, &m);
    buffer[idx++ % 16] = m;
    buffer[idx++ % 16] = overflow;
    
    /* Combined operations */
    int32_t val = 10;
    for (int i = 0; i < 10; i++) {
        int32_t old = val;
        overflow = __builtin_add_overflow(val, val * 2, &val);
        if (overflow || val < old) {
            val = INT32_MAX;
        }
        buffer[i % 16] = val;
    }
    
    checksum = compute_checksum(buffer, sizeof(buffer));
    sink = checksum;
    return checksum;
}

/* Main function that runs all tests */
int main(void) {
    uint32_t total_checksum = 0;
    
    printf("Running integer range analysis tests...\n");
    
    total_checksum ^= test_narrowing_conversions();
    total_checksum ^= test_loop_range_analysis();
    total_checksum ^= test_saturation_arithmetic();
    total_checksum ^= test_bitfield_ranges();
    total_checksum ^= test_overflow_builtins();
    
    printf("Total checksum: %u\n", total_checksum);
    
    /* Use results to prevent optimization */
    if (total_checksum == 0xDEADBEEF) {
        printf("Impossible condition\n");
    }
    
    return 0;
}
