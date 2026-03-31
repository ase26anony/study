/* test_resource_coverage.c
 * This program generates RTL patterns that should trigger
 * mark_referenced_resources() for ZERO_EXTRACT, STRICT_LOW_PART,
 * SUBREG, and MEM_P with complex addressing.
 */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with -O2 or -O3 for proper RTL generation"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 20;
};

/* Function 1: Generate ZERO_EXTRACT patterns */
NOINLINE static unsigned int test_zero_extract(void) {
    struct bitfield_struct bf = {0};
    unsigned int result = 0;
    
    /* Multiple bit-field operations that may generate ZERO_EXTRACT */
    bf.field1 = 5;
    bf.field2 = 0xA5;
    bf.field3 = 0x12345;
    
    /* Extract and combine bit-fields */
    result = (bf.field1 << 16) | (bf.field2 << 8) | (bf.field3 & 0xFF);
    
    /* Additional bit-field extraction */
    result += ((bf.field3 >> 4) & 0xF);  /* Should generate ZERO_EXTRACT */
    
    /* More complex extraction */
    unsigned int temp = bf.field2;
    result += (temp >> 2) & 0x3F;  /* Another potential ZERO_EXTRACT */
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns (x86-specific) */
NOINLINE static unsigned int test_strict_low_part(void) {
    unsigned int result = 0;
    
#ifdef __x86_64__ || __i386__
    /* Inline assembly that modifies partial registers */
    unsigned char byte_val = 0;
    unsigned short word_val = 0;
    unsigned int dword_val = 0x12345678;
    
    /* Byte operation - may generate STRICT_LOW_PART */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(byte_val)
        : "r"((unsigned char)0x42)
        : "cc"
    );
    
    /* Word operation */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r"(word_val)
        : "r"((unsigned short)0xABCD)
        : "cc"
    );
    
    /* Partial register update */
    asm volatile (
        "movl %1, %0\n\t"
        : "+r"(dword_val)
        : "r"(0x87654321)
        : "cc"
    );
    
    result = byte_val + word_val + dword_val;
#else
    /* Fallback: Use volatile operations that might generate similar patterns */
    volatile unsigned int v = 0x12345678;
    volatile unsigned short *p = (volatile unsigned short*)&v;
    *p = 0xABCD;  /* Partial write to 32-bit variable */
    result = v;
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns */
NOINLINE static unsigned int test_subreg(void) {
    unsigned int result = 0;
    
    /* Type conversions that generate SUBREG */
    long long big_val = 0x123456789ABCDEF0LL;
    int int_val = (int)big_val;           /* Truncation */
    short short_val = (short)int_val;     /* More truncation */
    char char_val = (char)short_val;      /* Further truncation */
    
    /* Access different parts of larger types */
    union {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } parts;
    } u;
    
    u.full = 0x1122334455667788ULL;
    result = u.parts.low + u.parts.high;  /* Accesses generate SUBREG */
    
    /* Pointer casting for sub-parts */
    uint32_t array[2] = {0xDEADBEEF, 0xCAFEBABE};
    uint16_t *half_ptr = (uint16_t*)array;
    result += half_ptr[1];  /* Access 16-bit part of 32-bit element */
    
    /* Mixed-size operations */
    result += int_val + short_val + char_val;
    
    return result;
}

/* Function 4: Generate MEM_P with complex addressing */
NOINLINE static unsigned int test_mem_addressing(void) {
    unsigned int result = 0;
    
    /* Multi-dimensional array with variable indices */
    int matrix[10][10];
    volatile int idx1 = 3, idx2 = 7;
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Complex addressing modes */
    result += matrix[idx1][idx2];
    result += matrix[idx2][idx1];
    
    /* Pointer arithmetic with variables */
    int *ptr = &matrix[0][0];
    result += *(ptr + idx1 * 10 + idx2);
    result += ptr[idx1 * 10 + idx2];
    
    /* Structure with array member */
    struct {
        int data[20];
        int count;
    } s;
    
    s.count = 5;
    for (int i = 0; i < 20; i++) {
        s.data[i] = i * 2;
    }
    
    /* Complex memory access */
    result += s.data[s.count * 2];
    result += s.data[idx1 + idx2];
    
    /* More pointer arithmetic */
    int *p1 = &s.data[0];
    int *p2 = p1 + idx1;
    result += *p2;
    result += p1[idx2];
    
    return result;
}

/* Function 5: Combined patterns in loop */
NOINLINE static unsigned int test_combined(void) {
    unsigned int sum = 0;
    
    /* Loop to increase RTL generation opportunities */
    for (volatile int i = 0; i < 10; i++) {
        /* Mix different patterns */
        sum += test_zero_extract();
        sum += test_strict_low_part();
        sum += test_subreg();
        sum += test_mem_addressing();
        
        /* Prevent loop elimination */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Main function */
int main(void) {
    unsigned int result = 0;
    
    /* Compile-time check for optimization */
    _Static_assert(__OPTIMIZE__, "Optimization must be enabled");
    
    /* Execute all pattern generators */
    result += test_zero_extract();
    result += test_strict_low_part();
    result += test_subreg();
    result += test_mem_addressing();
    result += test_combined();
    
    /* Use result to prevent dead code elimination */
    volatile unsigned int sink = result;
    
    return (sink > 0) ? 0 : 1;
}
