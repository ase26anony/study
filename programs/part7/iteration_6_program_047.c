/* Test program to cover GCC resource.cc lines 282-290 */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fschedule-insns -fprofile-arcs -ftest-coverage */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization (-O1, -O2, or -O3) to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Function 1: Generate ZERO_EXTRACT RTL pattern */
/* Bit-field extraction on volatile to prevent optimization */
NOINLINE static int bitfield_extract(void) {
    volatile unsigned int source = 0xABCD1234;
    /* Multiple bit-field operations to increase chances */
    unsigned int a = (source >> 4) & 0xFFF;      /* 12-bit extract */
    unsigned int b = (source >> 16) & 0xFFFF;    /* 16-bit extract */
    unsigned int c = (source >> 8) & 0x3FF;      /* 10-bit extract */
    return (int)(a + b + c);
}

/* Function 2: Generate STRICT_LOW_PART RTL pattern */
/* Use inline assembly for partial register access (x86/x86-64 specific) */
NOINLINE static int partial_register_access(void) {
    int result = 0;
    
    #if defined(__i386__) || defined(__x86_64__)
    /* Byte operations that may generate STRICT_LOW_PART */
    unsigned char byte_val = 0x42;
    unsigned int dword_val = 0;
    
    /* Multiple inline asm statements with byte constraints */
    asm volatile (
        "movb %1, %b0\n\t"           /* Move byte to low part of register */
        : "=r"(dword_val)
        : "r"(byte_val)
        : "cc"
    );
    
    /* Another with explicit low-byte constraint */
    unsigned short word_val = 0;
    asm volatile (
        "movw %1, %w0\n\t"           /* Move word to low part of register */
        : "=r"(word_val)
        : "r"(0x1234)
        : "cc"
    );
    
    result = dword_val + word_val;
    #else
    /* Fallback for non-x86: use bit-field structure */
    struct {
        unsigned int low : 8;
        unsigned int high : 24;
    } bits = {0x42, 0x123456};
    result = bits.low + (bits.high & 0xFF);
    #endif
    
    return result;
}

/* Function 3: Generate SUBREG RTL patterns */
/* Type conversions and sub-register accesses */
NOINLINE static int subreg_conversions(void) {
    /* Mixed-size operations */
    long long big = 0x123456789ABCDEF0LL;
    int medium = (int)big;           /* Truncation: may generate SUBREG */
    short small = (short)medium;     /* Another truncation */
    char tiny = (char)small;         /* Another truncation */
    
    /* Access different parts of larger types */
    union {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } halves;
    } converter;
    
    converter.full = big;
    /* Accessing halves may generate SUBREG */
    uint32_t low_part = converter.halves.low;
    uint32_t high_part = converter.halves.high;
    
    /* Pointer casting for sub-region access */
    uint16_t *half_ptr = (uint16_t*)&medium;
    uint16_t first_half = half_ptr[0];
    uint16_t second_half = half_ptr[1];
    
    return (int)(tiny + small + medium + low_part + high_part + first_half + second_half);
}

/* Function 4: Generate complex MEM_P RTL patterns */
/* Memory accesses with complex addressing modes */
NOINLINE static int complex_memory_access(int index1, int index2) {
    /* Multi-dimensional array with variable indices */
    int matrix[10][20];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Complex addressing: matrix[index1][index2] + matrix[index2][index1] */
    int val1 = matrix[index1][index2];
    int val2 = matrix[index2 % 10][index1 % 20];
    
    /* Pointer arithmetic with non-constant offsets */
    int *ptr = &matrix[0][0];
    int val3 = ptr[index1 * 20 + index2];
    int val4 = *(ptr + (index2 * 10 + index1));
    
    /* Structure with multiple fields */
    struct {
        int a;
        int b;
        int c;
        int d;
    } s = {1, 2, 3, 4};
    
    /* Access structure through pointer with offset */
    int *field_ptr = &s.a;
    int val5 = field_ptr[index1 % 4];
    
    return val1 + val2 + val3 + val4 + val5;
}

/* Main function that calls all pattern generators */
int main(void) {
    int total = 0;
    
    /* Loop to increase chance of RTL processing */
    for (int i = 0; i < 10; i++) {
        total += bitfield_extract();
        total += partial_register_access();
        total += subreg_conversions();
        total += complex_memory_access(i, i * 2);
    }
    
    /* Use result to prevent dead code elimination */
    volatile int sink = total;
    
    /* Simple validation */
    if (total != 0) {
        return 0;  /* Success */
    }
    
    return 1;  /* Should never reach here */
}
