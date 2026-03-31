/* test_resource_coverage.c
 * 
 * This program is designed to generate RTL patterns that will exercise
 * the uncovered lines in GCC's resource.cc mark_referenced_resources function.
 * Specifically, it aims to produce RTL with:
 * - ZERO_EXTRACT (bit-field operations)
 * - STRICT_LOW_PART (partial register accesses via inline assembly)
 * - SUBREG (type conversions and sub-register accesses)
 * - MEM_P with complex addressing (array/pointer operations)
 */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization enabled (-O2 or -O3)"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Function 1: Generate ZERO_EXTRACT patterns via bit-field operations */
NOINLINE static uint32_t bitfield_ops(volatile uint32_t *val) {
    /* Volatile forces memory access and prevents optimization */
    uint32_t x = *val;
    
    /* Various bit-field extractions that may generate ZERO_EXTRACT */
    uint32_t result = 0;
    
    /* Extract bits 3-6 */
    result |= (x >> 3) & 0xF;
    
    /* Extract bits 10-15 */
    result |= ((x >> 10) & 0x3F) << 4;
    
    /* Extract bits 20-23 */
    result |= ((x >> 20) & 0xF) << 10;
    
    /* Combined extraction with mask */
    uint32_t mask = 0x00FF00FF;
    result ^= (x & mask) | ((x >> 8) & mask);
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART via inline assembly (x86/x86-64 only) */
NOINLINE static uint32_t partial_reg_ops(uint32_t val) {
    uint32_t result = val;
    
#if defined(__i386__) || defined(__x86_64__)
    /* Byte operations that may generate STRICT_LOW_PART */
    uint8_t byte1, byte2;
    
    /* Force byte register operations */
    asm volatile (
        "movb %[in], %[out1]\n\t"
        "incb %[out1]\n\t"
        "movb %[out1], %[out2]"
        : [out1] "=q" (byte1), [out2] "=q" (byte2)
        : [in] "q" ((uint8_t)val)
        : "cc"
    );
    
    result = (result & 0xFFFFFF00) | byte1 | (byte2 << 8);
    
    /* Half-word operation */
    uint16_t halfword;
    asm volatile (
        "movw %w[in], %w[out]\n\t"
        "notw %w[out]"
        : [out] "=r" (halfword)
        : [in] "r" ((uint16_t)val)
        : "cc"
    );
    
    result = (result & 0xFFFF0000) | halfword;
#endif
    
    /* Fallback for non-x86: use bit operations that might still generate
     * partial register RTL through other optimizations */
    result = (result & 0xFFFF) | ((result & 0xFF) << 16);
    
    return result;
}

/* Function 3: Generate SUBREG patterns through type conversions */
NOINLINE static uint64_t subreg_ops(uint64_t val) {
    /* Various type conversions that may generate SUBREG */
    
    /* 64-bit to smaller types */
    uint32_t low32 = (uint32_t)val;
    uint16_t low16 = (uint16_t)val;
    uint8_t low8 = (uint8_t)val;
    
    /* Smaller to larger with sign/zero extension */
    int32_t signed32 = (int32_t)low16;  /* May generate SUBREG for sign extension */
    uint32_t extended32 = (uint32_t)low8;
    
    /* Pointer casting between different sized types */
    uint32_t *ptr32 = (uint32_t*)&val;
    uint16_t *ptr16 = (uint16_t*)&val;
    
    /* Access different parts of the 64-bit value */
    uint64_t result = val;
    result ^= (uint64_t)low32;
    result ^= (uint64_t)signed32 << 32;
    result += (uint64_t)(*ptr32);
    result += (uint64_t)(*ptr16);
    
    /* Union for type punning - often generates SUBREG */
    union {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } parts;
    } converter;
    
    converter.full = result;
    result = converter.parts.low + (uint64_t)converter.parts.high;
    
    return result;
}

/* Function 4: Generate complex MEM_P addressing patterns */
NOINLINE static uint32_t memory_ops(uint32_t *base, int index1, int index2) {
    /* Complex addressing modes */
    uint32_t result = 0;
    
    /* Multi-dimensional array style access */
    uint32_t arr[10][10];
    
    /* Initialize to prevent undefined behavior */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Variable indexing - forces non-constant address computation */
    result += arr[index1 % 10][index2 % 10];
    
    /* Pointer arithmetic with variable offsets */
    result += *(base + index1);
    result += *(base + index2);
    result += *(base + index1 + index2);
    
    /* Structure-like access via pointer offsets */
    struct {
        uint32_t a, b, c, d;
    } *struct_ptr = (void*)base;
    
    result += struct_ptr->b + struct_ptr->d;
    
    /* Post-increment addressing simulation */
    uint32_t *ptr = base;
    for (int i = 0; i < 4; i++) {
        result += *ptr++;
    }
    
    return result;
}

/* Main test driver that calls all pattern generators */
int main(void) {
    volatile uint32_t seed = 0x89ABCDEF;
    uint32_t array[100];
    uint32_t result = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 0x1234567;
    }
    
    /* Loop to increase chance of RTL generation and resource marking */
    for (int i = 0; i < 100; i++) {
        /* Mix all operations */
        result ^= bitfield_ops(&seed);
        result += partial_reg_ops(result);
        result ^= (uint32_t)subreg_ops(result);
        result += memory_ops(array, i, i * 2);
        
        /* Modify seed to change bit patterns */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Use result to prevent dead code elimination */
    return (int)(result & 0xFF);
}
