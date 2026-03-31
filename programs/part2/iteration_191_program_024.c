/* test_resource_tracking.c
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking pass (resource.cc lines 282-290).
 * It creates ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM
 * RTL expressions through various C constructs.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global arrays/structs for memory operand patterns */
int g_array[256];
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
} g_bitfield;

union MixedSizeUnion {
    int64_t full;
    int32_t half[2];
    int16_t quarter[4];
    int8_t eighth[8];
} g_union;

/* ==================== ZERO_EXTRACT Patterns ==================== */

/* Pattern 1: Bit-field extraction from a struct member */
unsigned int extract_bitfield_low(void) {
    /* Taking address of bit-field member may generate ZERO_EXTRACT */
    struct BitFieldStruct local = g_bitfield;
    unsigned int val = local.low8;  /* Potential ZERO_EXTRACT */
    return val + g_volatile_counter;
}

/* Pattern 2: Manual bit extraction with volatile pointer */
unsigned int extract_bits_manual(volatile unsigned int *ptr) {
    /* Complex shift/mask that might not be optimized away */
    unsigned int temp = *ptr;
    /* This pattern often creates ZERO_EXTRACT in RTL */
    return (temp >> (g_volatile_flag & 0x7)) & ((1 << 8) - 1);
}

/* Pattern 3: Multiple extractions in a loop */
unsigned int extract_multiple_bits(void) {
    volatile unsigned int source = 0xDEADBEEF;
    unsigned int result = 0;
    
    for (int i = 0; i < 4; i++) {
        /* Each iteration extracts different bits */
        result ^= (source >> (i * 8)) & 0xFF;
    }
    return result;
}

/* ==================== STRICT_LOW_PART Patterns ==================== */

/* Pattern 1: Writing to only low part of a larger variable */
void set_low_part_32(volatile uint32_t *dest, uint16_t value) {
    /* This write to 16-bit part of 32-bit location may create STRICT_LOW_PART */
    *(uint16_t *)dest = value;  /* Potential STRICT_LOW_PART */
}

/* Pattern 2: Using char assignment to part of int */
void set_low_byte(volatile unsigned int *p, unsigned char v) {
    /* Clear low byte, then set it - may generate STRICT_LOW_PART */
    *p = (*p & ~0xFF) | v;
}

/* Pattern 3: Inline assembly that hints at partial register write */
void partial_reg_write(void) {
    uint32_t x = 0x12345678;
    /* Cast to smaller type assignment */
    *(uint16_t *)(&x) = 0xABCD;  /* Potential STRICT_LOW_PART */
    g_array[0] = x;
}

/* ==================== SUBREG Patterns ==================== */

/* Pattern 1: Union-based type punning */
int32_t subreg_via_union(void) {
    union MixedSizeUnion u;
    u.full = 0x1122334455667788ULL;
    
    /* Accessing parts through different union members */
    int32_t val1 = u.half[0];  /* SUBREG from 64-bit to 32-bit */
    int16_t val2 = u.quarter[2]; /* Another SUBREG */
    
    return val1 + val2;
}

/* Pattern 2: Pointer casting between different sizes */
int32_t subreg_via_cast(int64_t big_val) {
    /* Cast between pointer types of different sizes */
    int32_t *ptr = (int32_t *)&big_val;
    return ptr[0] + ptr[1];  /* Multiple SUBREG accesses */
}

/* Pattern 3: Mixed-size operations in expressions */
int64_t mixed_size_ops(int32_t a, int16_t b, int8_t c) {
    /* Implicit promotions and conversions create SUBREG patterns */
    int64_t result = a;          /* Sign extend: SUBREG */
    result += b;                 /* Another SUBREG */
    result *= c;                 /* And another */
    return result;
}

/* ==================== Complex MEM Patterns ==================== */

/* Pattern 1: Array access with complex index calculation */
int mem_complex_index(int *base, int idx1, int idx2, int idx3) {
    /* Multi-component address calculation */
    return base[(idx1 * idx2 + idx3) & 0xFF];  /* Complex MEM address */
}

/* Pattern 2: Struct with array member and pointer arithmetic */
struct Container {
    int data[100];
    int metadata;
};

int mem_struct_array(struct Container *c, int offset) {
    /* Address calculation through struct member */
    return c->data[offset * 2 + g_volatile_flag];  /* Complex MEM */
}

/* Pattern 3: Multiple memory accesses with different addressing modes */
int mem_multiple_patterns(void) {
    int sum = 0;
    int *ptr = g_array;
    
    /* Various addressing modes */
    sum += ptr[g_volatile_counter];              /* Indexed */
    sum += *(ptr + (g_volatile_flag * 16));      /* Scaled offset */
    sum += ptr[g_array[0] & 0xF];                /* Indirect index */
    
    return sum;
}

/* ==================== Combined Function ==================== */

/* Function that combines multiple patterns in control flow */
unsigned int combined_patterns(int selector) {
    unsigned int result = 0;
    volatile unsigned int mem_buffer[4] = {0};
    
    /* Use volatile flag to create unpredictable control flow */
    if (g_volatile_flag & 0x1) {
        /* ZERO_EXTRACT pattern */
        result ^= extract_bits_manual(&mem_buffer[0]);
    }
    
    if (g_volatile_flag & 0x2) {
        /* STRICT_LOW_PART pattern */
        set_low_part_32(&mem_buffer[1], 0x1234);
    }
    
    if (g_volatile_flag & 0x4) {
        /* SUBREG pattern */
        result += subreg_via_union();
    }
    
    if (g_volatile_flag & 0x8) {
        /* Complex MEM pattern */
        result += mem_complex_index(g_array, 
                                   selector, 
                                   g_volatile_counter,
                                   g_volatile_flag);
    }
    
    return result;
}

/* ==================== Main Driver ==================== */

int main(void) {
    unsigned int final_result = 0;
    
    /* Initialize globals */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    g_bitfield.low8 = 0xAB;
    g_bitfield.mid8 = 0xCD;
    g_bitfield.high16 = 0xEF01;
    
    g_union.full = 0x8877665544332211ULL;
    
    /* Loop to increase pass activity and create more RTL contexts */
    for (int iteration = 0; iteration < 100; iteration++) {
        g_volatile_counter = iteration;
        g_volatile_flag = (iteration * 37) & 0xF;  /* Pseudo-random */
        
        /* Call pattern functions in varying sequences */
        switch (iteration % 5) {
            case 0:
                final_result += extract_bitfield_low();
                break;
            case 1:
                final_result += extract_multiple_bits();
                break;
            case 2:
                partial_reg_write();
                final_result += g_array[0];
                break;
            case 3:
                final_result += mixed_size_ops(iteration, 
                                              iteration * 2, 
                                              iteration * 3);
                break;
            case 4:
                final_result += mem_multiple_patterns();
                break;
        }
        
        /* Always call the combined function */
        final_result ^= combined_patterns(iteration);
        
        /* Force memory side effects */
        g_array[iteration & 0xFF] = final_result;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: 0x%08X\n", final_result);
    
    return (final_result & 0xFF) == 0 ? 0 : 1;
}
