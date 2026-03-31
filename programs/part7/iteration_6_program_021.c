/* test_resource_coverage.c
 * 
 * This program is designed to generate RTL patterns that will exercise
 * the uncovered lines in GCC's resource.cc mark_referenced_resources function.
 * Specifically, it aims to produce:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register accesses (x86-specific)
 * - SUBREG for type conversions and partial accesses
 * - MEM_P with complex addressing modes
 */

#include <stdint.h>
#include <assert.h>

/* Compile-time check to ensure optimization is enabled */
#ifndef __OPTIMIZE__
#error "This test requires optimization to be enabled (-O1, -O2, or -O3)"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* ========== Function 1: Generate ZERO_EXTRACT patterns ========== */
NOINLINE static uint32_t test_zero_extract(void) {
    /* Use volatile to prevent constant propagation */
    volatile uint32_t source = 0x12345678;
    volatile uint32_t shift_amount = 8;
    volatile uint32_t mask_width = 12;
    
    /* Bit-field extraction that may generate ZERO_EXTRACT in RTL */
    uint32_t result = (source >> shift_amount) & ((1U << mask_width) - 1);
    
    /* Another pattern: extract bit-field from structure */
    struct {
        volatile uint32_t field1 : 8;
        volatile uint32_t field2 : 12;
        volatile uint32_t field3 : 12;
    } bitfield = {0xAA, 0xBBB, 0xCCC};
    
    result += bitfield.field2;
    
    return result;
}

/* ========== Function 2: Generate STRICT_LOW_PART patterns (x86-specific) ========== */
NOINLINE static uint32_t test_strict_low_part(void) {
    uint32_t result = 0;
    
    /* x86-specific inline assembly that may generate STRICT_LOW_PART */
#if defined(__i386__) || defined(__x86_64__)
    uint16_t word_var = 0x1234;
    uint8_t byte_var = 0xAB;
    
    /* Byte operation that may use partial register */
    asm volatile (
        "movb %1, %b0\n\t"  /* Move byte to low part of register */
        : "=r"(word_var)
        : "r"(byte_var)
        : /* No clobbers */
    );
    
    /* Word operation that may use partial register */
    uint32_t dword_var = 0x12345678;
    asm volatile (
        "movw %1, %w0\n\t"  /* Move word to low part of register */
        : "+r"(dword_var)
        : "r"(word_var)
        : /* No clobbers */
    );
    
    result = dword_var;
#endif
    
    /* Fallback for non-x86: use bit-field operations that might generate
       similar partial access patterns */
#ifndef __i386__
#ifndef __x86_64__
    volatile uint32_t val = 0x12345678;
    /* Access low byte through masking */
    result = (val & 0xFF);
    /* Access low word through masking */
    result |= (val & 0xFFFF) << 16;
#endif
#endif
    
    return result;
}

/* ========== Function 3: Generate SUBREG patterns ========== */
NOINLINE static uint32_t test_subreg(void) {
    uint32_t result = 0;
    
    /* Type conversions that may generate SUBREG */
    volatile int64_t large_val = 0x123456789ABCDEF0LL;
    
    /* Truncation to smaller type */
    int32_t truncated = (int32_t)large_val;
    result += truncated;
    
    /* Sign extension */
    volatile int16_t short_val = -32768;
    int32_t extended = (int32_t)short_val;  /* May generate SUBREG for sign extension */
    result += extended;
    
    /* Access halves of 64-bit value */
    union {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } parts;
    } splitter;
    
    splitter.full = 0x123456789ABCDEF0ULL;
    result += splitter.parts.low;   /* May involve SUBREG */
    result += splitter.parts.high;  /* May involve SUBREG */
    
    /* Pointer casting between different sized types */
    volatile uint32_t array[4] = {1, 2, 3, 4};
    uint16_t *half_ptr = (uint16_t *)array;
    result += half_ptr[1];  /* Access 16-bit subpart of 32-bit element */
    
    return result;
}

/* ========== Function 4: Generate complex MEM_P patterns ========== */
NOINLINE static uint32_t test_mem_complex_address(void) {
    volatile uint32_t result = 0;
    
    /* Multi-dimensional array with variable indices */
    volatile uint32_t matrix[10][10];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    volatile int idx1 = 3;
    volatile int idx2 = 7;
    
    /* Complex addressing: matrix[idx1][idx2] */
    result += matrix[idx1][idx2];
    
    /* Pointer arithmetic with variable offset */
    volatile uint32_t *ptr = &matrix[0][0];
    volatile int offset = idx1 * 10 + idx2;
    result += *(ptr + offset);  /* Another memory access with computed address */
    
    /* Structure with multiple fields */
    struct {
        uint32_t a;
        uint32_t b;
        uint32_t c;
        uint32_t array[5];
    } mystruct;
    
    mystruct.a = 100;
    mystruct.b = 200;
    mystruct.c = 300;
    for (int i = 0; i < 5; i++) {
        mystruct.array[i] = i * 10;
    }
    
    /* Access structure fields with pointer */
    volatile uint32_t *struct_ptr = &mystruct.a;
    result += struct_ptr[1];  /* Access mystruct.b */
    result += struct_ptr[idx1]; /* Variable index into structure */
    
    /* Post-increment addressing pattern */
    volatile uint32_t *walk = &mystruct.array[0];
    for (int i = 0; i < 3; i++) {
        result += *walk++;
    }
    
    return result;
}

/* ========== Main function with loop to increase RTL generation ========== */
int main(void) {
    uint32_t total = 0;
    
    /* Loop to increase chance of RTL generation and resource marking */
    for (int i = 0; i < 10; i++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_complex_address();
        
        /* Conditional to prevent loop unrolling from eliminating all RTL */
        if (total > 1000000) {
            total = 0;
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile uint32_t sink = total;
    
    /* Simple validation that we computed something */
    assert(sink != 0xDEADBEEF);
    
    return (int)(sink & 0x7FFFFFFF);  /* Return non-negative value */
}
