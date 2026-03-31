#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bit_packed {
    volatile uint32_t full;
    struct {
        uint32_t low : 8;
        uint32_t mid : 12;
        uint32_t high : 10;
        uint32_t flags : 2;
    } parts;
};

/* Global array to create complex MEM addresses */
static volatile uint32_t global_array[64];

int main(void) {
    struct bit_packed bp = {0};
    volatile uint32_t accumulator = 0;
    volatile uint32_t index_reg = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 64; i++) {
        global_array[i] = i * 3;
    }
    
    /* Loop to create data dependencies and complex RTL */
    for (volatile uint32_t i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        uint32_t temp = i * 7 + 13;
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Access through pointer to inhibit optimization */
        struct bit_packed *bp_ptr = &bp;
        bp_ptr->parts.mid = temp & 0xFFF;  /* 12-bit field */
        bp_ptr->parts.flags = (temp >> 12) & 0x3;  /* 2-bit field */
        
        /* 3. Mixed operations on the same location */
        /* This creates a data dependency chain */
        bp.full = bp.full + (temp << 3);
        
        /* 4. Complex memory addressing with pointer arithmetic */
        /* Create index with bit manipulation */
        index_reg = (i * 11) ^ (bp.parts.mid << 2);
        uint32_t idx = (index_reg & 0x3F);  /* 0-63 */
        
        /* Array access with computed index - creates complex MEM address */
        accumulator += global_array[idx];
        
        /* 5. Additional bit manipulation on memory */
        /* Modify specific bits of array element */
        global_array[idx] = (global_array[idx] & 0xFFFF0000) | 
                           ((global_array[idx] + i) & 0xFFFF);
        
        /* 6. Pointer dereference with offset calculation */
        volatile uint32_t *ptr = &global_array[0] + (idx * 2) % 64;
        accumulator ^= *ptr;
        
        /* Force memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
    
    /* 7. Final bit-field extraction and store */
    /* This may generate ZERO_EXTRACT as destination */
    volatile uint32_t result = 0;
    result = bp.parts.low | (bp.parts.mid << 8) | (bp.parts.high << 20);
    
    /* Mix in accumulator to create observable output */
    result ^= accumulator;
    
    /* Print result to prevent optimization */
    printf("Result: %u\n", (unsigned int)result);
    
    return (int)(result & 0xFF);
}
