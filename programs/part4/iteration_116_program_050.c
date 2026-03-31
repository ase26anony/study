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

/* Global array with volatile elements to prevent optimization */
static volatile int global_array[32];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    volatile int index = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 3;
    }
    
    /* Loop to create complex RTL patterns */
    for (int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        uint32_t temp = bf.full + i;
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Access through structure pointer to encourage complex MEM */
        struct bitfield_struct *p = &bf;
        p->parts.mid = (temp >> 4) & 0xFFF;  /* 12-bit field */
        
        /* 3. Mixed operations: arithmetic then bitwise */
        bf.full = bf.full * 13 + 7;
        
        /* 4. Bit manipulation on specific bits - may generate ZERO_EXTRACT */
        bf.parts.low = (bf.parts.low ^ i) & 0x7F;
        
        /* 5. Complex memory addressing with variable index */
        /* Create index with arithmetic to prevent simple addressing */
        index = ((i * 17) + (bf.parts.low * 3)) & 0x1F;
        
        /* Array access with complex address calculation */
        accumulator += global_array[index];
        
        /* 6. Pointer arithmetic and dereference */
        volatile int *ptr = &global_array[0];
        ptr += (bf.parts.high >> 2) & 0x7;  /* Complex address offset */
        accumulator ^= *ptr;
        
        /* 7. Additional bit-field operation on volatile */
        bf.parts.high = (bf.parts.high + accumulator) & 0xFFF;
    }
    
    /* Ensure all operations are observable */
    printf("Result: %u\n", accumulator);
    printf("Bitfield full: %u\n", bf.full);
    printf("Bitfield parts: low=%u, mid=%u, high=%u\n", 
           bf.parts.low, bf.parts.mid, bf.parts.high);
    
    return 0;
}
