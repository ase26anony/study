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
static volatile uint32_t global_array[32];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    volatile uint32_t index_reg = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Main loop creating complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Create SUBREG patterns through arithmetic */
        uint32_t temp = i * 7;
        temp += (i & 0xF) << 8;  /* Mix high and low bits */
        
        /* 2. Bit-field assignment to encourage ZERO_EXTRACT */
        bf.parts.low = temp & 0xFF;        /* Likely ZERO_EXTRACT */
        bf.parts.mid = (temp >> 8) & 0xFFF; /* Likely ZERO_EXTRACT */
        
        /* 3. Complex MEM addressing with pointer arithmetic */
        index_reg = (temp + i) % 32;
        
        /* Array access with computed index - creates complex MEM */
        uint32_t array_val = global_array[index_reg];
        
        /* 4. Mixed operations on the same memory location */
        bf.full = bf.full ^ array_val;     /* Full word operation */
        
        /* 5. Another bit-field operation after memory access */
        bf.parts.high = (bf.parts.high + 1) & 0xFFF;
        
        /* 6. Additional complex addressing with multiple indices */
        uint32_t idx2 = (index_reg + bf.parts.low) % 32;
        uint32_t idx3 = (idx2 + bf.parts.mid) % 32;
        
        /* Chain of dependent memory accesses */
        accumulator ^= global_array[idx2];
        accumulator += global_array[idx3];
        
        /* 7. Force memory barrier to prevent optimization */
        asm volatile("" : : "r"(&bf) : "memory");
    }
    
    /* Use all variables to prevent dead code elimination */
    uint32_t result = accumulator + bf.full + bf.parts.low + 
                     bf.parts.mid + bf.parts.high;
    
    printf("Result: %u\n", result);
    return 0;
}
