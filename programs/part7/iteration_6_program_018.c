/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource management code (resource.cc lines 282-290). When compiled
 * with optimization, it should generate RTL containing:
 * - ZERO_EXTRACT (bit-field operations)
 * - STRICT_LOW_PART (partial register accesses via inline assembly)
 * - SUBREG (type conversions and partial accesses)
 * - MEM_P with complex addressing (array/pointer operations)
 */

#include <stdint.h>
#include <assert.h>

/* Compile-time check for optimization */
#ifndef __OPTIMIZE__
#error "This test requires optimization (-O1, -O2, or -O3) to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* ===== Function 1: Generate ZERO_EXTRACT RTL ===== */
/* Bit-field operations on volatile variables often produce ZERO_EXTRACT */
NOINLINE static uint32_t bitfield_ops(void) {
    volatile uint32_t source = 0x12345678;
    uint32_t result = 0;
    
    /* Multiple bit-field extractions */
    result = (source >> 4) & 0xF;      /* Extract 4 bits at offset 4 */
    result += (source >> 16) & 0xFF;   /* Extract 8 bits at offset 16 */
    result += (source & 0x3);          /* Extract low 2 bits */
    
    /* Compound extraction that may generate ZERO_EXTRACT */
    uint32_t mask = 0x1F;
    uint32_t shift = 3;
    result += (source >> shift) & mask;
    
    return result;
}

/* ===== Function 2: Generate STRICT_LOW_PART RTL ===== */
/* Inline assembly with partial register constraints */
NOINLINE static uint32_t partial_reg_ops(void) {
    uint32_t result = 0;
    
#ifdef __x86_64__ || __i386__
    /* x86-specific: byte operations often generate STRICT_LOW_PART */
    uint8_t byte_val = 0x42;
    uint32_t dword_val;
    
    /* Move byte to lower part of register */
    asm volatile ("movb %1, %b0"
                  : "=r"(dword_val)
                  : "r"(byte_val)
                  : /* No clobbers */);
    
    /* Multiple byte operations to increase chances */
    uint16_t word_val = 0xABCD;
    uint32_t another_val;
    
    asm volatile ("movw %1, %w0"
                  : "=r"(another_val)
                  : "r"(word_val)
                  : /* No clobbers */);
    
    result = dword_val + another_val;
#else
    /* Generic fallback: operations on small types may also generate partial reg ops */
    uint16_t data16 = 0x1234;
    uint32_t data32 = data16;  /* Zero extension */
    result = data32;
    
    /* Access individual bytes through pointer casting */
    uint32_t composite = 0xAABBCCDD;
    uint8_t low_byte = ((uint8_t*)&composite)[0];
    result += low_byte;
#endif
    
    return result;
}

/* ===== Function 3: Generate SUBREG RTL ===== */
/* Type conversions and partial accesses */
NOINLINE static uint32_t subreg_ops(void) {
    uint32_t result = 0;
    
    /* Mixed-size operations */
    int64_t large_val = 0x1122334455667788LL;
    int32_t small_val = large_val;  /* Truncation - may generate SUBREG */
    result = small_val;
    
    /* Access halves of larger types */
    uint32_t high_half = (large_val >> 32);
    result += high_half;
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m = {1, 2, 3, 4};
    
    /* Access different-sized members */
    result += m.s;  /* short to int conversion */
    result += m.i;
    
    /* Union for type punning */
    union pun {
        uint32_t u32;
        uint16_t u16[2];
        uint8_t u8[4];
    } p;
    
    p.u32 = 0xDEADBEEF;
    result += p.u16[0];  /* Access 16-bit subpart of 32-bit value */
    result += p.u8[1];   /* Access 8-bit subpart */
    
    return result;
}

/* ===== Function 4: Generate MEM_P with complex addressing ===== */
/* Complex memory addressing modes */
NOINLINE static uint32_t complex_mem_ops(int index1, int index2) {
    /* Multi-dimensional array with variable indices */
    int matrix[10][10];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Complex addressing with variable indices */
    int result = matrix[index1][index2];
    result += matrix[index2][index1];
    
    /* Pointer arithmetic with non-constant offsets */
    int *ptr = &matrix[0][0];
    result += *(ptr + index1 * 10 + index2);
    result += *(ptr + index2 * 10 + index1);
    
    /* Structure pointer with field access */
    struct data {
        int a[5];
        int b[5];
    } d;
    
    for (int i = 0; i < 5; i++) {
        d.a[i] = i;
        d.b[i] = i * 2;
    }
    
    struct data *dptr = &d;
    result += dptr->a[index1 % 5];
    result += dptr->b[index2 % 5];
    
    return result;
}

/* ===== Main function with loop to ensure RTL generation ===== */
int main(void) {
    uint32_t total = 0;
    
    /* Loop to increase chance of RTL generation and resource marking */
    for (int i = 0; i < 100; i++) {
        total += bitfield_ops();
        total += partial_reg_ops();
        total += subreg_ops();
        total += complex_mem_ops(i % 5, (i + 1) % 5);
    }
    
    /* Use result to prevent dead code elimination */
    volatile uint32_t sink = total;
    
    /* Simple validation */
    if (total > 0) {
        return 0;  /* Success */
    }
    
    return 1;  /* Should never reach here */
}
