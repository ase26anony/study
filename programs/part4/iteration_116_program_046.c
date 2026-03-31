#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full;
    struct {
        uint32_t low : 8;
        uint32_t mid : 12;
        uint32_t high : 12;
    } parts;
};

/* Global array to create complex MEM addresses */
static volatile int global_array[32];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    volatile int index_reg = 0;
    
    /* Initialize with non-zero values */
    bf.full = 0x12345678;
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 3;
    }
    
    /*
     * Loop designed to:
     * 1. Create arithmetic operations that may use SUBREG
     * 2. Generate bit-field assignments (ZERO_EXTRACT/STRICT_LOW_PART)
     * 3. Create complex MEM addresses through pointer arithmetic
     */
    for (int i = 0; i < 100; i++) {
        /* Arithmetic operation that may create SUBREG in RTL */
        uint32_t temp = bf.full + i;
        
        /* Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.parts.mid = (temp >> 4) & 0xFFF;  /* 12-bit field */
        
        /* Mixed bitwise and arithmetic - encourages complex RTL */
        bf.full = (bf.full & 0xFFFF0000) | (temp & 0xFFFF);
        
        /* Create complex addressing for MEM_P path */
        index_reg = (i * 7 + 3) & 0x1F;  /* Keep within array bounds */
        
        /* Array access with computed index - creates [reg + offset] addressing */
        accumulator += global_array[index_reg];
        
        /* Pointer arithmetic followed by dereference */
        volatile int *ptr = &global_array[0];
        ptr += (index_reg * 2) / 3;
        accumulator += *ptr;
        
        /* Additional bit manipulation on the accumulator */
        accumulator = (accumulator & 0xFFFFFF00) | ((accumulator + i) & 0xFF);
    }
    
    /* Force all operations to be observable */
    printf("Result: %u\n", (unsigned int)accumulator);
    printf("Bitfield full: 0x%08x\n", (unsigned int)bf.full);
    printf("Bitfield mid: 0x%03x\n", (unsigned int)bf.parts.mid);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
