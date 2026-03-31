/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates operations
   that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex
   MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization and create unpredictable
   control flow */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global variables for memory operations */
unsigned int g_bitfield_source = 0xDEADBEEF;
int g_array[256];
int g_result = 0;

/* Structs for bit-field and union operations */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
};

union mixed_types {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

/* 1. ZERO_EXTRACT pattern generator */
int zero_extract_pattern(volatile unsigned int *p) {
    /* Multiple bit-field extractions that may generate ZERO_EXTRACT */
    struct bitfield_struct *bfs = (struct bitfield_struct *)p;
    int result = 0;
    
    /* Direct bit-field access - may generate ZERO_EXTRACT */
    result += bfs->low8;
    
    /* Shift-and-mask pattern that may also generate ZERO_EXTRACT */
    result += (*p >> 8) & 0xFF;        /* Extract bits 8-15 */
    result += (*p >> 16) & 0xFFFF;     /* Extract bits 16-31 */
    
    /* Conditional extraction to create control flow */
    if (g_volatile_flag) {
        result += (*p >> 4) & 0xF;     /* Extract bits 4-7 */
    }
    
    return result;
}

/* 2. STRICT_LOW_PART pattern generator */
void strict_low_part_pattern(volatile unsigned int *p, unsigned char v) {
    /* Writing only to low part of a larger integer */
    *p = (*p & ~0xFF) | v;  /* Only modify low 8 bits */
    
    /* Another pattern using pointer cast */
    if (g_volatile_counter % 2) {
        int32_t *int_ptr = (int32_t *)p;
        int16_t *short_ptr = (int16_t *)int_ptr;
        *short_ptr = v * 2;  /* Write to low 16 bits only */
    }
}

/* 3. SUBREG pattern generator */
int subreg_pattern(void) {
    union mixed_types u;
    int result = 0;
    
    /* Initialize with volatile to prevent constant propagation */
    u.full = g_volatile_counter * 0x10001;
    
    /* Access parts through smaller types - may generate SUBREG */
    result += u.halves[0];      /* Access low 16 bits */
    result += u.halves[1];      /* Access high 16 bits */
    result += u.bytes[2];       /* Access third byte */
    
    /* Pointer casting between different sizes */
    long long ll_value = g_volatile_counter * 0x100000001LL;
    int *int_ptr = (int *)&ll_value;
    result += int_ptr[0];       /* Access low 32 bits */
    result += int_ptr[1];       /* Access high 32 bits */
    
    return result;
}

/* 4. Complex MEM pattern generator */
int complex_mem_pattern(int *base, int idx1, int idx2, int idx3) {
    int result = 0;
    
    /* Complex addressing modes with multiple calculations */
    result += base[idx1 + idx2 * 4];           /* Base + scaled index */
    result += base[(idx1 << 2) + idx3];        /* Base + shifted index + offset */
    
    /* Struct with array access */
    struct container {
        int data[64];
        int padding;
    } *cp = (struct container *)base;
    
    result += cp->data[idx1 & 0x3F];           /* Struct member access */
    
    /* Pointer arithmetic in loop */
    int *ptr = base;
    for (int i = 0; i < 4; i++) {
        result += *(ptr + idx1 + i * idx2);    /* Complex pointer arithmetic */
    }
    
    return result;
}

/* Main function that combines all patterns */
int main(void) {
    int checksum = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 0x01010101;
    }
    
    /* Loop to create multiple basic blocks and increase pass activity */
    for (g_volatile_counter = 0; g_volatile_counter < 100; g_volatile_counter++) {
        /* Vary the volatile flag to create unpredictable control flow */
        g_volatile_flag = (g_volatile_counter % 3) != 0;
        
        /* 1. Generate ZERO_EXTRACT patterns */
        checksum += zero_extract_pattern(&g_bitfield_source);
        
        /* 2. Generate STRICT_LOW_PART patterns */
        strict_low_part_pattern(&g_bitfield_source, 
                               (unsigned char)g_volatile_counter);
        
        /* 3. Generate SUBREG patterns */
        checksum += subreg_pattern();
        
        /* 4. Generate complex MEM patterns with varying indices */
        int idx1 = g_volatile_counter % 64;
        int idx2 = (g_volatile_counter * 7) % 32;
        int idx3 = (g_volatile_counter * 13) % 16;
        
        checksum += complex_mem_pattern(g_array, idx1, idx2, idx3);
        
        /* Mix operations to create data dependencies */
        g_bitfield_source ^= checksum;
    }
    
    /* Use result to prevent dead code elimination */
    g_result = checksum;
    
    /* Print result to ensure all code has observable effect */
    printf("Result checksum: %d\n", g_result);
    
    return g_result != 0 ? 0 : 1;
}
