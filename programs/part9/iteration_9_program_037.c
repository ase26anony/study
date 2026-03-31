#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent dead code elimination
static volatile int sink;

// Prevent inlining for better coverage tracking
#define NOINLINE __attribute__((noinline))

// Checksum function to prevent optimization
NOINLINE unsigned int compute_checksum(const void *data, size_t len) {
    const unsigned char *bytes = (const unsigned char *)data;
    unsigned int hash = 0x811c9dc5;
    for (size_t i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= 0x01000193;
    }
    return hash;
}

// ==================== Test 1: Narrowing Conversions ====================
NOINLINE unsigned int test_narrowing_conversions(void) {
    unsigned int checksum = 0;
    char buffer[256];
    int pos = 0;
    
    // Constants at type boundaries
    int64_t large_vals[] = {
        INT64_MAX, INT64_MIN, 
        (int64_t)INT32_MAX + 1, (int64_t)INT32_MIN - 1,
        0x7FFFFFFF00000000LL, 0x8000000000000000LL
    };
    
    for (int i = 0; i < 6; i++) {
        // Narrowing conversions that require range analysis
        int32_t narrowed = (int32_t)large_vals[i];
        uint16_t narrowed_u16 = (uint16_t)large_vals[i];
        int8_t narrowed_i8 = (int8_t)large_vals[i];
        
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, 
                       "%d:%u:%d:", narrowed, narrowed_u16, narrowed_i8);
        
        // Operations that might overflow
        int32_t x = (int32_t)(large_vals[i] >> 32);
        int32_t y = (int32_t)(large_vals[i] & 0xFFFFFFFF);
        int32_t sum = x + y;  // Potential overflow
        
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%d:", sum);
    }
    
    checksum = compute_checksum(buffer, pos);
    sink = checksum;
    return checksum;
}

// ==================== Test 2: Loop Range Analysis ====================
NOINLINE unsigned int test_loop_range_analysis(void) {
    unsigned int checksum = 0;
    int results[100];
    int idx = 0;
    
    // Complex loop bounds with bitwise operations
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    uint32_t mask = 0x00000FFF;
    
    // Loop with bitwise-derived bounds
    for (uint32_t i = a & mask; i < (b | 0x7FF); i += (i & 0x3F) + 1) {
        if (i < 100) results[idx++] = (int)i;
        if (idx >= 100) break;
    }
    
    // Nested loops with dependent bounds
    for (int outer = 0; outer < 10; outer++) {
        int limit = (outer * 37) & 0x1F;  // Range 0-31
        for (int inner = outer; inner < limit; inner += (outer & 0x3) + 1) {
            if (idx < 100) results[idx++] = outer * 100 + inner;
        }
    }
    
    // Loop with shifting bound
    int shift_val = 8;
    for (int i = 0; i < (1 << shift_val); i += (i & 0xF) + 1) {
        if (idx < 100) results[idx++] = i;
    }
    
    checksum = compute_checksum(results, idx * sizeof(int));
    sink = checksum;
    return checksum;
}

// ==================== Test 3: Saturation Arithmetic ====================
NOINLINE unsigned int test_saturation_arithmetic(void) {
    unsigned int checksum = 0;
    int32_t saturated[50];
    int count = 0;
    
    // Manual saturation implementation
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
    
    // Test values near boundaries
    int32_t test_vals[] = {
        INT32_MAX, INT32_MIN, 
        INT32_MAX - 100, INT32_MIN + 100,
        10000, -10000, 0
    };
    
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            if (count >= 50) break;
            saturated[count++] = sat_add(test_vals[i], test_vals[j]);
            if (count >= 50) break;
            saturated[count++] = sat_mul(test_vals[i], test_vals[j]);
        }
    }
    
    // Fixed-point types if available (GCC extension)
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.75r;
    _Fract fsum = f1 + f2;  // May saturate
    saturated[count % 50] = *(int32_t*)&fsum;
    #endif
    
    checksum = compute_checksum(saturated, count * sizeof(int32_t));
    sink = checksum;
    return checksum;
}

// ==================== Test 4: Bitfield Ranges ====================
NOINLINE unsigned int test_bitfield_ranges(void) {
    unsigned int checksum = 0;
    
    // Struct with various bitfields
    struct BitfieldStruct {
        unsigned int a : 3;   // 0-7
        signed int b : 5;     // -16 to 15
        unsigned int c : 12;  // 0-4095
        signed int d : 20;    // -524288 to 524287
        unsigned int e : 1;   // 0-1
    } bs;
    
    unsigned char buffer[sizeof(bs) * 10];
    int buf_idx = 0;
    
    // Test assignments and comparisons
    unsigned int test_values[] = {0, 1, 7, 8, 15, 4095, 4096, 65535};
    
    for (int i = 0; i < 8; i++) {
        bs.a = test_values[i] & 0x7;  // Force into 3-bit range
        bs.b = (test_values[i] & 0x1F) - 16;  // Signed 5-bit
        bs.c = test_values[i] & 0xFFF;  // 12-bit
        bs.d = (test_values[i] & 0xFFFFF) - 0x80000;  // Signed 20-bit
        bs.e = test_values[i] & 0x1;
        
        // Comparisons that test range understanding
        int cmp_results = 0;
        if (bs.a > 3) cmp_results |= 1;
        if (bs.b < -8) cmp_results |= 2;
        if (bs.c >= 2048) cmp_results |= 4;
        if (bs.d <= -262144) cmp_results |= 8;
        if (bs.e == 0) cmp_results |= 16;
        
        memcpy(buffer + buf_idx, &bs, sizeof(bs));
        buf_idx += sizeof(bs);
        buffer[buf_idx++] = cmp_results;
    }
    
    // Union with bitfields for type punning
    union BitfieldUnion {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } parts;
        uint32_t whole;
    } bu;
    
    bu.whole = 0x12345678;
    bu.parts.low = (bu.parts.low + 1) & 0xFFFF;  // Keep in 16-bit range
    bu.parts.high = (bu.parts.high - 1) & 0xFFFF;
    
    memcpy(buffer + buf_idx, &bu, sizeof(bu));
    buf_idx += sizeof(bu);
    
    checksum = compute_checksum(buffer, buf_idx);
    sink = checksum;
    return checksum;
}

// ==================== Test 5: Overflow Builtins ====================
NOINLINE unsigned int test_overflow_builtins(void) {
    unsigned int checksum = 0;
    int overflow_results[100];
    int res_idx = 0;
    
    // Test values with partially known ranges
    int32_t vals[] = {100, 1000, 10000, 100000, INT32_MAX / 2, INT32_MAX / 3};
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            int32_t result;
            int overflow;
            
            // Range-restricting conditions
            if (vals[i] > 0 && vals[j] > 0) {
                overflow = __builtin_add_overflow(vals[i], vals[j], &result);
                overflow_results[res_idx++] = overflow ? -1 : result;
                
                overflow = __builtin_mul_overflow(vals[i], vals[j], &result);
                overflow_results[res_idx++] = overflow ? -2 : result;
            }
            
            if (vals[i] < 1000 && vals[j] < 1000) {
                overflow = __builtin_sub_overflow(vals[i], vals[j], &result);
                overflow_results[res_idx++] = overflow ? -3 : result;
            }
            
            if (res_idx >= 95) break;
        }
        if (res_idx >= 95) break;
    }
    
    // Complex expression with builtins
    for (int i = 0; i < 10 && res_idx < 100; i++) {
        int32_t a = i * 1000000;
        int32_t b = (i + 1) * 500000;
        int32_t c;
        
        if (!__builtin_add_overflow(a, b, &c)) {
            int32_t d;
            if (!__builtin_mul_overflow(c, 2, &d)) {
                overflow_results[res_idx++] = d;
            } else {
                overflow_results[res_idx++] = -4;
            }
        } else {
            overflow_results[res_idx++] = -5;
        }
    }
    
    checksum = compute_checksum(overflow_results, res_idx * sizeof(int));
    sink = checksum;
    return checksum;
}

// ==================== Test 6: Edge Case Comparisons ====================
NOINLINE unsigned int test_edge_case_comparisons(void) {
    unsigned int checksum = 0;
    int comparisons[64];
    int cmp_idx = 0;
    
    // Variables with constrained ranges
    int32_t x = 100;
    int32_t y = -100;
    
    // Chain of comparisons that define value ranges
    if (x > 0 && x < 1000) {
        comparisons[cmp_idx++] = 1;
        // x is now known to be in (0, 1000)
        
        if (x > INT32_MAX - 500) {
            comparisons[cmp_idx++] = 2;  // Dead branch? Requires analysis
        }
        
        if (x < INT32_MIN + 1000) {
            comparisons[cmp_idx++] = 3;  // Dead branch?
        }
    }
    
    // Modulo operations create known ranges
    uint32_t mod_val = 123456;
    uint32_t constrained = mod_val % 1000;  // Known to be 0-999
    
    if (constrained > 500) {
        comparisons[cmp_idx++] = 4;
        if (constrained < 600) {
            comparisons[cmp_idx++] = 5;  // Further refinement
        }
    }
    
    // Shift operations that might overflow
    for (int i = 28; i <= 32; i++) {
        uint32_t shifted = 1U << i;  // Shift may exceed type width
        comparisons[cmp_idx++] = (shifted > 0x80000000) ? 6 : 7;
    }
    
    // Extreme comparisons
    int64_t big = INT64_MAX;
    if (big > INT32_MAX) comparisons[cmp_idx++] = 8;
    if (big - 1 < INT64_MAX) comparisons[cmp_idx++] = 9;
    
    checksum = compute_checksum(comparisons, cmp_idx * sizeof(int));
    sink = checksum;
    return checksum;
}

// ==================== MAIN ====================
int main(void) {
    unsigned int final_checksum = 0;
    
    // Run all tests
    final_checksum ^= test_narrowing_conversions();
    final_checksum ^= test_loop_range_analysis();
    final_checksum ^= test_saturation_arithmetic();
    final_checksum ^= test_bitfield_ranges();
    final_checksum ^= test_overflow_builtins();
    final_checksum ^= test_edge_case_comparisons();
    
    // Use sink to prevent optimization
    sink = final_checksum;
    
    printf("Test completed. Final checksum: %u\n", final_checksum);
    return 0;
}
