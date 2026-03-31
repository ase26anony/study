/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource management code (resource.cc lines 282-290) during compilation.
 * When compiled with optimization (-O2 or -O3), these operations should
 * generate RTL containing ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and
 * complex memory addressing modes.
 */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for proper RTL generation */
#ifndef __OPTIMIZE__
#error "This test requires optimization (-O2 or -O3) to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* ========== Pattern 1: ZERO_EXTRACT (bit-field operations) ========== */
NOINLINE static int test_zero_extract(void) {
    /* Use volatile to prevent constant propagation */
    volatile unsigned int source = 0x12345678;
    volatile unsigned int shift = 8;
    volatile unsigned int mask = 0xFF;
    
    /* This should generate ZERO_EXTRACT RTL for bit-field extraction */
    unsigned int result = (source >> shift) & mask;
    
    /* Additional bit-field operations */
    struct bitfield {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 4;
    } bf;
    
    volatile struct bitfield *bf_ptr = (volatile struct bitfield *)&source;
    result += bf_ptr->field2;  /* Bit-field access may generate ZERO_EXTRACT */
    
    return (int)result;
}

/* ========== Pattern 2: STRICT_LOW_PART (partial register access) ========== */
NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    
    /* STRICT_LOW_PART is often generated for byte/halfword operations */
    volatile short s_val = 0x1234;
    volatile char c_val = 0x56;
    
    /* Type conversions that may generate SUBREG/STRICT_LOW_PART */
    int i_val = s_val;          /* short to int conversion */
    result += i_val;
    
    /* Byte operations */
    unsigned char buffer[4] = {0x11, 0x22, 0x33, 0x44};
    result += buffer[1] + buffer[2];
    
    /* Platform-specific inline assembly for STRICT_LOW_PART */
#if defined(__x86_64__) || defined(__i386__)
    /* Inline assembly that modifies only part of a register */
    unsigned int reg_val = 0;
    asm volatile (
        "movb %1, %b0\n\t"      /* Move byte to lower part of register */
        : "=r"(reg_val)
        : "r"(c_val)
        : "cc"
    );
    result += reg_val;
    
    /* Another assembly pattern that might generate STRICT_LOW_PART */
    unsigned short half_val = 0;
    asm volatile (
        "movw %1, %w0\n\t"      /* Move word to lower part of register */
        : "=r"(half_val)
        : "r"(s_val)
        : "cc"
    );
    result += half_val;
#endif
    
    return result;
}

/* ========== Pattern 3: SUBREG (sub-register operations) ========== */
NOINLINE static int test_subreg(void) {
    int result = 0;
    
    /* Operations on different-sized types generate SUBREG */
    volatile long long ll_val = 0x123456789ABCDEF0LL;
    volatile int i_val = (int)ll_val;      /* Truncation */
    volatile short s_val = (short)i_val;   /* Further truncation */
    
    result += i_val + s_val;
    
    /* Access different parts of a larger type */
    union {
        long long full;
        struct {
            int low;
            int high;
        } parts;
    } u;
    
    u.full = ll_val;
    result += u.parts.low - u.parts.high;  /* May generate SUBREG accesses */
    
    /* Pointer casting between different types */
    volatile int *iptr = (volatile int *)&ll_val;
    result += iptr[0] + iptr[1];  /* Access halves of long long */
    
    /* Mixed-size arithmetic */
    volatile char c_array[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    for (int i = 0; i < 8; i++) {
        result += c_array[i] * i;  /* char promoted to int may use SUBREG */
    }
    
    return result;
}

/* ========== Pattern 4: MEM_P with complex addressing ========== */
NOINLINE static int test_mem_complex_address(void) {
    int result = 0;
    
    /* Multi-dimensional array with variable indices */
    volatile int matrix[10][10];
    volatile int idx1 = 3, idx2 = 7;
    
    /* Complex addressing mode */
    result += matrix[idx1][idx2];
    result += matrix[idx2][idx1];
    
    /* Pointer arithmetic with non-constant offsets */
    volatile int *ptr = (volatile int *)matrix;
    volatile int offset = 15;
    
    for (int i = 0; i < 5; i++) {
        result += ptr[offset + i];      /* Variable offset */
        result += *(ptr + i * 2);       /* Scaled index */
    }
    
    /* Structure with nested arrays */
    struct nested {
        int data[5][5];
        int extra;
    } nested_struct;
    
    volatile struct nested *ns_ptr = &nested_struct;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            result += ns_ptr->data[i][j];  /* Complex memory addressing */
        }
    }
    
    /* Base + index + displacement addressing */
    volatile int array[100];
    volatile int *base = &array[10];
    for (int i = 0; i < 20; i++) {
        result += base[i * 3];  /* Should generate complex MEM addressing */
    }
    
    return result;
}

/* ========== Main function to drive all patterns ========== */
int main(void) {
    int total = 0;
    
    /* Loop to increase chance of RTL generation and resource marking */
    for (int iteration = 0; iteration < 10; iteration++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_complex_address();
        
        /* Conditional to prevent loop elimination */
        if (total > 1000000) {
            total = 0;  /* Never happens, but compiler doesn't know */
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile int sink = total;
    
    /* Simple validation that code executed */
    assert(sink != 0xDEADBEEF);
    
    return (sink > 0) ? 0 : 1;
}
