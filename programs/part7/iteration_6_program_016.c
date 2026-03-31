/* test_resource_coverage.c
 * 
 * This program is designed to generate RTL patterns that will exercise
 * the uncovered lines in GCC's resource.cc mark_referenced_resources function.
 * Specifically, it aims to produce:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register accesses
 * - SUBREG for sub-register operations
 * - MEM_P with complex addressing modes
 */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization enabled (-O2 or -O3)"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
};

/* Function 1: Generate ZERO_EXTRACT patterns through bit-field operations */
NOINLINE static unsigned int test_zero_extract(struct bitfield *bf) {
    unsigned int result = 0;
    
    /* Multiple bit-field operations that may generate ZERO_EXTRACT */
    result = bf->field1;                     /* Simple extraction */
    result |= (bf->field2 << 4);            /* Shifted extraction */
    result |= ((bf->field3 >> 2) & 0x3F);   /* Shifted and masked extraction */
    
    /* Compound bit-field operation */
    bf->field2 = (result >> 1) & 0x7F;
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns using inline assembly (x86) */
NOINLINE static uint32_t test_strict_low_part(uint32_t val) {
    uint32_t result = 0;
    
#ifdef __x86_64__ || __i386__
    /* Byte operation that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %b1, %b0\n\t"          /* Move byte - may generate STRICT_LOW_PART */
        : "=r"(result)
        : "r"(val)
        : "cc"
    );
    
    /* Half-word operation */
    uint16_t halfword;
    asm volatile (
        "movw %w1, %w0\n\t"          /* Move word - may generate STRICT_LOW_PART */
        : "=r"(halfword)
        : "r"(val)
        : "cc"
    );
    result |= halfword;
#else
    /* Generic fallback: operations that might still generate partial register access */
    result = (uint8_t)val;           /* Truncation to byte */
    result |= (uint16_t)(val >> 8);  /* Truncation to half-word */
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns through type conversions */
NOINLINE static int test_subreg(int a, short b, char c) {
    int result = 0;
    
    /* Various type conversions that may generate SUBREG */
    short s = a;                     /* int to short - potential SUBREG */
    result = s;
    
    char ch = b;                     /* short to char - potential SUBREG */
    result += ch;
    
    int from_char = c;               /* char to int - potential SUBREG with extension */
    result += from_char;
    
    /* Access halves of 64-bit value */
    uint64_t large = 0x123456789ABCDEF0ULL;
    uint32_t low = large;            /* Low 32 bits - potential SUBREG */
    uint32_t high = (large >> 32);   /* High 32 bits */
    
    result += low & 0xFF;
    result += high & 0xFF;
    
    return result;
}

/* Function 4: Generate complex MEM_P addressing patterns */
NOINLINE static int test_mem_addressing(int *base, int index1, int index2) {
    int result = 0;
    
    /* Complex array indexing with variable offsets */
    int arr[10][10];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Multiple complex memory accesses with variable indices */
    result += arr[index1 % 10][index2 % 10];          /* 2D array access */
    result += *(base + index1);                       /* Pointer arithmetic */
    result += *(base + index2 * 2);                   /* Scaled pointer arithmetic */
    
    /* Structure with multiple fields */
    struct {
        int a;
        int b;
        int c[5];
    } s = {0};
    
    result += s.a;
    result += s.b;
    result += s.c[index1 % 5];                        /* Array field access */
    
    return result;
}

/* Function 5: Mixed operations to increase coverage probability */
NOINLINE static int test_mixed_operations(void) {
    volatile int counter = 0;
    int result = 0;
    
    /* Loop with mixed operations to keep RTL complex */
    for (int i = 0; i < 100; i++) {
        /* Bit-field like operation */
        result ^= (counter >> (i % 16)) & 0xF;        /* Potential ZERO_EXTRACT */
        
        /* Type conversion */
        short temp = result;                          /* Potential SUBREG */
        result = temp + i;
        
        /* Memory access with computation */
        int local_arr[4] = {1, 2, 3, 4};
        result += local_arr[i % 4];                   /* MEM_P with addressing */
        
        counter++;
    }
    
    return result;
}

/* Main function that calls all test functions */
int main(void) {
    int total = 0;
    
    /* Initialize test data */
    struct bitfield bf = {1, 2, 3};
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* Call all test functions multiple times */
    for (int i = 0; i < 10; i++) {
        total += test_zero_extract(&bf);
        total += test_strict_low_part(i * 37);
        total += test_subreg(i, i * 2, i * 3);
        total += test_mem_addressing(array, i, i * 7);
        total += test_mixed_operations();
    }
    
    /* Use result to prevent dead code elimination */
    return total % 256;  /* Return non-zero but bounded value */
}
