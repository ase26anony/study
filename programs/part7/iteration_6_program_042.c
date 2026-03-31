/* test_resource_coverage.c
 * This program is designed to generate RTL patterns that will exercise
 * the uncovered lines in GCC's resource.cc (lines 282-290) during compilation.
 * The patterns target: ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P.
 */

#include <stdio.h>
#include <stdint.h>

/* Compile-time check for optimization */
#ifndef __OPTIMIZE__
#warning "Compile with -O2 or -O3 for best coverage results"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* ========== Function 1: Generate ZERO_EXTRACT patterns ========== */
NOINLINE static int test_zero_extract(void) {
    /* Use volatile to prevent constant folding */
    volatile uint32_t x = 0x12345678;
    volatile uint32_t y = 0x9ABCDEF0;
    
    /* Bit-field extraction operations that may generate ZERO_EXTRACT */
    uint32_t result = 0;
    
    /* Multiple bit-field extractions */
    result |= (x >> 4) & 0xF;        /* Extract 4 bits */
    result |= (y >> 8) & 0xFF;       /* Extract 8 bits */
    result |= (x >> 16) & 0x3;       /* Extract 2 bits */
    result |= (y >> 24) & 0x7;       /* Extract 3 bits */
    
    /* Nested extractions */
    uint32_t temp = (x >> 12) & 0xFFF;
    result |= (temp >> 4) & 0xF;
    
    return (int)result;
}

/* ========== Function 2: Generate STRICT_LOW_PART patterns ========== */
NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    
    /* Use inline assembly for byte operations (x86/x86-64 specific) */
    #if defined(__x86_64__) || defined(__i386__)
    uint32_t val32 = 0x12345678;
    uint16_t val16 = 0;
    uint8_t val8 = 0;
    
    /* Byte operations that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(val8) 
        : "r"((uint8_t)val32)
        : "cc"
    );
    
    /* Word operations */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r"(val16)
        : "r"((uint16_t)val32)
        : "cc"
    );
    
    result = val8 + val16;
    
    #else
    /* Fallback for non-x86: use bit-field structures */
    struct {
        unsigned int low8 : 8;
        unsigned int high24 : 24;
    } bitfield;
    
    bitfield.low8 = 0x78;
    bitfield.high24 = 0x123456;
    result = bitfield.low8;
    #endif
    
    return result;
}

/* ========== Function 3: Generate SUBREG patterns ========== */
NOINLINE static int test_subreg(void) {
    int result = 0;
    
    /* Type conversions that may generate SUBREG */
    int32_t i32 = 0x12345678;
    int16_t i16 = i32;               /* Truncation */
    int8_t i8 = i32;                 /* Further truncation */
    
    /* Access different parts of larger types */
    int64_t i64 = 0x123456789ABCDEF0LL;
    i32 = i64;                       /* 64-bit to 32-bit */
    i16 = i64;                       /* 64-bit to 16-bit */
    
    /* Use unions for type punning */
    union {
        uint32_t u32;
        uint16_t u16[2];
        uint8_t u8[4];
    } converter;
    
    converter.u32 = 0xDEADBEEF;
    result = converter.u16[0] + converter.u8[1];
    
    /* Pointer casting for sub-register access */
    uint32_t *ptr32 = &converter.u32;
    uint16_t *ptr16 = (uint16_t *)ptr32;
    result += ptr16[0] + ptr16[1];
    
    return result;
}

/* ========== Function 4: Generate MEM_P with complex addressing ========== */
NOINLINE static int test_mem_operands(void) {
    volatile int arr[256][256];  /* Large array to prevent optimization */
    volatile int *ptr = (int *)arr;
    int result = 0;
    
    /* Complex addressing modes */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            /* Multi-dimensional array access with variable indices */
            result += arr[i][j];
            
            /* Pointer arithmetic with non-constant offsets */
            result += *(ptr + i * 256 + j);
            
            /* Structure-like access using pointer arithmetic */
            int *row_ptr = (int *)&arr[i][0];
            result += row_ptr[j * 2];
        }
    }
    
    /* Additional memory patterns */
    struct {
        int a;
        int b;
        int c[4];
    } s;
    
    s.a = 1;
    s.b = 2;
    for (int i = 0; i < 4; i++) {
        s.c[i] = i * 3;
        result += s.c[i];  /* Structure field access */
    }
    
    return result;
}

/* ========== Main function with loop to force RTL generation ========== */
int main(void) {
    int total = 0;
    
    /* Call each test function multiple times in a loop
     * This increases the chance of RTL generation and resource marking
     */
    for (int i = 0; i < 10; i++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_operands();
        
        /* Add a conditional to prevent loop unrolling */
        if (total > 1000000) {
            total = 0;  /* This should never happen */
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
