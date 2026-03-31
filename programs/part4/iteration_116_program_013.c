/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 *
 * Compilation recommendations:
 *   - For general RTL generation: gcc -O1 -fdump-rtl-all test_resource_coverage.c
 *   - For scheduling passes: gcc -O2 -fschedule-insns test_resource_coverage.c
 *   - For BPF target: clang -target bpf -O2 -c test_resource_coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full_word;
    struct {
        volatile uint8_t low3 : 3;
        volatile uint8_t mid4 : 4;
        volatile uint8_t high1 : 1;
    } bits;
    volatile uint16_t array[8];
};

/* Global variables to force memory operations */
static volatile int global_index = 0;
static volatile int global_mask = 0x0F;

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    volatile int *ptr_array[4];
    int local_array[16] = {0};
    
    /* Initialize pointer array with complex addressing patterns */
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &local_array[i * 3 + 1];  /* Non-linear addressing */
    }
    
    /* Main loop to generate complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG in RTL */
        bf.full_word += i * 3;
        
        /* 2. Bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.bits.low3 = (i & 0x07);        /* Direct bit-field assignment */
        bf.bits.mid4 = ((i * 2) & 0x0F);  /* Another bit-field assignment */
        
        /* 3. Mixed bitwise operation on full word */
        bf.full_word &= ~global_mask;     /* May generate ZERO_EXTRACT for partial word */
        bf.full_word |= (i << 8);         /* Bitwise OR with shift */
        
        /* 4. Complex memory addressing with array indexing */
        int idx = (i * 7 + 3) & 0x07;     /* Non-trivial index calculation */
        bf.array[idx] = bf.full_word & 0xFFFF;
        
        /* 5. Pointer arithmetic with dereference - complex MEM address */
        int *ptr = ptr_array[i & 0x03] + (i & 0x01);
        *ptr = bf.array[idx] + i;
        
        /* 6. Additional bit manipulation on memory location */
        *ptr &= 0xFFF;                    /* May generate ZERO_EXTRACT for memory */
        *ptr |= (bf.bits.low3 << 12);
        
        /* 7. Accumulate results to prevent optimization */
        accumulator += bf.full_word + *ptr + bf.array[idx];
        
        /* 8. Modify global index to create data dependencies */
        global_index = (global_index + i) & 0x3F;
    }
    
    /* Final output to ensure all operations are observable */
    printf("Result: %u\n", (unsigned int)accumulator);
    printf("Bitfield values: low3=%u, mid4=%u, high1=%u\n",
           (unsigned int)bf.bits.low3,
           (unsigned int)bf.bits.mid4,
           (unsigned int)bf.bits.high1);
    
    return 0;
}
