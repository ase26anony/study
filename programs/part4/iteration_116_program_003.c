#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full;
    struct {
        volatile uint8_t low : 3;
        volatile uint8_t mid : 5;
        volatile uint8_t high : 4;
    } bits;
    volatile uint16_t pad;
};

/* Global array to create complex MEM addresses */
static volatile int32_t global_array[32];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile int32_t base = 0x12345678;
    volatile int32_t result = 0;
    volatile int32_t *ptr;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 3;
    }
    
    /* Loop to create data dependencies and complex RTL */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic creating potential SUBREG usage */
        base = base + (i & 0xFF);
        
        /* 2. Bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.bits.low = (base >> 0) & 0x7;   /* 3 bits */
        bf.bits.mid = (base >> 3) & 0x1F;  /* 5 bits */
        bf.bits.high = (base >> 8) & 0xF;  /* 4 bits */
        
        /* 3. Mixed bitwise operations on the full field */
        bf.full = (bf.full & 0xFFFF0000) | (base & 0xFFFF);
        
        /* 4. Complex MEM addressing with pointer arithmetic */
        int idx = (base + i) % 32;
        ptr = &global_array[idx];
        
        /* 5. Additional pointer manipulation */
        ptr = (volatile int32_t *)((uintptr_t)ptr + (i & 0x3));
        
        /* 6. Memory access with complex address */
        *ptr = *ptr + bf.bits.low;
        
        /* 7. More bit manipulation on memory */
        *ptr = (*ptr & 0xFFFFFF00) | (bf.bits.mid & 0xFF);
        
        /* 8. Accumulate results to prevent optimization */
        result += base + bf.full + *ptr;
        
        /* 9. Additional volatile access to force RTL generation */
        volatile int32_t temp = global_array[(i * 7) % 32];
        result ^= temp;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", (int)result);
    printf("Bitfield: low=%d mid=%d high=%d\n", 
           (int)bf.bits.low, (int)bf.bits.mid, (int)bf.bits.high);
    
    return 0;
}
