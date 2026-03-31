/*
 * Test program to cover lines 282-290 in GCC's resource.cc
 * These lines handle ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P operations
 */

#include <stdint.h>
#include <stdio.h>

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Compile-time check for optimization */
#ifndef __OPTIMIZE__
#error "Compile with optimization enabled (-O2 or -O3)"
#endif

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile unsigned int bitfield_source = 0xABCD1234;

/* ===== Function 1: Generate ZERO_EXTRACT patterns ===== */
NOINLINE static unsigned int test_zero_extract(void) {
    /* Bit-field extraction operations that may generate ZERO_EXTRACT RTL */
    volatile unsigned int x = bitfield_source;
    
    /* Multiple bit-field extractions with different widths/positions */
    unsigned int result = 0;
    
    /* Extract 4 bits starting at position 3 */
    result += (x >> 3) & 0xF;
    
    /* Extract 8 bits starting at position 12 */
    result += (x >> 12) & 0xFF;
    
    /* Extract single bit at position 7 */
    result += (x >> 7) & 0x1;
    
    /* Extract 16 bits starting at position 8 */
    result += (x >> 8) & 0xFFFF;
    
    return result;
}

/* ===== Function 2: Generate STRICT_LOW_PART patterns ===== */
NOINLINE static unsigned int test_strict_low_part(void) {
    unsigned int result = 0;
    
#ifdef __x86_64__ || __i386__
    /* Inline assembly that modifies only part of a register */
    unsigned char byte_val = 0;
    unsigned short word_val = 0;
    unsigned int dword_val = 0;
    
    /* Byte operation - may generate STRICT_LOW_PART for low 8 bits */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(byte_val)
        : "r"((unsigned char)(global_counter & 0xFF))
        : "cc"
    );
    
    /* Word operation - may generate STRICT_LOW_PART for low 16 bits */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r"(word_val)
        : "r"((unsigned short)(global_counter & 0xFFFF))
        : "cc"
    );
    
    result = byte_val + word_val;
#else
    /* Fallback for non-x86: use bit-field structures */
    struct {
        unsigned int low_byte : 8;
        unsigned int next_byte : 8;
        unsigned int rest : 16;
    } bitfield;
    
    bitfield.low_byte = global_counter & 0xFF;
    bitfield.next_byte = (global_counter >> 8) & 0xFF;
    result = bitfield.low_byte + bitfield.next_byte;
#endif
    
    return result;
}

/* ===== Function 3: Generate SUBREG patterns ===== */
NOINLINE static unsigned int test_subreg(void) {
    unsigned int result = 0;
    
    /* Type conversions that may generate SUBREG RTL */
    int int_val = global_counter;
    
    /* Conversions between different-sized types */
    short short_val = int_val;           /* int -> short */
    char char_val = int_val;             /* int -> char */
    long long long_val = int_val;        /* int -> long long */
    
    /* Access different parts of larger types */
    union {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } parts;
    } converter;
    
    converter.full = 0x123456789ABCDEF0ULL;
    result = converter.parts.low + converter.parts.high;
    
    /* More type punning */
    float float_val = int_val;
    int int_from_float = float_val;
    
    result += short_val + char_val + int_from_float;
    
    return result;
}

/* ===== Function 4: Generate complex MEM_P patterns ===== */
NOINLINE static unsigned int test_mem_operands(void) {
    /* Complex memory addressing modes */
    int array_2d[10][20];
    int *ptr_array[30];
    int result = 0;
    
    /* Initialize with some values */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            array_2d[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 30; i++) {
        ptr_array[i] = &array_2d[i % 10][0];
    }
    
    /* Complex memory accesses with variable indices */
    int idx1 = global_counter % 10;
    int idx2 = (global_counter * 7) % 20;
    int idx3 = (global_counter * 13) % 30;
    
    /* Multiple memory operations with complex addressing */
    result += array_2d[idx1][idx2];                     /* 2D array access */
    result += *(ptr_array[idx3] + idx2);                /* Pointer arithmetic */
    result += *(*(array_2d + idx1) + idx2);            /* Pointer to array */
    
    /* Structure with nested arrays */
    struct {
        int data[5][5];
        int *ptr;
    } nested;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            nested.data[i][j] = i * 10 + j;
        }
    }
    nested.ptr = &nested.data[2][2];
    
    result += nested.data[idx1 % 5][idx2 % 5];
    result += *(nested.ptr + (idx1 % 3));
    
    return result;
}

/* ===== Main function with loop to trigger optimization passes ===== */
int main(void) {
    unsigned int total = 0;
    
    /* Loop to ensure RTL generation and resource marking passes are triggered */
    for (int i = 0; i < 100; i++) {
        global_counter = i;
        
        /* Call all pattern-generating functions */
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_operands();
        
        /* Conditional that compiler can't fully optimize away */
        if (total > 1000000) {
            total = total % 1000000;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %u\n", total);
    
    return (int)(total % 256);
}
