/* test_resource_coverage.c
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource.cc mark_referenced_resources function:
 * - ZERO_EXTRACT: bit-field operations
 * - STRICT_LOW_PART: inline assembly with partial register constraints
 * - SUBREG: type conversions and sub-register accesses
 * - MEM_P: complex memory addressing modes
 */

#include <stdint.h>
#include <assert.h>

/* Force functions to not be inlined to ensure RTL generation */
#define NOINLINE __attribute__((noinline))

/* Compile-time check for optimization */
#ifndef __OPTIMIZE__
#error "This test requires optimization (-O2 or -O3) to generate target RTL patterns"
#endif

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_mask = 0xFF;

/* ==================== ZERO_EXTRACT Patterns ==================== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 4;
    volatile unsigned int field4 : 16;
};

NOINLINE static int test_zero_extract(void) {
    volatile struct bitfield_struct bf = {0};
    int result = 0;
    
    /* Multiple bit-field operations that may generate ZERO_EXTRACT */
    bf.field1 = (global_counter >> 2) & 0xF;          /* Potential ZERO_EXTRACT */
    bf.field2 = (global_counter >> 4) & 0xFF;         /* Potential ZERO_EXTRACT */
    bf.field3 = (bf.field2 >> 2) & 0xF;               /* Chained bit-field ops */
    
    /* Explicit bit masking that may generate ZERO_EXTRACT */
    result = (bf.field4 >> 8) & 0xFF;                 /* Likely ZERO_EXTRACT */
    result |= ((global_counter << 4) & 0xF0);         /* Another pattern */
    
    /* Complex expression with multiple extractions */
    for (int i = 0; i < 4; i++) {
        result += ((bf.field4 >> (i * 4)) & 0xF);     /* Loop may unroll to multiple ZERO_EXTRACT */
    }
    
    return result;
}

/* ==================== STRICT_LOW_PART Patterns ==================== */

NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    
    /* Inline assembly for x86/x86_64 that may generate STRICT_LOW_PART */
    #if defined(__i386__) || defined(__x86_64__)
    unsigned char byte_val;
    unsigned short word_val;
    unsigned int dword_val = global_counter;
    
    /* Byte operation - may generate STRICT_LOW_PART for partial register */
    asm volatile ("movb %1, %0\n\t"
                  : "=q" (byte_val)
                  : "r" ((unsigned char)dword_val)
                  : "cc");
    
    /* Word operation */
    asm volatile ("movw %1, %0\n\t"
                  : "=r" (word_val)
                  : "r" ((unsigned short)dword_val)
                  : "cc");
    
    result = byte_val + word_val;
    
    /* Multiple constraints that force partial register updates */
    unsigned int combined;
    asm volatile ("addl %1, %0\n\t"
                  "andb $0xF, %b0\n\t"  /* Operate on low byte */
                  : "+r" (combined)
                  : "r" (global_counter)
                  : "cc");
    result += combined;
    #else
    /* Fallback for non-x86: use volatile byte accesses */
    volatile uint32_t val32 = global_counter;
    volatile uint8_t *byte_ptr = (volatile uint8_t*)&val32;
    result = byte_ptr[0] + byte_ptr[1];  /* May still generate partial reg ops */
    #endif
    
    return result;
}

/* ==================== SUBREG Patterns ==================== */

NOINLINE static int test_subreg(void) {
    int result = 0;
    
    /* Type conversions that may generate SUBREG */
    long long big_val = (long long)global_counter * 1000LL;
    int small_val = (int)big_val;                     /* Potential SUBREG */
    
    /* Access different parts of larger types */
    union {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } parts;
    } converter;
    
    converter.full = (uint64_t)global_counter << 32 | global_counter;
    result = converter.parts.low - converter.parts.high;  /* SUBREG accesses */
    
    /* Pointer casting between different sized types */
    uint32_t array[4] = {0};
    uint16_t *half_ptr = (uint16_t*)array;
    for (int i = 0; i < 8; i++) {
        half_ptr[i] = (uint16_t)(global_counter + i);  /* SUBREG stores */
    }
    
    /* Mixed-size operations */
    short s1 = (short)small_val;
    int i1 = (int)s1;                                  /* SUBREG conversions */
    result += i1;
    
    return result;
}

/* ==================== MEM_P Patterns ==================== */

NOINLINE static int test_mem_p(void) {
    int result = 0;
    
    /* Complex array addressing with variable indices */
    int matrix[10][10];
    volatile int idx1 = global_counter % 10;
    volatile int idx2 = (global_counter * 7) % 10;
    
    /* Multiple complex memory addresses */
    result = matrix[idx1][idx2];                      /* Complex MEM address */
    result += matrix[idx2][idx1];                     /* Another pattern */
    
    /* Pointer arithmetic with non-constant offsets */
    int *ptr = &matrix[0][0];
    for (int i = 0; i < 10; i++) {
        result += ptr[idx1 * i + idx2];               /* Variable offset */
    }
    
    /* Structure with nested arrays */
    struct nested {
        int data[5][5];
        int more[3];
    } nested_struct;
    
    volatile int idx3 = global_counter % 5;
    result += nested_struct.data[idx3][idx3];         /* Nested array access */
    
    /* Memory access through pointer indirection */
    int **ptr_ptr = &ptr;
    result += (*ptr_ptr)[idx1];                       /* Double indirection */
    
    return result;
}

/* ==================== Combined Test ==================== */

NOINLINE static int test_combined(void) {
    /* Mix all patterns in one function */
    int result = 0;
    
    /* ZERO_EXTRACT pattern */
    volatile uint32_t val = global_counter;
    result += (val >> 4) & 0xF;
    
    /* SUBREG pattern through union */
    union {
        uint32_t word;
        uint8_t bytes[4];
    } u;
    u.word = val;
    result += u.bytes[0];  /* Could involve SUBREG */
    
    /* Complex MEM access */
    static int array[100];
    volatile int idx = global_counter % 100;
    result += array[idx * 2 + 1];
    
    return result;
}

/* ==================== Main Driver ==================== */

int main(void) {
    int total = 0;
    
    /* Run each test multiple times to increase RTL generation opportunities */
    for (int i = 0; i < 10; i++) {
        global_counter = i;
        
        total += test_zero_extract();     /* Target ZERO_EXTRACT */
        total += test_strict_low_part();  /* Target STRICT_LOW_PART */
        total += test_subreg();           /* Target SUBREG */
        total += test_mem_p();            /* Target MEM_P */
        total += test_combined();         /* Combined patterns */
        
        /* Prevent loop unrolling from eliminating all RTL */
        if (total > 1000) {
            total %= 1000;
        }
    }
    
    /* Use result to prevent dead code elimination */
    return total & 0xFF;
}
