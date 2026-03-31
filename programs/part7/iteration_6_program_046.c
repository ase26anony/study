/*
 * Test program to cover lines 282-290 in GCC's resource.cc
 * Generates RTL patterns with ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P
 */

#include <stdint.h>
#include <assert.h>

/* Force functions to not be inlined to ensure RTL generation */
#define NOINLINE __attribute__((noinline))

/* Ensure optimization is enabled for RTL pattern generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization enabled (-O2 or -O3)"
#endif

/* Global volatile variables to prevent optimization */
volatile unsigned int global_volatile = 0x12345678;
volatile int global_index = 0;

/* ========== Function 1: Generate ZERO_EXTRACT RTL ========== */
NOINLINE static unsigned int test_zero_extract(void) {
    /* Bit-field extraction operations that may generate ZERO_EXTRACT */
    volatile unsigned int x = global_volatile;
    
    /* Multiple bit-field extraction patterns */
    unsigned int result = 0;
    
    /* Pattern 1: Shift and mask - classic ZERO_EXTRACT candidate */
    result = (x >> 4) & 0xFF;           /* Extract bits 4-11 */
    
    /* Pattern 2: Nested extractions */
    result += ((x >> 8) & 0xF) | ((x >> 16) & 0xF0);
    
    /* Pattern 3: Variable bit position extraction */
    int shift = global_index & 0x7;
    result += (x >> shift) & 0x3F;
    
    /* Pattern 4: Bit-field struct (may also generate ZERO_EXTRACT) */
    struct {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } bits = { .a = (x & 0xF), .b = ((x >> 4) & 0xFF), .c = ((x >> 12) & 0xF) };
    
    result += bits.a + bits.b + bits.c;
    
    return result;
}

/* ========== Function 2: Generate STRICT_LOW_PART RTL ========== */
NOINLINE static unsigned int test_strict_low_part(void) {
    unsigned int result = 0;
    
#ifdef __x86_64__ || __i386__
    /* x86-specific inline assembly that may generate STRICT_LOW_PART */
    unsigned char byte_val;
    unsigned short word_val;
    unsigned int dword_val = global_volatile;
    
    /* Byte operation - may generate STRICT_LOW_PART for partial register update */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q" (byte_val)
        : "r" ((unsigned char)dword_val)
        : "cc"
    );
    result += byte_val;
    
    /* Word operation */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r" (word_val)
        : "r" ((unsigned short)dword_val)
        : "cc"
    );
    result += word_val;
    
    /* Mixed-size operations */
    unsigned int temp = 0;
    asm volatile (
        "movl %1, %0\n\t"
        "andb $0xF, %b0\n\t"  /* Modify only low byte */
        : "=r" (temp)
        : "r" (dword_val)
        : "cc"
    );
    result += temp;
#else
    /* Generic fallback: operations on different-sized types */
    unsigned int val = global_volatile;
    unsigned char low_byte = val & 0xFF;
    unsigned short low_word = val & 0xFFFF;
    
    /* Type punning through union may generate partial register accesses */
    union {
        unsigned int full;
        struct {
            unsigned short low;
            unsigned short high;
        } parts;
    } converter;
    
    converter.full = val;
    converter.parts.low = low_word;  /* Partial update */
    
    result = low_byte + converter.full;
#endif
    
    return result;
}

/* ========== Function 3: Generate SUBREG RTL ========== */
NOINLINE static unsigned int test_subreg(void) {
    unsigned int result = 0;
    
    /* Operations between different-sized types generate SUBREG */
    int int_val = global_volatile;
    short short_val = int_val;          /* Truncation - SUBREG */
    result += short_val;
    
    /* Sign extension */
    signed char char_val = int_val;
    int extended = char_val;            /* Extension - SUBREG */
    result += extended;
    
    /* Type punning through pointers */
    long long big_val = 0x123456789ABCDEF0LL;
    int *ptr = (int*)&big_val;
    result += ptr[0] + ptr[1];          /* Access halves - SUBREG */
    
    /* Union for type conversion */
    union {
        float f;
        int i;
    } u;
    u.f = 3.14159f;
    result += u.i;                      /* Bitcast - may use SUBREG */
    
    /* Array of different types */
    char bytes[8];
    *(long long*)bytes = big_val;       /* Cast between types - SUBREG */
    for (int i = 0; i < 8; i++) {
        result += bytes[i];
    }
    
    return result;
}

/* ========== Function 4: Generate MEM_P with complex addressing ========== */
NOINLINE static unsigned int test_mem_operands(void) {
    unsigned int result = 0;
    
    /* Complex array accesses with variable indices */
    int array[256][4];
    int *ptr_array[128];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 4; j++) {
            array[i][j] = i * 4 + j;
        }
    }
    
    for (int i = 0; i < 128; i++) {
        ptr_array[i] = &array[i*2][0];
    }
    
    /* Complex memory addressing patterns */
    volatile int idx = global_index;
    
    /* Pattern 1: Multi-dimensional array with variable index */
    result += array[idx % 256][(idx >> 2) % 4];
    
    /* Pattern 2: Pointer arithmetic with scaling */
    int *ptr = &array[0][0];
    result += ptr[idx * 3 + 1];
    
    /* Pattern 3: Indirect pointer access */
    result += *(ptr_array[idx % 128] + (idx % 4));
    
    /* Pattern 4: Structure field access */
    struct {
        int a;
        int b[4];
        struct {
            int x;
            int y;
        } nested;
    } s;
    
    s.a = idx;
    for (int i = 0; i < 4; i++) s.b[i] = idx + i;
    s.nested.x = idx * 2;
    s.nested.y = idx * 3;
    
    result += s.a + s.b[idx % 4] + s.nested.x;
    
    /* Pattern 5: Memory with displacement */
    result += *(&s.a + (idx & 1));
    
    return result;
}

/* ========== Function 5: Combined patterns ========== */
NOINLINE static unsigned int test_combined(void) {
    /* Combine all patterns in one function for maximum interaction */
    unsigned int result = 0;
    
    /* Memory access with bit-field extraction */
    volatile unsigned int *mem_ptr = &global_volatile;
    result = (*mem_ptr >> 4) & 0xF;          /* MEM + ZERO_EXTRACT */
    
    /* Type conversion of memory result */
    short short_result = result;             /* SUBREG */
    
    /* Partial register update based on memory value */
#ifdef __x86_64__ || __i386__
    unsigned char byte_out;
    asm volatile (
        "movb (%1), %0\n\t"
        : "=q" (byte_out)
        : "r" (mem_ptr)
        : "memory"
    );
    result += byte_out;                      /* STRICT_LOW_PART from memory */
#else
    result += (unsigned char)(*mem_ptr);
#endif
    
    /* Complex addressing calculation */
    int array[64];
    for (int i = 0; i < 64; i++) array[i] = i;
    
    int idx = global_index;
    result += array[(idx * 7) % 64] +        /* Complex MEM addressing */
              array[((idx << 2) + 1) % 64];
    
    return result + short_result;
}

/* ========== Main function with loop ========== */
int main(void) {
    unsigned int total = 0;
    
    /* Loop to increase chance of RTL generation and resource marking */
    for (int i = 0; i < 100; i++) {
        global_index = i;  /* Update volatile to affect all functions */
        
        /* Call all pattern functions */
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_operands();
        total += test_combined();
        
        /* Prevent loop optimization */
        if (total > 0xFFFFFFF) {
            total = total % 1000;
        }
    }
    
    /* Use result to prevent dead code elimination */
    return (int)(total % 256);
}
