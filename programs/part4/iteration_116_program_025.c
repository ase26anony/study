#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bit_packed {
    volatile uint32_t full;
    struct {
        uint32_t low : 8;
        uint32_t mid : 12;
        uint32_t high : 12;
    } parts;
};

/* Global array with volatile elements to prevent optimization */
static volatile int mem_array[16];

int main(void) {
    struct bit_packed bp = {0};
    volatile uint32_t accumulator = 0;
    volatile int index = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 16; i++) {
        mem_array[i] = i * 3 + 1;
    }
    
    /* Loop to create complex RTL patterns */
    for (int i = 0; i < 100; i++) {
        /* 1. Arithmetic on base integer (potential SUBREG in RTL) */
        bp.full = bp.full + i * 7;
        
        /* 2. Bit-field assignments (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        bp.parts.low = (bp.parts.low ^ i) & 0xFF;      /* May generate ZERO_EXTRACT */
        bp.parts.mid = (bp.parts.mid + i) & 0xFFF;     /* Bit-field assignment */
        bp.parts.high = bp.parts.high | (i << 4);      /* Another bit-field */
        
        /* 3. Complex memory addressing with pointer arithmetic */
        index = (i * 13 + bp.parts.low) % 16;
        
        /* Create data dependency chain */
        int temp = mem_array[index];
        temp = temp + bp.parts.mid;
        temp = temp * 3 - bp.parts.high;
        
        /* 4. Mixed operations on the same memory location */
        mem_array[index] = temp & 0xFFFF;              /* May generate ZERO_EXTRACT */
        
        /* 5. Additional complex MEM address with multiple components */
        int idx2 = (index + bp.parts.low + 5) % 16;
        mem_array[idx2] = mem_array[idx2] ^ (temp >> 8);
        
        /* Accumulate to prevent optimization */
        accumulator += bp.full + mem_array[index] + mem_array[idx2];
        
        /* 6. Pointer dereference with offset calculation */
        volatile int *ptr = &mem_array[0];
        ptr += (i & 3) * 2;
        *ptr = *ptr + accumulator;
        
        /* 7. Another bit-field operation on volatile */
        bp.parts.low = (bp.parts.low * 3) & 0x7F;
    }
    
    /* Final computation to use all values */
    uint32_t result = accumulator;
    for (int i = 0; i < 16; i++) {
        result += mem_array[i];
    }
    result += bp.full;
    
    printf("Result: %u\n", (unsigned int)result);
    return 0;
}
