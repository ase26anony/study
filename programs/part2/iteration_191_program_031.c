/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates code patterns
   that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM
   RTL expressions. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int g_volatile_flag = 1;
volatile int g_volatile_index = 0;
volatile unsigned int g_volatile_mask = 0xFF00FF00;

/* Global arrays and structs for memory operand patterns */
int g_array[256];
struct BitFieldStruct {
    unsigned int low_bits : 8;
    unsigned int mid_bits : 16;
    unsigned int high_bits : 8;
} g_bitfield;

struct ComplexMemStruct {
    int data[4][8];
    int padding[12];
} g_cmem;

/* Union for SUBREG patterns */
union MixedSizeUnion {
    int64_t large;
    int32_t medium[2];
    int16_t small[4];
    int8_t tiny[8];
} g_union;

/* ========== ZERO_EXTRACT patterns ========== */
/* Bit-field extraction using struct member access */
unsigned int extract_bitfield_member(void) {
    /* Taking address of bit-field member may generate ZERO_EXTRACT */
    unsigned int val = g_bitfield.mid_bits;
    return val * g_volatile_flag;
}

/* Bit-field extraction using shift and mask */
unsigned int extract_shift_mask(volatile unsigned int *p) {
    /* Complex expression that might generate ZERO_EXTRACT */
    return ((*p >> g_volatile_index) & 0x3FF) * g_volatile_flag;
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Writing to only low part of a variable */
void write_low_part_16(volatile uint32_t *p) {
    /* Cast to smaller type assignment */
    *(uint16_t *)p = (uint16_t)g_volatile_flag;
}

/* Using char assignment to create low-part write */
void write_low_part_8(volatile uint32_t *p) {
    /* Write to low byte only */
    *(uint8_t *)p = (uint8_t)(g_volatile_flag & 0xFF);
}

/* ========== SUBREG patterns ========== */
/* Access parts of larger type through smaller types */
int32_t subreg_via_union(void) {
    /* Mixed size accesses through union */
    g_union.large = 0x123456789ABCDEF0LL;
    g_union.small[1] = (int16_t)g_volatile_flag;  /* SUBREG pattern */
    return g_union.medium[0] + g_union.medium[1];
}

/* Pointer casting between different sizes */
int32_t subreg_via_cast(void) {
    int64_t big_val = 0x9876543210LL;
    /* Access part of larger type */
    int32_t part = *(int32_t *)((char *)&big_val + 2);
    return part * g_volatile_flag;
}

/* ========== Complex MEM patterns ========== */
/* Memory access with complex addressing mode */
int complex_mem_access(int idx1, int idx2) {
    /* Multi-dimensional array with non-trivial index calculation */
    return g_cmem.data[idx1 % 4][(idx1 + idx2 * 3) % 8];
}

/* Memory access with pointer arithmetic and struct */
int struct_mem_access(struct ComplexMemStruct *s, int i, int j) {
    /* Complex address computation */
    return s->data[i & 3][j & 7] + s->padding[(i + j) % 12];
}

/* ========== Combined function with control flow ========== */
/* Function that combines multiple patterns in complex control flow */
unsigned int combined_patterns(int iterations) {
    volatile uint32_t low_part_var = 0;
    unsigned int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Unpredictable control flow based on volatile */
        if (g_volatile_flag & (1 << (i % 8))) {
            /* ZERO_EXTRACT pattern */
            result ^= extract_shift_mask(&g_volatile_mask);
            
            /* Complex MEM pattern */
            result += complex_mem_access(i, g_volatile_index);
        } else {
            /* STRICT_LOW_PART pattern */
            write_low_part_16(&low_part_var);
            
            /* SUBREG pattern */
            result += subreg_via_union();
        }
        
        /* Alternate between different memory access patterns */
        if (i & 1) {
            result += struct_mem_access(&g_cmem, i, g_volatile_index);
        }
        
        /* Modify volatile to change control flow */
        g_volatile_index = (g_volatile_index + 1) & 0xF;
    }
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    unsigned int final_result = 0;
    int i;
    
    /* Initialize globals */
    for (i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    g_bitfield.low_bits = 0xAB;
    g_bitfield.mid_bits = 0xCDEF;
    g_bitfield.high_bits = 0x12;
    
    for (i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            g_cmem.data[i][j] = i * 100 + j;
        }
    }
    for (i = 0; i < 12; i++) {
        g_cmem.padding[i] = i * 50;
    }
    
    /* Call pattern functions multiple times with different parameters */
    final_result += extract_bitfield_member();
    final_result += extract_shift_mask(&g_volatile_mask);
    
    write_low_part_8((volatile uint32_t *)&final_result);
    write_low_part_16((volatile uint32_t *)&final_result);
    
    final_result += subreg_via_union();
    final_result += subreg_via_cast();
    
    /* Complex memory accesses */
    for (i = 0; i < 16; i++) {
        final_result += complex_mem_access(i, i * 2);
        final_result += struct_mem_access(&g_cmem, i, i + 3);
    }
    
    /* Combined patterns with control flow */
    final_result += combined_patterns(32);
    
    /* Use result to prevent optimization */
    printf("Result: 0x%08X\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
