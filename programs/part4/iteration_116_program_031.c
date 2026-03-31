/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 *
 * Compilation recommendations:
 *   - For RTL expansion analysis: gcc -O1 -fdump-rtl-all -fdump-rtl-expand test_resource_coverage.c
 *   - For scheduling/resource tracking: gcc -O2 -fschedule-insns -freschedule-modulo-scheduled-loops test_resource_coverage.c
 *   - For BPF target (specific ZERO_EXTRACT patterns): gcc -target bpf -O2 -fdump-rtl-all test_resource_coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full_word;
    struct {
        volatile uint16_t low_bits : 4;
        volatile uint16_t mid_bits : 8;
        volatile uint16_t high_bits : 4;
    } parts;
};

/* Global variables to force memory operations */
static volatile int global_index = 0;
static volatile int global_mask = 0x0F;

int main(void) {
    /* Local variables with volatile to prevent optimization */
    volatile uint32_t base_value = 0x12345678;
    volatile uint32_t accumulator = 0;
    volatile int array_index = 0;
    
    /* Array with volatile elements to force MEM operations */
    volatile uint32_t data_array[8] = {0};
    
    /* Packed structure with bit-fields */
    struct bitfield_struct bf = {0};
    bf.full_word = 0xDEADBEEF;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 8; i++) {
        data_array[i] = i * 0x11111111;
    }
    
    /* Main loop creating complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG in RTL */
        base_value += i * 0x10001;  /* Creates potential for SUBREG operations */
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT or STRICT_LOW_PART */
        /* Modify specific bits of the structure */
        bf.parts.low_bits = (i & 0x0F);          /* 4-bit field */
        bf.parts.mid_bits = ((i * 3) & 0xFF);    /* 8-bit field */
        bf.parts.high_bits = ((i >> 4) & 0x0F);  /* 4-bit field */
        
        /* 3. Bitwise operation on volatile variable - may generate ZERO_EXTRACT */
        base_value &= ~global_mask;              /* Clear specific bits */
        base_value |= (i & global_mask);         /* Set bits from loop counter */
        
        /* 4. Complex memory addressing - forces MEM_P(x) with non-trivial address */
        /* Compute index with arithmetic to create complex addressing mode */
        array_index = (i * 3 + 1) & 0x07;        /* 0-7 range */
        
        /* Array access with computed index - creates MEM with complex address */
        accumulator += data_array[array_index];
        
        /* 5. Pointer arithmetic and dereference - alternative complex MEM */
        volatile uint32_t *ptr = (volatile uint32_t *)&data_array[0];
        ptr += (i & 0x03);                       /* Pointer arithmetic */
        accumulator ^= *ptr;                     /* Dereference */
        
        /* 6. Mixed operations on the bit-field structure */
        bf.full_word = (bf.full_word << 1) | (accumulator & 1);
        
        /* 7. Additional bit manipulation that may use ZERO_EXTRACT */
        /* Extract and modify specific bit ranges */
        uint32_t temp = bf.full_word;
        temp = (temp & 0xFFFF0000) | ((temp & 0x0000FFFF) ^ accumulator);
        bf.full_word = temp;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: base_value = 0x%08X, accumulator = 0x%08X\n", 
           (unsigned int)base_value, (unsigned int)accumulator);
    printf("Bitfield: full_word = 0x%08X, low_bits = %u\n",
           (unsigned int)bf.full_word, (unsigned int)bf.parts.low_bits);
    
    /* Simple checksum for verification */
    volatile uint32_t checksum = base_value + accumulator + bf.full_word;
    printf("Checksum: 0x%08X\n", (unsigned int)checksum);
    
    return (checksum != 0) ? 0 : 1;
}
