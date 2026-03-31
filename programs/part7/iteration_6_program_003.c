/* test_resource_coverage.c
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource.cc mark_referenced_resources function:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register accesses
 * - SUBREG for subregister operations
 * - MEM_P with complex addressing modes
 */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization (-O2 or -O3) to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Function 1: Generate ZERO_EXTRACT RTL for bit-field operations */
NOINLINE static uint32_t bitfield_operations(volatile uint32_t *val) {
    /* Multiple bit-field operations to increase chances of ZERO_EXTRACT */
    uint32_t x = *val;
    
    /* These operations often generate ZERO_EXTRACT in RTL */
    uint32_t a = (x >> 3) & 0x1F;      /* Extract bits 3-7 */
    uint32_t b = (x >> 8) & 0xFF;      /* Extract bits 8-15 */
    uint32_t c = (x >> 16) & 0x7;      /* Extract bits 16-18 */
    
    /* Combine with another volatile read to prevent optimization */
    volatile uint32_t y = *val;
    uint32_t d = (y >> 20) & 0x3FF;    /* Extract bits 20-29 */
    
    return a + b + c + d;
}

/* Function 2: Generate STRICT_LOW_PART RTL using inline assembly */
NOINLINE static uint32_t partial_register_ops(uint32_t value) {
    uint32_t result = 0;
    
#if defined(__i386__) || defined(__x86_64__)
    /* x86 inline assembly that operates on partial registers */
    uint8_t byte_val;
    uint16_t word_val;
    
    /* Byte operation - may generate STRICT_LOW_PART for AL register */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q" (byte_val)        /* =q constraint for byte-addressable register */
        : "r" ((uint8_t)value)
        : "cc"
    );
    
    /* Word operation - may generate STRICT_LOW_PART for AX register */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r" (word_val)        /* Word-sized operation */
        : "r" ((uint16_t)value)
        : "cc"
    );
    
    result = byte_val + word_val;
#else
    /* Generic fallback: use bit-field structure */
    struct {
        uint32_t low16 : 16;
        uint32_t high16 : 16;
    } bits;
    
    bits.low16 = value & 0xFFFF;
    bits.high16 = (value >> 16) & 0xFFFF;
    
    /* Access partial bits through shifts and masks */
    uint16_t low_part = (value >> 8) & 0xFF;  /* May generate partial reg access */
    uint16_t high_part = (value >> 24) & 0xFF;
    
    result = low_part + high_part + bits.low16;
#endif
    
    return result;
}

/* Function 3: Generate SUBREG RTL through type conversions */
NOINLINE static uint64_t subregister_conversions(uint64_t big_val) {
    /* Multiple type conversions to generate SUBREG operations */
    
    /* 64-bit to 32-bit truncation */
    uint32_t low32 = (uint32_t)big_val;
    uint32_t high32 = (uint32_t)(big_val >> 32);
    
    /* 32-bit to 16-bit conversions */
    uint16_t a = (uint16_t)low32;
    uint16_t b = (uint16_t)(low32 >> 16);
    uint16_t c = (uint16_t)high32;
    uint16_t d = (uint16_t)(high32 >> 16);
    
    /* 16-bit to 8-bit extractions */
    uint8_t a_low = a & 0xFF;
    uint8_t a_high = (a >> 8) & 0xFF;
    
    /* Mix different sized operations */
    uint32_t mixed = ((uint32_t)a << 16) | b;
    uint16_t mixed16 = (mixed >> 8) & 0xFFFF;
    
    /* Use a union for type punning (may generate SUBREG) */
    union {
        uint64_t full;
        uint32_t halves[2];
        uint16_t words[4];
        uint8_t bytes[8];
    } converter;
    
    converter.full = big_val;
    uint32_t from_union = converter.halves[0] + converter.halves[1];
    
    return (uint64_t)a_low + a_high + mixed16 + from_union;
}

/* Function 4: Generate complex MEM_P addressing modes */
NOINLINE static int complex_memory_access(int *base, int index1, int index2) {
    /* Multi-dimensional array with variable indices */
    int matrix[10][10];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Complex addressing calculations */
    int *ptr1 = &matrix[index1 % 10][index2 % 10];
    int *ptr2 = base + (index1 * 3 + index2 * 7) % 100;
    
    /* Pointer arithmetic with scaling */
    int val1 = *(ptr1 + (index1 & 3));
    int val2 = *(ptr2 - (index2 & 7));
    
    /* Structure with multiple fields */
    struct {
        int a;
        int b;
        int c;
        int array[5];
    } s;
    
    s.a = index1;
    s.b = index2;
    s.c = val1;
    
    for (int i = 0; i < 5; i++) {
        s.array[i] = val2 + i;
    }
    
    /* Complex memory expression */
    return val1 + val2 + s.array[index1 % 5] + *(base + index2);
}

/* Function 5: Combined operations to increase RTL pattern density */
NOINLINE static uint32_t combined_operations(volatile uint32_t *mem, int idx) {
    uint32_t x = *mem;
    
    /* Bit-field extraction (ZERO_EXTRACT potential) */
    uint32_t extracted = (x >> (idx & 0xF)) & ((1 << (idx & 0x7)) - 1);
    
    /* Type conversion chain (SUBREG potential) */
    uint64_t big = (uint64_t)x * 0x12345678;
    uint32_t conv1 = (uint16_t)big;
    uint32_t conv2 = (uint8_t)(big >> 32);
    
    /* Memory access with computation (MEM_P potential) */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = i * i;
    }
    
    int mem_val = array[(extracted + idx) & 0xF];
    mem_val += array[(conv1 + conv2) & 0xF];
    
    return extracted + conv1 + conv2 + mem_val;
}

/* Main function that exercises all patterns */
int main(void) {
    volatile uint32_t seed = 0xDEADBEEF;
    int result = 0;
    int data[100];
    
    /* Initialize data array */
    for (int i = 0; i < 100; i++) {
        data[i] = i * 3;
    }
    
    /* Call each function multiple times in a loop
     * This increases the chance of RTL generation during optimization passes */
    for (int i = 0; i < 10; i++) {
        result += bitfield_operations(&seed);
        result += partial_register_ops(seed + i);
        result += (int)subregister_conversions((uint64_t)seed * i);
        result += complex_memory_access(data, i, i * 2);
        result += combined_operations(&seed, i);
        
        /* Modify seed to create varying patterns */
        seed = seed * 1103515245 + 12345;
    }
    
    /* Use result to prevent dead code elimination */
    volatile int sink = result;
    
    /* Simple validation */
    if (result != 0) {
        return 0;  /* Success */
    }
    
    return 1;  /* Should never reach here with proper seed */
}
