/* test_resource_tracking.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking pass (resource.cc lines 282-290). It creates:
 * 1. ZERO_EXTRACT patterns through bit-field operations
 * 2. STRICT_LOW_PART patterns through partial register writes
 * 3. SUBREG patterns through type punning and mixed-size accesses
 * 4. Complex MEM patterns through pointer arithmetic and struct accesses
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int g_volatile_flag = 1;
volatile int g_volatile_index = 0;
volatile unsigned int g_volatile_mask = 0xFF00FF00;

/* Global arrays and structs for memory operand patterns */
unsigned int g_global_array[256];
int g_another_array[128];

/* Struct with bit-fields for ZERO_EXTRACT */
struct BitFieldStruct {
    unsigned int low_bits : 8;
    unsigned int middle_bits : 16;
    unsigned int high_bits : 8;
    volatile unsigned int padding;
};

/* Union for SUBREG patterns */
union MixedSizeUnion {
    uint64_t full;
    uint32_t halves[2];
    uint16_t words[4];
    uint8_t bytes[8];
};

/* Struct with array for complex MEM addressing */
struct ArrayContainer {
    int data[100];
    int stride;
    volatile int offset;
};

/* ==================== ZERO_EXTRACT Patterns ==================== */

/* Method 1: Direct bit-field extraction using struct */
unsigned int extract_bitfield_method1(struct BitFieldStruct *bfs) {
    /* Taking address of bit-field member may generate ZERO_EXTRACT */
    unsigned int val = bfs->middle_bits;
    return val * g_volatile_flag;
}

/* Method 2: Manual extraction with shifts - may generate ZERO_EXTRACT */
unsigned int extract_bitfield_method2(volatile unsigned int *p) {
    /* Complex extraction pattern */
    unsigned int temp = *p;
    /* Extract bits 8-15 */
    unsigned int extracted = (temp >> 8) & 0xFF;
    /* Extract bits 16-23 with masking */
    extracted |= ((temp >> 16) & 0xFF) << 8;
    return extracted;
}

/* Method 3: Multiple extractions in loop */
void extract_in_loop(volatile unsigned int *src, unsigned int *dst, int count) {
    for (int i = 0; i < count; i++) {
        /* Various extraction patterns */
        dst[i] = (src[i] >> g_volatile_index) & 0x3F;
        if (g_volatile_flag) {
            dst[i] |= (src[i] >> 16) & 0xFF00;
        }
    }
}

/* ==================== STRICT_LOW_PART Patterns ==================== */

/* Method 1: Partial write to 32-bit variable via 16-bit pointer */
void write_low_part_16(volatile uint32_t *p, uint16_t value) {
    /* Cast to 16-bit pointer for partial write */
    *(volatile uint16_t *)p = value;
}

/* Method 2: Byte-wise write to integer */
void write_byte_to_int(volatile uint32_t *p, int index, uint8_t value) {
    /* Write single byte - may generate STRICT_LOW_PART */
    ((volatile uint8_t *)p)[index] = value;
}

/* Method 3: Mask and set low bits */
uint32_t set_low_bits(uint32_t original, uint32_t new_low) {
    /* Keep high bits, set low 8 bits */
    return (original & ~0xFF) | (new_low & 0xFF);
}

/* ==================== SUBREG Patterns ==================== */

/* Method 1: Union-based type punning */
int32_t subreg_via_union(union MixedSizeUnion *u, int index) {
    /* Access different views of same data */
    u->words[1] = 0xABCD;
    return u->halves[0] + u->bytes[3];
}

/* Method 2: Pointer casting between different sizes */
int64_t subreg_via_cast(volatile int64_t *large) {
    /* Access parts of 64-bit as 32-bit */
    int32_t low_part = *(volatile int32_t *)large;
    int32_t high_part = *((volatile int32_t *)large + 1);
    return (int64_t)low_part + ((int64_t)high_part << 32);
}

/* Method 3: Mixed-size operations */
int mixed_size_ops(int32_t a, int16_t b, int8_t c) {
    /* Operations with different sized operands */
    int32_t temp = a + b;  /* b promoted, but SUBREG may appear in RTL */
    temp *= c;
    return temp;
}

/* ==================== Complex MEM Patterns ==================== */

/* Method 1: Array access with complex index calculation */
int complex_array_access(int *base, int idx1, int idx2, int idx3) {
    /* Multi-dimensional addressing */
    return base[idx1 * 16 + idx2 * 8 + idx3];
}

/* Method 2: Struct with computed offset */
int struct_mem_access(struct ArrayContainer *container, int multiplier) {
    /* Complex addressing with struct fields */
    int effective_offset = container->offset * multiplier;
    int stride_offset = container->stride * g_volatile_index;
    return container->data[effective_offset + stride_offset];
}

/* Method 3: Pointer arithmetic with multiple bases */
int multi_base_access(int *base1, int *base2, volatile int *selector) {
    /* Conditional memory access with different bases */
    int *selected_base = (*selector & 1) ? base1 : base2;
    int offset = (*selector >> 1) & 0x3F;
    return selected_base[offset * 2];
}

/* ==================== Combined Function ==================== */

/* Function that combines multiple patterns in control flow */
unsigned int combined_patterns(int iteration) {
    unsigned int result = 0;
    static union MixedSizeUnion u = {0x123456789ABCDEF0ULL};
    static struct BitFieldStruct bfs = {0, 0, 0, 0};
    static struct ArrayContainer container = {{0}, 4, 0};
    
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        container.data[i] = i * iteration;
    }
    container.offset = iteration % 10;
    
    /* ZERO_EXTRACT patterns */
    if (g_volatile_flag & 1) {
        bfs.middle_bits = iteration & 0xFFFF;
        result ^= extract_bitfield_method1(&bfs);
    }
    
    /* STRICT_LOW_PART patterns */
    if (g_volatile_flag & 2) {
        uint32_t temp = result;
        write_byte_to_int(&temp, 1, iteration & 0xFF);
        result = set_low_bits(result, temp);
    }
    
    /* SUBREG patterns */
    if (g_volatile_flag & 4) {
        result += subreg_via_union(&u, iteration & 3);
    }
    
    /* Complex MEM patterns */
    if (g_volatile_flag & 8) {
        int idx = g_volatile_index & 31;
        result ^= complex_array_access(g_global_array, 
                                      idx, 
                                      idx * 2, 
                                      iteration & 7);
        result += struct_mem_access(&container, iteration % 5);
    }
    
    /* Mixed operation to use all results */
    result = (result >> 16) | (result << 16);  /* byte swap */
    
    return result;
}

/* ==================== Main Function ==================== */

int main(void) {
    unsigned int final_result = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        g_global_array[i] = i * 3 + 1;
    }
    for (int i = 0; i < 128; i++) {
        g_another_array[i] = i * 5 - 2;
    }
    
    /* Main loop with multiple iterations to increase pass activity */
    for (int i = 0; i < 100; i++) {
        /* Update volatile variables to affect control flow */
        g_volatile_flag = (g_volatile_flag * 1103515245 + 12345) & 0x7FFFFFFF;
        g_volatile_index = i & 63;
        g_volatile_mask = (g_volatile_mask << 1) | (g_volatile_mask >> 31);
        
        /* Call combined function */
        unsigned int iter_result = combined_patterns(i);
        
        /* Also call individual pattern functions occasionally */
        if (i % 7 == 0) {
            union MixedSizeUnion u_local = {0xFEDCBA9876543210ULL};
            iter_result ^= subreg_via_union(&u_local, i & 3);
        }
        
        if (i % 11 == 0) {
            struct BitFieldStruct bfs_local = {0, 0, 0, 0};
            bfs_local.middle_bits = i & 0xFFFF;
            iter_result += extract_bitfield_method1(&bfs_local);
        }
        
        /* Complex memory access */
        if (i % 13 == 0) {
            iter_result ^= multi_base_access(g_global_array, 
                                           g_another_array, 
                                           &g_volatile_flag);
        }
        
        /* Accumulate final result */
        final_result = (final_result * 31 + iter_result) & 0xFFFFFFFF;
    }
    
    /* Use result to prevent optimization */
    printf("Final checksum: 0x%08X\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
