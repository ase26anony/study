/* test_resource_coverage.c
 * 
 * This program is designed to generate RTL patterns that will exercise
 * the uncovered lines in GCC's resource.cc file (lines 282-290).
 * Specifically, it aims to produce:
 * 1. ZERO_EXTRACT for bit-field operations
 * 2. STRICT_LOW_PART for partial register accesses
 * 3. SUBREG for subregister operations
 * 4. MEM_P with complex addressing modes
 */

#include <stdint.h>
#include <stdio.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization enabled (-O2 or -O3) for proper RTL generation"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* ========== ZERO_EXTRACT Patterns ========== */

/* Bit-field structure that may generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
};

NOINLINE static unsigned int test_zero_extract(void) {
    struct bitfield_struct bf;
    unsigned int result = 0;
    
    /* Multiple bit-field operations that may generate ZERO_EXTRACT */
    bf.field1 = 5;
    bf.field2 = 0xAB;
    bf.field3 = 0xDEF;
    
    /* Extract and combine bit-fields */
    result = (bf.field1 << 16) | (bf.field2 << 8) | bf.field3;
    
    /* Additional bit-field extraction */
    unsigned int extracted = (result >> 4) & 0xFFF;  /* 12-bit extraction */
    extracted |= ((result >> 20) & 0xF) << 12;       /* 4-bit extraction */
    
    /* Complex bit manipulation */
    for (int i = 0; i < 4; i++) {
        extracted ^= (result >> (i * 4)) & 0xF;
    }
    
    return extracted;
}

/* ========== STRICT_LOW_PART Patterns ========== */

NOINLINE static uint32_t test_strict_low_part(void) {
    uint32_t result = 0;
    
#ifdef __x86_64__ || __i386__
    /* Inline assembly that may generate STRICT_LOW_PART on x86 */
    uint8_t byte_val = 0x42;
    uint16_t word_val = 0x1234;
    uint32_t dword_val = 0xDEADBEEF;
    
    /* Byte operations that may use partial registers */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q" (byte_val)
        : "r" ((uint8_t)0x55)
        : "cc"
    );
    
    /* Word operation */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r" (word_val)
        : "r" ((uint16_t)0x5678)
        : "cc"
    );
    
    /* Mix of sizes */
    result = byte_val | (word_val << 8);
    
    /* Additional partial register access */
    uint32_t temp = dword_val;
    asm volatile (
        "movb %%al, %0\n\t"
        : "=m" (*(uint8_t*)&temp)
        : "a" ((uint8_t)0x99)
        : "cc"
    );
    
    result ^= temp;
#else
    /* Fallback for non-x86: use volatile accesses to different-sized types */
    volatile uint8_t b1 = 0x11, b2 = 0x22;
    volatile uint16_t w1 = 0x3344;
    volatile uint32_t d1 = 0x55667788;
    
    /* Operations that might generate partial register accesses */
    result = b1;
    result = (result << 8) | b2;
    result = (result << 16) | w1;
    
    /* Type punning through pointer casts */
    uint8_t *ptr = (uint8_t*)&d1;
    for (int i = 0; i < 4; i++) {
        result ^= (ptr[i] << (i * 8));
    }
#endif
    
    return result;
}

/* ========== SUBREG Patterns ========== */

NOINLINE static uint64_t test_subreg(void) {
    uint64_t result = 0;
    
    /* Operations between different-sized types */
    uint32_t dword = 0x12345678;
    uint16_t word = 0x9ABC;
    uint8_t byte = 0xDE;
    
    /* Type conversions that may generate SUBREG */
    result = dword;                     /* 32-bit to 64-bit */
    result = (result << 16) | word;     /* 16-bit to 64-bit */
    result = (result << 8) | byte;      /* 8-bit to 64-bit */
    
    /* Access halves of 64-bit value */
    uint32_t low_part = (uint32_t)result;
    uint32_t high_part = (uint32_t)(result >> 32);
    
    /* Mix them back with shifts */
    result = ((uint64_t)high_part << 32) | low_part;
    
    /* Structure with mixed types */
    struct mixed_types {
        char c;
        short s;
        int i;
        long long ll;
    } mt = {'A', 0x1234, 0x56789ABC, 0xDEADBEEFCAFEBABEULL};
    
    /* Access different parts */
    result ^= mt.c;
    result ^= ((uint64_t)mt.s << 8);
    result ^= ((uint64_t)mt.i << 16);
    result ^= mt.ll;
    
    return result;
}

/* ========== MEM_P with Complex Addressing ========== */

NOINLINE static int test_mem_addressing(int index) {
    /* Multi-dimensional array with variable indexing */
    int arr3d[3][4][5];
    int result = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                arr3d[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Complex memory addressing with variable indices */
    volatile int idx1 = index % 3;
    volatile int idx2 = (index * 2) % 4;
    volatile int idx3 = (index * 3) % 5;
    
    /* Multiple memory accesses with complex addressing */
    result += arr3d[idx1][idx2][idx3];
    result += arr3d[(idx1 + 1) % 3][(idx2 + 1) % 4][idx3];
    result += arr3d[idx1][(idx2 + 2) % 4][(idx3 + 1) % 5];
    
    /* Pointer arithmetic with variable offsets */
    int *ptr = &arr3d[0][0][0];
    for (int i = 0; i < 10; i++) {
        result += ptr[(index + i) % (3 * 4 * 5)];
    }
    
    /* Structure with array member */
    struct with_array {
        int data[7];
        int extra;
    } wa = {{1, 2, 3, 4, 5, 6, 7}, 42};
    
    /* Access with variable index */
    result += wa.data[index % 7];
    result += wa.extra;
    
    return result;
}

/* ========== Main Driver ========== */

int main(void) {
    unsigned int total = 0;
    
    /* Call each test function multiple times to increase RTL generation */
    for (int i = 0; i < 10; i++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_addressing(i);
    }
    
    /* Simple validation - just ensure we computed something */
    printf("Result: %u (0x%08X)\n", total, total);
    
    /* Return non-zero if total is zero (shouldn't happen) */
    return total == 0 ? 1 : 0;
}
