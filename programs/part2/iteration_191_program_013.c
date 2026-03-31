/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates code patterns
   that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex
   MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable
   control flow */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_counter = 0;

/* Global variables for memory access patterns */
volatile unsigned int global_bitfield = 0xDEADBEEF;
int global_array[256];
long long global_ll = 0x123456789ABCDEF0LL;

/* Struct with bitfields to generate ZERO_EXTRACT */
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
};

/* Union for SUBREG patterns */
union MixedSizeUnion {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

/* Struct for complex memory addressing */
struct ComplexMemStruct {
    int data[64];
    int padding[16];
    int more_data[32];
};

/* 1. Generate ZERO_EXTRACT patterns through bitfield operations */
int extract_bitfield_patterns(void) {
    struct BitFieldStruct bfs;
    volatile unsigned int *volatile_ptr = &global_bitfield;
    
    /* Direct bitfield access - may generate ZERO_EXTRACT */
    int result = 0;
    
    /* Multiple bitfield extraction patterns */
    if (v_flag1) {
        /* Extract middle 16 bits */
        result = (global_bitfield >> 8) & 0xFFFF;
        
        /* Another extraction pattern */
        result ^= (*volatile_ptr >> 4) & 0x0F0F0F0F;
    }
    
    /* Struct bitfield access */
    bfs.low8 = 0xAB;
    bfs.mid16 = 0xCDEF;
    result += bfs.mid16;  /* This access may generate ZERO_EXTRACT */
    
    /* Complex extraction with volatile to prevent optimization */
    volatile unsigned int temp = *volatile_ptr;
    result += (temp >> 24) & 0xFF;
    result += (temp >> 16) & 0xFF;
    result += (temp >> 8) & 0xFF;
    result += temp & 0xFF;
    
    return result;
}

/* 2. Generate STRICT_LOW_PART patterns */
void strict_low_part_patterns(int *output) {
    volatile unsigned int *p = &global_bitfield;
    
    /* Write to low byte only */
    *p = (*p & ~0xFF) | 0x42;
    
    /* Write to low 16 bits */
    if (v_flag2) {
        *p = (*p & ~0xFFFF) | 0x1234;
    }
    
    /* Pointer cast to smaller type - may generate STRICT_LOW_PART */
    int32_t x = 0x87654321;
    *(int16_t*)&x = 0xABCD;  /* Write to low 16 bits */
    *output += x;
    
    /* Another pattern with volatile */
    volatile int32_t vx = 0;
    *(volatile int16_t*)&vx = 0x5555;
    *output += vx;
}

/* 3. Generate SUBREG patterns through mixed-size access */
int subreg_patterns(void) {
    union MixedSizeUnion u;
    int result = 0;
    
    /* Initialize */
    u.full = 0x12345678;
    
    /* Access through different-sized views - may generate SUBREG */
    result += u.halves[0];  /* Access low 16 bits */
    result += u.halves[1];  /* Access high 16 bits */
    
    /* Modify through smaller type */
    u.halves[1] = 0x9ABC;  /* Write to high 16 bits */
    result += u.full;
    
    /* Byte access */
    u.bytes[0] = 0xFF;
    result += u.full;
    
    /* Pointer-based subreg-like access */
    long long ll = global_ll;
    int i = *(int*)&ll;  /* Access low 32 bits of 64-bit value */
    result += i;
    
    /* Another pointer cast pattern */
    double d = 3.14159;
    int di = *(int*)&d;  /* Type punning - may generate SUBREG */
    result ^= di;
    
    return result;
}

/* 4. Generate complex MEM patterns with addressing modes */
int complex_mem_patterns(struct ComplexMemStruct *cms, int idx1, int idx2) {
    int result = 0;
    
    /* Complex array indexing with multiple terms */
    result += cms->data[idx1 + idx2 * 8];
    
    /* More complex addressing with struct member */
    result += cms->more_data[idx1 * 2 - idx2];
    
    /* Pointer arithmetic with multiple operations */
    int *ptr = &cms->data[0];
    result += ptr[idx1 * 3 + idx2];
    
    /* Nested array access */
    result += global_array[idx1 * 16 + idx2];
    
    /* Address computation with multiple components */
    int offset = idx1 * 4 + idx2 * 2;
    result += *(int*)((char*)cms->data + offset * sizeof(int));
    
    return result;
}

/* 5. Combined function with control flow to increase RTL complexity */
int combined_patterns_with_control_flow(void) {
    int result = 0;
    struct ComplexMemStruct cms;
    union MixedSizeUnion u;
    
    /* Initialize structures */
    for (int i = 0; i < 64; i++) {
        cms.data[i] = i * 3;
    }
    for (int i = 0; i < 32; i++) {
        cms.more_data[i] = i * 5;
    }
    u.full = 0;
    
    /* Loop with volatile condition to create complex control flow */
    for (volatile int i = 0; i < 10; i++) {
        v_counter++;
        
        if (v_flag1) {
            /* ZERO_EXTRACT pattern */
            result += (global_bitfield >> (i * 2)) & 0xF;
            
            /* STRICT_LOW_PART pattern */
            int32_t temp = result;
            *(int16_t*)&temp = i * 100;
            result = temp;
        }
        
        if (v_flag2 || (i % 3 == 0)) {
            /* SUBREG pattern */
            u.halves[i % 2] = i * 7;
            result += u.full;
            
            /* Complex MEM pattern */
            result += complex_mem_patterns(&cms, i, i % 4);
        }
        
        /* Alternate between patterns */
        if (i % 2 == 0) {
            result ^= extract_bitfield_patterns();
        } else {
            strict_low_part_patterns(&result);
        }
    }
    
    return result;
}

/* Main function that orchestrates all patterns */
int main(void) {
    int final_result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 11;
    }
    
    printf("Starting RTL pattern generation test...\n");
    
    /* Execute each pattern generator multiple times with different
       volatile conditions to ensure the compiler generates various
       RTL expressions */
    
    v_flag1 = 1;
    v_flag2 = 0;
    final_result += extract_bitfield_patterns();
    
    v_flag1 = 0;
    v_flag2 = 1;
    strict_low_part_patterns(&final_result);
    
    v_flag1 = 1;
    v_flag2 = 1;
    final_result += subreg_patterns();
    
    /* Create a complex memory structure */
    struct ComplexMemStruct cms;
    for (int i = 0; i < 64; i++) cms.data[i] = i * 7;
    for (int i = 0; i < 32; i++) cms.more_data[i] = i * 13;
    
    v_flag1 = (v_counter % 2);
    v_flag2 = (v_counter % 3 == 0);
    final_result += complex_mem_patterns(&cms, 5, 7);
    
    /* Finally, run the combined function with complex control flow */
    final_result += combined_patterns_with_control_flow();
    
    /* Use the result to prevent dead code elimination */
    printf("Final checksum: %d\n", final_result);
    
    /* Return the result to make the program's output observable */
    return final_result % 256;
}
