/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   - For general RTL expansion: gcc -O1 -fdump-rtl-all test_resource_coverage.c
 *   - For scheduling passes: gcc -O2 -fschedule-insns test_resource_coverage.c
 *   - For BPF target: gcc -target bpf -O2 -fdump-rtl-all test_resource_coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full;
    struct {
        volatile uint8_t low3 : 3;
        volatile uint8_t mid4 : 4;
        volatile uint8_t high1 : 1;
    } bits;
    volatile uint16_t tail;
};

/* Global array to create complex MEM addresses */
static volatile int global_array[32];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    volatile int index_reg;
    
    /* Initialize with non-zero values */
    bf.full = 0xDEADBEEF;
    bf.bits.low3 = 0x5;
    bf.bits.mid4 = 0xA;
    bf.bits.high1 = 0x1;
    bf.tail = 0xCAFE;
    
    for (int i = 0; i < 100; i++) {
        /* 1. Create SUBREG potential through arithmetic on mixed-size types */
        uint32_t temp = bf.full;
        temp += i * 0x10001;          /* Creates 32-bit operations */
        uint16_t temp16 = temp >> 16; /* May generate SUBREG in RTL */
        
        /* 2. Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
        /* Direct bit-field assignment - common source of ZERO_EXTRACT */
        bf.bits.mid4 = (temp16 & 0x0F); /* Assign to specific bits */
        
        /* Manual bit masking that may also generate ZERO_EXTRACT */
        bf.full = (bf.full & ~0x1F) | (temp & 0x1F);
        
        /* 3. Complex memory addressing for MEM_P(x) path */
        /* Create data-dependent array index with arithmetic */
        index_reg = (temp16 ^ i) & 0x1F; /* 0-31 range */
        
        /* Array access with complex addressing: [global + variable_index] */
        global_array[index_reg] += temp & 0xFF;
        
        /* Pointer arithmetic for additional MEM complexity */
        volatile int *ptr = &global_array[16];
        ptr[(i & 0x3) - 2] ^= temp16; /* May create [reg + reg*scale + offset] */
        
        /* 4. Mixed operations to prevent optimization */
        accumulator ^= bf.full;
        accumulator += global_array[index_reg];
        accumulator = (accumulator << 3) | (accumulator >> 29); /* Rotate */
        
        /* 5. Additional bit manipulation on memory */
        /* This may generate ZERO_EXTRACT when targeting bit-fields in memory */
        bf.tail &= ~(0x7 << 4);       /* Clear bits 4-6 */
        bf.tail |= ((accumulator & 0x7) << 4); /* Set them from accumulator */
    }
    
    /* Final computation to ensure all operations are observable */
    volatile uint32_t result = accumulator;
    for (int i = 0; i < 32; i++) {
        result ^= global_array[i];
    }
    result ^= bf.full;
    result ^= bf.tail;
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%08X\n", (unsigned int)result);
    
    return 0;
}
