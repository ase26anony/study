/* test_resource_coverage.c
 * 
 * This program is designed to generate RTL patterns that will cause
 * GCC's mark_referenced_resources function to execute the uncovered
 * lines handling ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P.
 */

#include <stdint.h>
#include <stdio.h>

/* Ensure optimization is enabled for proper RTL generation */
#ifndef __OPTIMIZE__
#error "This test requires optimization (-O2 or -O3) to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Structure for bit-field operations */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 4;
    unsigned int padding : 16;
};

/* Union for type punning and SUBREG generation */
union type_punning {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } halves;
    uint8_t bytes[4];
};

/* Global variables to prevent constant propagation */
volatile int g_index1 = 1;
volatile int g_index2 = 2;
volatile int g_index3 = 3;

/* Function 1: Generate ZERO_EXTRACT patterns through bit-field operations */
NOINLINE static int bitfield_operations(struct bitfield_struct *bf) {
    int result = 0;
    
    /* Multiple bit-field extractions that may generate ZERO_EXTRACT */
    result |= bf->field1;           /* Direct bit-field access */
    result |= (bf->field2 >> 2) & 0x0F;  /* Shift and mask - likely ZERO_EXTRACT */
    result |= (bf->field3 << 1) & 0x1E;  /* Another potential ZERO_EXTRACT */
    
    /* Complex bit-field expression */
    unsigned int temp = bf->field2;
    result |= (temp >> g_index1) & ((1 << bf->field1) - 1);
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns using inline assembly (x86) */
NOINLINE static int assembly_operations(int a, int b) {
    int result = a;
    
#ifdef __x86_64__ || __i386__
    /* Byte operations that may generate STRICT_LOW_PART */
    unsigned char byte_val;
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(byte_val)          /* =q constraint for byte register */
        : "r"((unsigned char)b)
        : "cc"
    );
    result += byte_val;
    
    /* Half-word operation */
    unsigned short half_val;
    asm volatile (
        "movw %1, %0\n\t"
        : "=r"(half_val)          /* May generate STRICT_LOW_PART for 16-bit */
        : "r"((unsigned short)b)
        : "cc"
    );
    result += half_val;
#else
    /* Fallback for non-x86: use bit operations that might still generate
       partial register accesses through other optimizations */
    result += (b & 0xFF);
    result += (b & 0xFFFF);
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns through type conversions */
NOINLINE static int subreg_operations(union type_punning *up) {
    int result = 0;
    
    /* Various type conversions that generate SUBREG */
    uint16_t low_half = up->full;           /* Truncation: uint32_t -> uint16_t */
    result += low_half;
    
    uint32_t extended = up->halves.low;     /* Zero extension: uint16_t -> uint32_t */
    result += extended;
    
    /* Sign extension through arithmetic */
    int16_t signed_low = (int16_t)up->halves.low;
    int32_t signed_extended = signed_low;   /* Sign extension */
    result += signed_extended;
    
    /* Access different parts of the union */
    result += up->bytes[g_index1 & 3];      /* Byte access */
    result += up->halves.high;              /* High half access */
    
    return result;
}

/* Function 4: Generate complex MEM_P patterns with addressing modes */
NOINLINE static int memory_operations(int *base, int size) {
    int result = 0;
    
    /* Complex array indexing with variable offsets */
    for (int i = 0; i < size; i++) {
        /* Multiple memory accesses with different addressing modes */
        result += base[i];                          /* Simple array access */
        result += *(base + i + g_index1);           /* Pointer arithmetic */
        result += base[g_index2 * i + g_index3];    /* Non-linear indexing */
    }
    
    /* Multi-dimensional array simulation */
    int matrix[4][4] = {{0}};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = i * 4 + j;
            result += matrix[i][j];                 /* 2D array access */
        }
    }
    
    /* Structure pointer access */
    struct {
        int a;
        int b;
        int c[3];
    } s = {0};
    
    result += s.a;
    result += s.b;
    result += s.c[g_index1 % 3];
    
    return result;
}

/* Function 5: Mixed operations to increase RTL variety */
NOINLINE static int mixed_operations(void) {
    int result = 0;
    static int array[100];
    
    /* Initialize array with values */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    /* Combine different patterns */
    union type_punning up;
    up.full = 0x12345678;
    
    result += bitfield_operations((struct bitfield_struct *)&up.full);
    result += assembly_operations(array[10], array[20]);
    result += subreg_operations(&up);
    result += memory_operations(array, 50);
    
    /* Conditional that prevents dead code elimination */
    if (result > 1000) {
        result = result % 1000;
    }
    
    return result;
}

/* Main function that drives all patterns */
int main(void) {
    int total = 0;
    
    /* Initialize test data */
    struct bitfield_struct bf = {5, 100, 10, 0};
    union type_punning up;
    up.full = 0x89ABCDEF;
    
    int data_array[64];
    for (int i = 0; i < 64; i++) {
        data_array[i] = i * 2 + 1;
    }
    
    /* Call pattern functions multiple times in a loop
       to increase RTL generation opportunities */
    for (int iteration = 0; iteration < 10; iteration++) {
        g_index1 = (iteration * 3) % 7;
        g_index2 = (iteration * 5) % 11;
        g_index3 = (iteration * 7) % 13;
        
        total += bitfield_operations(&bf);
        total += assembly_operations(iteration, iteration * 2);
        total += subreg_operations(&up);
        total += memory_operations(data_array, 16 + (iteration % 8));
        total += mixed_operations();
        
        /* Modify data to prevent constant propagation */
        bf.field1 = (bf.field1 + 1) & 0xF;
        bf.field2 = (bf.field2 * 3) & 0xFF;
        up.full = up.full * 1103515245 + 12345;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total % 1000);
    
    return total % 1000;
}
