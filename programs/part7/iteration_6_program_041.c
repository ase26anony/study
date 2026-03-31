/* test_resource_coverage.c
 * 
 * This program is designed to generate RTL patterns that will trigger
 * the uncovered lines in GCC's resource.cc (lines 282-290) during
 * compilation with optimization enabled.
 */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for proper RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with -O2 or -O3 to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* ========== Pattern 1: ZERO_EXTRACT (bit-field operations) ========== */
NOINLINE static int bitfield_extract(void) {
    /* Use volatile to prevent constant propagation */
    volatile unsigned int source = 0x12345678;
    volatile unsigned int mask = 0x00000F00;
    
    /* This should generate ZERO_EXTRACT RTL for bit-field extraction */
    unsigned int extracted = (source >> 8) & 0xF;
    
    /* Complex bit-field operation with variable shift */
    volatile unsigned int shift = 4;
    unsigned int var_extract = (source >> shift) & 0x7;
    
    return extracted + var_extract;
}

/* ========== Pattern 2: STRICT_LOW_PART (partial register access) ========== */
NOINLINE static int partial_register_access(void) {
    int result = 0;
    
    /* x86-specific inline assembly for byte register access */
#if defined(__i386__) || defined(__x86_64__)
    volatile uint32_t full_reg = 0xDEADBEEF;
    uint8_t low_byte;
    
    /* This may generate STRICT_LOW_PART for byte operation */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q" (low_byte)      /* =q constraint for byte-addressable register */
        : "r" ((uint8_t)full_reg)
        : /* no clobbers */
    );
    
    result += low_byte;
    
    /* Half-word access might also generate partial register patterns */
    uint16_t low_word;
    asm volatile (
        "movw %1, %0\n\t"
        : "=r" (low_word)
        : "r" ((uint16_t)full_reg)
    );
    
    result += low_word;
#endif
    
    /* Generic fallback using unions for partial access */
    union {
        uint32_t dword;
        struct {
            uint16_t low;
            uint16_t high;
        } words;
        struct {
            uint8_t b0, b1, b2, b3;
        } bytes;
    } converter;
    
    converter.dword = 0xAABBCCDD;
    result += converter.words.low;
    result += converter.bytes.b0;
    
    return result;
}

/* ========== Pattern 3: SUBREG (sub-register operations) ========== */
NOINLINE static int subregister_operations(void) {
    volatile long long big_val = 0x1122334455667788LL;
    
    /* Type conversions that may generate SUBREG */
    int truncated = (int)big_val;           /* truncation */
    short half = (short)truncated;          /* further truncation */
    unsigned char byte = (unsigned char)half; /* byte extraction */
    
    /* Access different parts of a larger type */
    union {
        uint64_t full;
        uint32_t halves[2];
        uint16_t quarters[4];
    } splitter;
    
    splitter.full = big_val;
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        sum += splitter.halves[i];  /* May generate SUBREG for 32-bit access */
    }
    for (int i = 0; i < 4; i++) {
        sum += splitter.quarters[i]; /* May generate SUBREG for 16-bit access */
    }
    
    /* Pointer casting for type punning */
    float f = 3.14159f;
    uint32_t int_view = *(uint32_t*)&f;  /* Type punning through pointer */
    
    return sum + byte + int_view;
}

/* ========== Pattern 4: MEM_P with complex addressing ========== */
NOINLINE static int complex_memory_access(void) {
    /* Multi-dimensional array with variable indices */
    volatile int matrix[10][10];
    volatile int indices[4] = {1, 2, 3, 4};
    
    int sum = 0;
    
    /* Complex addressing mode with multiple variables */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Variable index array access - may generate complex MEM addressing */
            sum += matrix[i + indices[0]][j + indices[1]];
        }
    }
    
    /* Pointer arithmetic with variable offsets */
    int array[100];
    volatile int offset1 = 10;
    volatile int offset2 = 20;
    
    sum += *(array + offset1);           /* Simple offset */
    sum += *(array + offset1 + offset2); /* Compound offset */
    
    /* Structure field access */
    struct point {
        int x;
        int y;
        int z;
    } points[5];
    
    volatile int idx = 2;
    sum += points[idx].x;
    sum += points[idx+1].y;
    
    return sum;
}

/* ========== Main driver with loops to amplify RTL generation ========== */
int main(void) {
    int total = 0;
    
    /* Loop to increase chance of RTL pattern generation and resource marking */
    for (int iteration = 0; iteration < 10; iteration++) {
        total += bitfield_extract();
        total += partial_register_access();
        total += subregister_operations();
        total += complex_memory_access();
        
        /* Conditional to prevent loop unrolling from eliminating patterns */
        if (total > 1000) {
            total -= 500;
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile int sink = total;
    
    /* Simple validation */
    if (sink != 0) {
        return 0;  /* Success */
    }
    
    return 1;  /* Should never reach here with proper execution */
}
