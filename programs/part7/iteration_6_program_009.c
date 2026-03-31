/*
 * Test program to cover lines 282-290 in GCC's resource.cc
 * Specifically targets ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P patterns
 */

#include <stdint.h>
#include <assert.h>

/* Force optimization level check */
#ifndef __OPTIMIZE__
#error "This test requires optimization (-O2 or -O3) to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 4;
};

/* Function 1: Generate ZERO_EXTRACT patterns through bit-field operations */
NOINLINE static unsigned int test_zero_extract(void) {
    struct bitfield_struct bf = {0};
    unsigned int result = 0;
    
    /* Multiple bit-field operations that may generate ZERO_EXTRACT */
    bf.field1 = 5;
    bf.field2 = 0xAB;
    bf.field3 = 3;
    
    /* Extract and combine bit-fields - may generate ZERO_EXTRACT in RTL */
    result = (bf.field1 << 8) | bf.field2;
    result = (result >> 2) & 0x3FF;  /* Potential ZERO_EXTRACT */
    
    /* More bit manipulations */
    volatile unsigned int x = 0x12345678;
    unsigned int y = (x >> 8) & 0xFFF;  /* Another potential ZERO_EXTRACT */
    
    return result + y;
}

/* Function 2: Generate STRICT_LOW_PART patterns using inline assembly (x86) */
NOINLINE static unsigned int test_strict_low_part(void) {
    unsigned int result = 0;
    
#ifdef __x86_64__ || __i386__
    /* Byte operations that may generate STRICT_LOW_PART */
    unsigned char byte_val = 0x42;
    unsigned int dword_val = 0;
    
    /* Inline assembly that modifies only part of a register */
    asm volatile (
        "movb %1, %b0\n\t"           /* Move byte - may generate STRICT_LOW_PART */
        : "=r"(dword_val)
        : "r"(byte_val)
        : "cc"
    );
    
    /* Another byte operation */
    unsigned short word_val = 0xABCD;
    asm volatile (
        "movw %1, %w0\n\t"           /* Move word - may generate STRICT_LOW_PART */
        : "=r"(result)
        : "r"(word_val)
        : "cc"
    );
    
    result += dword_val;
#else
    /* Fallback for non-x86: use volatile byte accesses */
    volatile unsigned int multi_byte = 0x11223344;
    unsigned char *byte_ptr = (unsigned char *)&multi_byte;
    
    /* Multiple byte accesses that might generate partial register operations */
    byte_ptr[0] = 0xAA;
    byte_ptr[1] = 0xBB;
    result = byte_ptr[0] + byte_ptr[1];
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns through type conversions */
NOINLINE static unsigned int test_subreg(void) {
    unsigned int result = 0;
    
    /* Various type conversions that may generate SUBREG */
    long long big_val = 0x123456789ABCDEF0LL;
    int small_val = (int)big_val;           /* Truncation - potential SUBREG */
    
    /* Access different parts of larger types */
    union {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } parts;
    } converter;
    
    converter.full = 0x8877665544332211ULL;
    result = converter.parts.low + converter.parts.high;  /* May use SUBREG */
    
    /* More conversions */
    double d = 3.14159;
    float f = (float)d;                     /* Type conversion */
    result += (unsigned int)f;
    
    /* Pointer casting between different sizes */
    uint16_t *short_ptr = (uint16_t *)&result;
    result += short_ptr[0] + short_ptr[1];  /* May generate SUBREG accesses */
    
    return result + small_val;
}

/* Function 4: Generate complex MEM_P patterns with addressing modes */
NOINLINE static unsigned int test_mem_operands(void) {
    /* Multi-dimensional array with variable indexing */
    int matrix[10][10];
    volatile int idx1 = 3, idx2 = 7;
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Complex memory accesses with variable indices */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        /* Multiple array accesses with non-constant offsets */
        sum += matrix[idx1][i] + matrix[i][idx2];
        
        /* Pointer arithmetic */
        int *row = matrix[i];
        sum += *(row + idx1) + row[idx2];
    }
    
    /* Structure with pointer chasing */
    struct node {
        int value;
        struct node *next;
    } nodes[5];
    
    for (int i = 0; i < 4; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[4].value = 40;
    nodes[4].next = NULL;
    
    /* Chain of memory accesses */
    struct node *current = &nodes[0];
    while (current) {
        sum += current->value;
        current = current->next;
    }
    
    return sum;
}

/* Function 5: Combined patterns in a loop to force resource marking */
NOINLINE static unsigned int test_combined(void) {
    unsigned int total = 0;
    
    /* Loop with multiple pattern-generating operations */
    for (volatile int i = 0; i < 10; i++) {
        /* Mix different patterns in one loop iteration */
        total += test_zero_extract();
        
        /* Conditional to prevent optimization */
        if (i & 1) {
            total += test_strict_low_part();
        } else {
            total += test_subreg();
        }
        
        /* Memory operations */
        total += test_mem_operands() % 100;
    }
    
    return total;
}

/* Main function that drives all tests */
int main(void) {
    unsigned int result = 0;
    
    /* Compile-time assertion for optimization */
    _Static_assert(__OPTIMIZE__, "Optimization required for RTL pattern generation");
    
    /* Execute all pattern tests */
    result += test_zero_extract();
    result += test_strict_low_part();
    result += test_subreg();
    result += test_mem_operands();
    result += test_combined();
    
    /* Return non-zero result to indicate successful execution */
    return (result != 0) ? 0 : 1;
}
