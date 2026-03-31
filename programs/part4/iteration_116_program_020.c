#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full;
    struct {
        uint32_t low : 8;
        uint32_t mid : 12;
        uint32_t high : 10;
        uint32_t flags : 2;
    } parts;
};

/* Global array to create complex MEM addresses */
static volatile uint32_t global_array[32];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    volatile uint32_t index_reg = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 3;
    }
    
    /* Loop to create data dependencies and complex RTL */
    for (volatile uint32_t i = 0; i < 100; i++) {
        /* 1. Create SUBREG opportunities through arithmetic */
        uint32_t temp = i + (i << 3);
        bf.full = temp;  /* May generate SUBREG in RTL */
        
        /* 2. Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
        bf.parts.low = (temp & 0xFF) ^ 0x55;      /* Likely ZERO_EXTRACT */
        bf.parts.mid = (temp >> 8) & 0xFFF;       /* Another ZERO_EXTRACT */
        bf.parts.flags = (i & 0x3);               /* Small bit-field */
        
        /* 3. Complex MEM addressing with pointer arithmetic */
        index_reg = (i * 7 + bf.parts.low) & 0x1F;
        
        /* Array access with computed index - creates complex MEM address */
        uint32_t* volatile ptr = (uint32_t*)&global_array[0];
        ptr += index_reg;  /* Pointer arithmetic */
        
        /* 4. Mixed operation: modify memory through pointer */
        *ptr ^= bf.parts.mid;  /* Complex MEM destination */
        
        /* 5. Additional bit manipulation on memory */
        *ptr &= ~(0xFF << 8);   /* Clear bits 8-15 */
        *ptr |= (bf.parts.low << 8);  /* Set bits with ZERO_EXTRACT source */
        
        /* 6. Accumulate to prevent optimization */
        accumulator += *ptr + bf.full + bf.parts.flags;
        
        /* 7. Create cross-iteration dependencies */
        bf.parts.high = (accumulator >> 10) & 0x3FF;
    }
    
    /* Final computation to use all values */
    uint32_t result = accumulator + bf.full;
    
    printf("Result: %u\n", (unsigned int)result);
    
    /* Additional volatile store to force all operations */
    volatile uint32_t sink = result;
    (void)sink;
    
    return 0;
}
