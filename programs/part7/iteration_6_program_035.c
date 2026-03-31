/*
 * Test program to cover lines 282-290 in GCC's resource.cc
 * These lines handle ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P operations
 */

#include <stdint.h>
#include <assert.h>

/* Compile-time check for optimization level */
#ifndef __OPTIMIZE__
#error "This test requires optimization (-O2 or -O3) to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate functions generate RTL */
#define NOINLINE __attribute__((noinline))

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int padding : 8;
};

/* Union for type punning and SUBREG operations */
union type_pun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } halves;
    uint8_t bytes[4];
};

/* Global variables to prevent constant propagation */
volatile int g_index1 = 3;
volatile int g_index2 = 7;
volatile int g_value = 0x12345678;

/* Function 1: Generate ZERO_EXTRACT patterns through bit-field operations */
NOINLINE static uint32_t test_zero_extract(void) {
    struct bitfield_struct bf;
    uint32_t result = 0;
    
    /* Multiple bit-field extractions that may generate ZERO_EXTRACT */
    bf.field1 = g_value & 0xF;
    bf.field2 = (g_value >> 4) & 0xFF;
    bf.field3 = (g_value >> 12) & 0xFFF;
    
    /* Combine with shifts and masks - classic ZERO_EXTRACT pattern */
    result |= (bf.field1 << 0);
    result |= (bf.field2 << 4);
    result |= (bf.field3 << 12);
    
    /* Additional bit-field extraction with variable shift */
    int shift = g_index1;
    result |= ((g_value >> shift) & 0x7) << 20;
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns using inline assembly (x86) */
NOINLINE static uint32_t test_strict_low_part(void) {
    uint32_t result = 0;
    
#ifdef __x86_64__ || __i386__
    /* Byte operations that may generate STRICT_LOW_PART */
    uint8_t byte_val = g_value & 0xFF;
    uint16_t word_val = g_value & 0xFFFF;
    
    /* Inline assembly with constraints that modify partial registers */
    asm volatile (
        "movb %1, %b0\n\t"          /* Move byte - may generate STRICT_LOW_PART */
        : "=r"(result)
        : "r"(byte_val)
        : "cc"
    );
    
    /* Another potential STRICT_LOW_PART for word operations */
    asm volatile (
        "movw %1, %w0\n\t"          /* Move word - may generate STRICT_LOW_PART */
        : "+r"(result)
        : "r"(word_val)
        : "cc"
    );
#else
    /* Fallback for non-x86: use bit-field operations that might also generate
       similar patterns through compiler optimizations */
    union type_pun tp;
    tp.full = g_value;
    result = tp.bytes[g_index1 & 3];  /* Byte access */
    result |= (tp.halves.low << 8);   /* Half-word access */
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns through type conversions and unions */
NOINLINE static uint32_t test_subreg(void) {
    union type_pun tp;
    uint32_t result = 0;
    
    /* Initialize with global value */
    tp.full = g_value;
    
    /* Multiple type conversions that may generate SUBREG */
    uint16_t low_half = tp.full;           /* Truncation: uint32_t -> uint16_t */
    uint16_t high_half = tp.full >> 16;    /* Shift and truncation */
    
    /* Access through different-sized views */
    int8_t byte1 = tp.bytes[0];
    int8_t byte2 = tp.bytes[1];
    int16_t half1 = tp.halves.low;
    int16_t half2 = tp.halves.high;
    
    /* Mix operations to force register usage */
    result = low_half + (high_half << 16);
    result += byte1 + byte2;
    result += half1 - half2;
    
    /* Cast between different integer sizes */
    int64_t large = g_value;
    int32_t medium = large;          /* May generate SUBREG for truncation */
    int16_t small = medium;          /* Another potential SUBREG */
    
    result += small;
    
    return result;
}

/* Function 4: Generate complex MEM_P patterns with addressing modes */
NOINLINE static uint32_t test_mem_operands(void) {
    /* Multi-dimensional array with variable indexing */
    int array[8][8];
    static int static_array[16][16];  /* Static for different addressing */
    
    /* Initialize arrays */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            array[i][j] = i * 8 + j;
        }
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            static_array[i][j] = i * 16 + j;
        }
    }
    
    uint32_t result = 0;
    volatile int idx1 = g_index1;
    volatile int idx2 = g_index2;
    
    /* Complex memory addressing patterns */
    result += array[idx1][idx2 & 7];                 /* 2D array with variable indices */
    result += static_array[idx2 & 15][idx1 & 15];    /* Static array access */
    
    /* Pointer arithmetic with non-constant offsets */
    int *ptr = &array[0][0];
    result += *(ptr + idx1 * 8 + idx2);              /* Linearized array access */
    
    /* Structure field access through pointer */
    struct {
        int a;
        int b;
        int c[4];
    } s = {1, 2, {3, 4, 5, 6}};
    
    result += s.c[idx1 & 3];                         /* Structure array field */
    
    /* Memory access with scaled index */
    for (int i = 0; i < 4; i++) {
        result += array[i][i] * i;                   /* Loop with memory access */
    }
    
    return result;
}

/* Function 5: Combined test with all patterns in a loop */
NOINLINE static uint32_t test_combined(void) {
    uint32_t total = 0;
    
    /* Loop to increase RTL generation opportunities */
    for (int i = 0; i < 10; i++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_operands();
        
        /* Modify global to prevent loop elimination */
        g_value ^= total;
    }
    
    return total;
}

/* Main function that calls all test patterns */
int main(void) {
    uint32_t result = 0;
    
    /* Individual pattern tests */
    result += test_zero_extract();
    result += test_strict_low_part();
    result += test_subreg();
    result += test_mem_operands();
    
    /* Combined test with loop */
    result += test_combined();
    
    /* Use result to prevent dead code elimination */
    volatile uint32_t sink = result;
    
    /* Simple validation - just ensure we don't crash */
    assert(sink != 0xDEADBEEF);  /* Arbitrary check */
    
    return (sink > 0) ? 0 : 1;
}
