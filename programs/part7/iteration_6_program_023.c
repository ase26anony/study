/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource.cc mark_referenced_resources function:
 * 1. ZERO_EXTRACT - via bit-field operations
 * 2. STRICT_LOW_PART - via inline assembly with byte constraints
 * 3. SUBREG - via type conversions and partial accesses
 * 4. MEM_P with complex addressing - via pointer arithmetic and arrays
 */

#include <stdint.h>
#include <stdio.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with -O2 or -O3 to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* ========== Pattern 1: ZERO_EXTRACT (bit-field extraction) ========== */
NOINLINE static int test_zero_extract(void) {
    /* Use volatile to prevent constant folding */
    volatile unsigned int source = 0xABCD1234;
    unsigned int result;
    
    /* Bit-field extraction that may generate ZERO_EXTRACT RTL */
    result = (source >> 8) & 0xFFF;  /* Extract 12 bits starting at bit 8 */
    
    /* More complex bit-field operations */
    struct bitfield {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } bf;
    
    volatile unsigned int val = 0xDEADBEEF;
    bf.a = (val >> 0) & 0xF;
    bf.b = (val >> 4) & 0xFF;
    bf.c = (val >> 12) & 0xF;
    
    return result + bf.a + bf.b + bf.c;
}

/* ========== Pattern 2: STRICT_LOW_PART (partial register access) ========== */
NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    
#ifdef __x86_64__ || __i386__
    /* x86-specific inline assembly that may generate STRICT_LOW_PART */
    unsigned char byte_val;
    unsigned short word_val;
    unsigned int dword_val = 0x12345678;
    
    /* Byte operation that only affects low part of register */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q" (byte_val)    /* =q constraint for byte-addressable register */
        : "r" ((unsigned char)0x42)
        : "cc"
    );
    
    /* Word operation */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r" (word_val)    /* May generate STRICT_LOW_PART for 16-bit */
        : "r" ((unsigned short)0xABCD)
        : "cc"
    );
    
    result = byte_val + word_val;
#else
    /* Generic fallback - use union for partial access */
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } u;
    
    u.full = 0x12345678;
    u.parts.low = 0xABCD;  /* Partial write to low 16 bits */
    result = u.parts.low;
#endif
    
    return result;
}

/* ========== Pattern 3: SUBREG (sub-register access) ========== */
NOINLINE static int test_subreg(void) {
    int result = 0;
    
    /* Type conversions that may generate SUBREG */
    long long big_val = 0x1122334455667788LL;
    int small_val = (int)big_val;           /* Truncation */
    short shorter = (short)small_val;       /* Further truncation */
    
    /* Access different parts of larger types */
    union {
        uint64_t dword;
        uint32_t words[2];
        uint16_t shorts[4];
        uint8_t bytes[8];
    } converter;
    
    converter.dword = 0x8877665544332211ULL;
    result = converter.words[0] + converter.shorts[2] + converter.bytes[5];
    
    /* Mixed-size operations */
    int a = 1000;
    short b = a;          /* Implicit truncation - may generate SUBREG */
    char c = b;           /* Further truncation */
    
    return result + b + c + shorter;
}

/* ========== Pattern 4: MEM_P with complex addressing ========== */
NOINLINE static int test_mem_complex_address(void) {
    volatile int result = 0;
    
    /* Multi-dimensional array with variable indices */
    int matrix[10][10];
    volatile int i = 3, j = 7;
    
    /* Complex addressing modes */
    matrix[i][j] = 42;
    result += matrix[i][j];
    
    /* Pointer arithmetic with non-constant offsets */
    int array[100];
    int *ptr = array;
    volatile int offset = 25;
    
    ptr += offset;
    *ptr = 99;
    result += *(ptr + offset/2);  /* More complex addressing */
    
    /* Structure field access */
    struct point {
        int x;
        int y;
        int z;
    } points[5];
    
    volatile int idx = 2;
    points[idx].x = 10;
    points[idx].y = 20;
    points[idx].z = 30;
    
    result += points[idx].x + points[idx].y + points[idx].z;
    
    /* Even more complex: array of pointers */
    int *ptr_array[10];
    for (int k = 0; k < 10; k++) {
        ptr_array[k] = &array[k * 10];
    }
    
    result += *ptr_array[i] + *ptr_array[j];
    
    return result;
}

/* ========== Main driver with loop to increase RTL processing ========== */
int main(void) {
    int total = 0;
    
    /* Loop to increase chance of RTL generation and resource marking */
    for (int iteration = 0; iteration < 10; iteration++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_complex_address();
        
        /* Add some branching to prevent loop optimization */
        if (iteration % 2 == 0) {
            total += iteration;
        }
    }
    
    /* Simple validation that code executed */
    printf("Result: %d (non-zero indicates execution)\n", total);
    
    /* Return non-zero to indicate success */
    return total != 0 ? 0 : 1;
}
