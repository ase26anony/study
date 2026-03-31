/* test_resource_coverage.c
 * 
 * This program is designed to generate RTL patterns that will exercise
 * the uncovered lines in GCC's resource.cc (lines 282-290):
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register accesses
 * - SUBREG for subregister operations
 * - MEM_P with complex addressing for memory operations
 */

#include <stdint.h>
#include <assert.h>

/* Force compilation with optimization */
#ifndef __OPTIMIZE__
#error "This test requires optimization (-O2 or -O3) to generate the required RTL patterns"
#endif

/* Prevent inlining to ensure separate functions generate RTL */
#define NOINLINE __attribute__((noinline))

/* Structure with bit-fields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    unsigned int padding : 8;
};

/* Global variables to prevent constant propagation */
volatile int g_var1 = 0x12345678;
volatile int g_var2 = 0x9ABCDEF0;
volatile int g_var3 = 0x13579BDF;
int g_array[256] = {0};
int g_index1 = 5;
int g_index2 = 10;

/* Function 1: Generate ZERO_EXTRACT patterns through bit-field operations */
NOINLINE static int test_zero_extract(void) {
    struct bitfield_struct bf;
    int result = 0;
    
    /* Multiple bit-field operations that may generate ZERO_EXTRACT */
    bf.field1 = (g_var1 >> 4) & 0xF;      /* Extract 4 bits */
    bf.field2 = (g_var2 >> 8) & 0xFF;     /* Extract 8 bits */
    bf.field3 = (g_var3 >> 12) & 0xFFF;   /* Extract 12 bits */
    
    /* Combine with shifting to ensure complex RTL */
    result = (bf.field1 << 16) | (bf.field2 << 8) | bf.field3;
    
    /* Additional bit-field extraction */
    unsigned int combined = g_var1 ^ g_var2;
    result ^= (combined >> 4) & 0x0F0F0F0F;
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns using inline assembly (x86) */
NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    
    /* x86-specific inline assembly for byte operations */
#ifdef __x86_64__ || __i386__
    unsigned char byte_val;
    unsigned short word_val;
    unsigned int dword_val;
    
    /* Byte operation that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q" (byte_val)
        : "r" ((unsigned char)g_var1)
        : "cc"
    );
    
    /* Word operation */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r" (word_val)
        : "r" ((unsigned short)g_var2)
        : "cc"
    );
    
    /* Mix operations to force partial register updates */
    dword_val = g_var3;
    asm volatile (
        "movb %b1, %b0\n\t"
        : "+r" (dword_val)
        : "r" ((unsigned char)byte_val)
        : "cc"
    );
    
    result = byte_val + word_val + dword_val;
#else
    /* Fallback for non-x86: use type punning to generate partial accesses */
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } converter;
    
    converter.full = g_var1;
    converter.parts.low = (uint16_t)g_var2;  /* May generate partial register access */
    result = converter.full;
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns through type conversions */
NOINLINE static int test_subreg(void) {
    int result = 0;
    
    /* Various type conversions that may generate SUBREG */
    long long big_val = (long long)g_var1 * g_var2;
    int int_val = (int)big_val;           /* Truncation */
    short short_val = (short)int_val;     /* Further truncation */
    char char_val = (char)short_val;      /* More truncation */
    
    /* Access different parts of larger types */
    union {
        uint64_t full64;
        uint32_t halves[2];
        uint16_t words[4];
        uint8_t bytes[8];
    } pun;
    
    pun.full64 = (uint64_t)g_var1 | ((uint64_t)g_var2 << 32);
    
    /* Mix different-sized accesses */
    result = pun.halves[0] + pun.words[2] + pun.bytes[5];
    
    /* Pointer-based type punning */
    int32_t *ptr32 = (int32_t*)&big_val;
    int16_t *ptr16 = (int16_t*)&big_val;
    result += *ptr32 + *ptr16;
    
    return result + char_val;
}

/* Function 4: Generate MEM_P patterns with complex addressing */
NOINLINE static int test_mem_addressing(void) {
    int result = 0;
    
    /* Complex array indexing with variable offsets */
    for (int i = 0; i < 16; i++) {
        /* Multi-dimensional access with variable indices */
        int idx1 = (g_index1 + i) & 0xFF;
        int idx2 = (g_index2 + i * 3) & 0xFF;
        
        /* Complex addressing modes */
        result += g_array[idx1] + g_array[idx2];
        result += *(g_array + idx1 * 2) + *(g_array + idx2 * 4);
        
        /* Structure-like access through pointer arithmetic */
        int *ptr = &g_array[0];
        result += ptr[idx1 + 1] - ptr[idx2 - 1];
    }
    
    /* Pointer chasing with arithmetic */
    int *ptr1 = &g_array[g_index1];
    int *ptr2 = &g_array[g_index2];
    
    for (int i = 0; i < 8; i++) {
        result += *ptr1++ + *ptr2--;
        
        /* More complex: *(ptr + offset + i*stride) */
        int offset = i * 3;
        result += *(ptr1 + offset) + *(ptr2 - offset);
    }
    
    return result;
}

/* Function 5: Mixed operations to increase RTL complexity */
NOINLINE static int test_mixed_operations(void) {
    int temp = test_zero_extract();
    
    /* Combine operations in ways that might generate all patterns */
    temp ^= test_strict_low_part();
    temp += test_subreg();
    temp *= test_mem_addressing();
    
    /* Additional bit-twiddling */
    temp = (temp >> 4) & 0x0F0F0F0F;      /* Potential ZERO_EXTRACT */
    temp = (temp << 8) | (temp >> 24);    /* Byte swapping */
    
    return temp;
}

/* Main function that calls all test patterns */
int main(void) {
    int result = 0;
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    /* Execute all test functions multiple times */
    for (int iteration = 0; iteration < 3; iteration++) {
        result += test_zero_extract();
        result += test_strict_low_part();
        result += test_subreg();
        result += test_mem_addressing();
        result += test_mixed_operations();
        
        /* Modify indices to change addressing patterns */
        g_index1 = (g_index1 * 13 + 7) & 0xFF;
        g_index2 = (g_index2 * 17 + 11) & 0xFF;
    }
    
    /* Use result to prevent dead code elimination */
    return result & 0xFF;
}
