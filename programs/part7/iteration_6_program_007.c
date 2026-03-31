/* test_resource_coverage.c
 * 
 * This program is designed to generate RTL patterns that will exercise
 * the uncovered lines in GCC's resource.cc file (lines 282-290).
 * Specifically, it aims to produce:
 * 1. ZERO_EXTRACT - through bit-field operations
 * 2. STRICT_LOW_PART - through inline assembly with partial register constraints
 * 3. SUBREG - through type conversions and partial accesses
 * 4. MEM_P with complex addressing - through pointer arithmetic and array access
 */

#include <stdio.h>
#include <stdint.h>

/* Compile-time check for optimization */
#ifndef __OPTIMIZE__
#warning "Compile with -O2 or -O3 for best coverage results"
#endif

/* Prevent inlining to ensure functions generate separate RTL */
#define NOINLINE __attribute__((noinline))

/* ========== Function 1: Generate ZERO_EXTRACT patterns ========== */
NOINLINE static int bitfield_operations(void) {
    /* Use volatile to prevent constant propagation */
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int mask = 0x00000FF0;
    
    /* These operations may generate ZERO_EXTRACT in RTL */
    unsigned int extracted = (source >> 4) & 0xFF;      /* Simple extraction */
    unsigned int masked = source & mask;                /* Bitfield mask */
    
    /* Complex bitfield extraction with variable shift */
    volatile unsigned int shift = 8;
    unsigned int var_extract = (source >> shift) & 0xF;
    
    /* Structure with bitfields - may generate ZERO_EXTRACT */
    struct {
        unsigned int low : 8;
        unsigned int mid : 12;
        unsigned int high : 12;
    } bitfield_struct;
    
    bitfield_struct.low = extracted & 0xFF;
    bitfield_struct.mid = (source >> 8) & 0xFFF;
    
    return extracted + masked + var_extract + bitfield_struct.mid;
}

/* ========== Function 2: Generate STRICT_LOW_PART patterns ========== */
NOINLINE static int partial_register_ops(void) {
    int result = 0;
    
    /* x86-specific inline assembly for partial register access */
#if defined(__i386__) || defined(__x86_64__)
    unsigned char byte_val = 0x42;
    unsigned short word_val = 0x1234;
    unsigned int dword_val = 0xDEADBEEF;
    
    /* Byte operations that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q" (byte_val)
        : "r" ((unsigned char)0x55)
        : "cc"
    );
    
    /* Word operations */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r" (word_val)
        : "r" ((unsigned short)0x5678)
        : "cc"
    );
    
    /* Mixed-size operations */
    unsigned int temp;
    asm volatile (
        "movb %%al, %0\n\t"
        : "=m" (temp)
        :
        : "al", "cc"
    );
    
    result = byte_val + word_val + temp;
#else
    /* Fallback for non-x86: use volatile byte operations */
    volatile uint32_t reg = 0x12345678;
    volatile uint8_t *byte_ptr = (volatile uint8_t *)&reg;
    
    /* Access individual bytes - may generate partial register ops */
    uint8_t low_byte = *byte_ptr;
    uint8_t high_byte = *(byte_ptr + 3);
    
    /* Modify partial register through memory */
    *byte_ptr = 0xAA;
    *(byte_ptr + 1) = 0xBB;
    
    result = low_byte + high_byte + reg;
#endif
    
    return result;
}

/* ========== Function 3: Generate SUBREG patterns ========== */
NOINLINE static int subreg_conversions(void) {
    volatile long long big_val = 0x1122334455667788LL;
    volatile int int_val = 0xDEADBEEF;
    volatile short short_val = 0x1234;
    volatile char char_val = 0x42;
    
    /* Type conversions that may generate SUBREG */
    int from_short = short_val;                 /* short -> int */
    short from_int = int_val;                   /* int -> short */
    int from_char = char_val;                   /* char -> int */
    
    /* Access halves of 64-bit value */
    int low_half = (int)big_val;                /* truncate to 32 bits */
    int high_half = (int)(big_val >> 32);       /* access high 32 bits */
    
    /* Pointer casting for type punning */
    int int_from_ll = *(int*)&big_val;          /* access first 4 bytes */
    short short_from_int = *(short*)&int_val;   /* access first 2 bytes */
    
    /* Union for type punning - may generate SUBREG */
    union {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } parts;
    } converter;
    
    converter.full = big_val;
    uint32_t union_low = converter.parts.low;
    uint32_t union_high = converter.parts.high;
    
    return from_short + from_int + from_char + low_half + high_half + 
           int_from_ll + short_from_int + union_low + union_high;
}

/* ========== Function 4: Generate MEM_P with complex addressing ========== */
NOINLINE static int complex_memory_access(void) {
    volatile int array[256];
    volatile int matrix[16][16];
    volatile int *ptr = (int*)array;
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    int sum = 0;
    volatile int index1 = 10;
    volatile int index2 = 20;
    
    /* Complex addressing modes that may generate interesting MEM_P patterns */
    
    /* 1. Array indexing with variable offset */
    sum += array[index1];
    sum += array[index1 + index2];
    sum += array[index1 * 3];
    
    /* 2. Pointer arithmetic */
    sum += *(ptr + index1);
    sum += *(ptr + index1 * 2);
    sum += ptr[index2 - index1];
    
    /* 3. Multi-dimensional array with variable indices */
    sum += matrix[index1 & 0xF][index2 & 0xF];
    sum += matrix[5][index1 % 8];
    
    /* 4. Structure-like access through pointer arithmetic */
    struct {
        int a, b, c, d;
    } mystruct;
    
    mystruct.a = 100;
    mystruct.b = 200;
    mystruct.c = 300;
    mystruct.d = 400;
    
    int *struct_ptr = &mystruct.a;
    sum += struct_ptr[0];  /* mystruct.a */
    sum += struct_ptr[2];  /* mystruct.c */
    
    /* 5. Complex expression in address calculation */
    sum += array[(index1 * index2) & 0xFF];
    sum += array[(index1 << 2) + (index2 >> 1)];
    
    return sum;
}

/* ========== Main function with loop to increase RTL generation ========== */
int main(void) {
    int total = 0;
    
    /* Loop to increase chance of RTL pattern generation and resource marking */
    for (int i = 0; i < 100; i++) {
        total += bitfield_operations();
        total += partial_register_ops();
        total += subreg_conversions();
        total += complex_memory_access();
        
        /* Prevent loop elimination */
        if (i % 10 == 0) {
            volatile int dummy = total;
            (void)dummy;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total % 1000);
    
    return total > 0 ? 0 : 1;
}
