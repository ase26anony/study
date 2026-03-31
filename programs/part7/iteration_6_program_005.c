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

/* Function 1: Generate ZERO_EXTRACT RTL patterns */
NOINLINE static unsigned int test_zero_extract(void) {
    struct bitfield_struct bf = {0};
    unsigned int result = 0;
    
    /* Multiple bit-field operations that may generate ZERO_EXTRACT */
    bf.field1 = 5;
    bf.field2 = 0xAB;
    bf.field3 = 3;
    
    /* Extract and combine bit-fields */
    result = (bf.field1 << 8) | bf.field2;
    result = (result >> 2) & 0x3FF;  /* Potential ZERO_EXTRACT */
    
    /* More bit-field manipulation */
    volatile unsigned int x = 0xDEADBEEF;
    unsigned int y = (x >> 12) & 0xFFF;  /* Another ZERO_EXTRACT candidate */
    
    return result + y;
}

/* Function 2: Generate STRICT_LOW_PART RTL patterns (x86-specific) */
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
    
    /* Mixed-size operations */
    result = byte_val + word_val;
    
    /* Partial register update through memory */
    volatile unsigned char *ptr = (unsigned char *)&dword_val;
    *ptr = 0x99;  /* Modify low byte */
    
#else
    /* Generic fallback - use bit-field operations */
    volatile unsigned int reg = 0x87654321;
    unsigned int temp = reg;
    temp &= ~0xFF;        /* Clear low byte */
    temp |= 0x42;         /* Set low byte - may generate partial reg patterns */
    reg = temp;
    result = reg;
#endif
    
    return result;
}

/* Function 3: Generate SUBREG RTL patterns */
NOINLINE static unsigned int test_subreg(void) {
    unsigned int result = 0;
    
    /* Type conversions that generate SUBREG */
    long long big_val = 0x1122334455667788LL;
    int int_val = (int)big_val;          /* Truncation */
    short short_val = (short)int_val;    /* More truncation */
    
    /* Access different parts of larger types */
    union {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } parts;
    } converter;
    
    converter.full = 0xAABBCCDDEEFF1122ULL;
    result = converter.parts.low + converter.parts.high;
    
    /* Mixed-size arithmetic */
    int a = 1000;
    short b = 100;
    int c = a + b;  /* b promoted, but may involve SUBREG in RTL */
    
    /* Pointer casting for sub-parts */
    uint32_t *ptr32 = (uint32_t *)&big_val;
    result += ptr32[0] + ptr32[1];
    
    return result + c + short_val;
}

/* Function 4: Generate complex MEM_P RTL patterns */
NOINLINE static unsigned int test_mem_operands(void) {
    unsigned int result = 0;
    volatile int i, j, k;
    
    /* Force variables to be non-constant */
    asm volatile ("" : "=r"(i) : "0"(1));
    asm volatile ("" : "=r"(j) : "0"(2));
    asm volatile ("" : "=r"(k) : "0"(3));
    
    /* Multi-dimensional array with variable indices */
    int arr[10][10];
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            arr[x][y] = x * y;
        }
    }
    
    /* Complex addressing modes */
    result += arr[i][j];              /* Base + index */
    result += arr[j][k];              /* Different indices */
    result += *(arr[i] + j);          /* Pointer arithmetic */
    
    /* Structure with multiple fields */
    struct {
        int a[5];
        int b[5];
        int c[5];
    } s;
    
    for (int idx = 0; idx < 5; idx++) {
        s.a[idx] = idx * 2;
        s.b[idx] = idx * 3;
        s.c[idx] = idx * 4;
    }
    
    /* Structure field access with computation */
    result += s.a[i] + s.b[j] + s.c[k];
    
    /* Pointer chasing with offset */
    int *ptr = s.a;
    result += ptr[i + j];             /* Complex memory address */
    result += *(ptr + i * j);         /* More complex addressing */
    
    return result;
}

/* Function 5: Combined patterns in loop for scheduling */
NOINLINE static unsigned int test_combined(void) {
    unsigned int sum = 0;
    
    /* Loop with conditional to prevent optimization */
    for (volatile int counter = 0; counter < 10; counter++) {
        /* Mix different patterns in loop body */
        if (counter & 1) {
            sum += test_zero_extract();
        } else {
            sum += test_subreg();
        }
        
        /* Memory operations in loop */
        volatile int temp[4] = {1, 2, 3, 4};
        sum += temp[counter & 3];
        
        /* Force register pressure */
        asm volatile ("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Main function that exercises all patterns */
int main(void) {
    unsigned int total = 0;
    
    /* Static assert to ensure optimization */
    _Static_assert(__OPTIMIZE__, "Compile with optimization enabled");
    
    /* Call pattern-specific functions */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_mem_operands();
    total += test_combined();
    
    /* Use result to prevent dead code elimination */
    volatile unsigned int *output = &total;
    asm volatile ("" : : "r"(output) : "memory");
    
    /* Return non-zero to indicate success */
    return (total != 0) ? 0 : 1;
}
