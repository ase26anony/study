#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) packed_data {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 12;
    unsigned int field4 : 8;
};

/* Volatile variables to prevent optimization */
static volatile struct packed_data global_packed;
static volatile uint32_t global_array[16];

int main(void) {
    volatile uint32_t accumulator = 0;
    volatile uint32_t base_value = 0x12345678;
    volatile uint32_t *volatile ptr_array = (uint32_t *)global_array;
    
    /* Local packed structure on stack */
    struct packed_data local_packed;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 16; i++) {
        global_array[i] = i * 0x11111111;
    }
    
    /* Main loop with complex operations */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        uint32_t temp = base_value + i;
        
        /* 2. Bit-field assignment - potential for ZERO_EXTRACT/STRICT_LOW_PART */
        /* Modify specific bits of the value */
        temp &= ~0xF;          /* Clear lower 4 bits */
        temp |= (i & 0xF);     /* Set lower 4 bits from loop counter */
        
        /* 3. Access packed structure field - encourages bit-field RTL */
        local_packed.field2 = (temp >> 4) & 0xFF;
        global_packed.field3 = (temp >> 8) & 0xFFF;
        
        /* 4. Complex memory addressing with pointer arithmetic */
        /* Create non-trivial address expression for MEM_P(x) path */
        uint32_t index = (temp + i) % 16;
        
        /* Array access with computed index - complex addressing mode */
        uint32_t array_val = global_array[index];
        
        /* 5. More bit manipulation on memory value */
        /* Clear specific bits in memory location */
        global_array[index] &= ~(0xFF << 8);
        
        /* Set specific bits using OR */
        global_array[index] |= ((temp & 0xFF) << 8);
        
        /* 6. Pointer dereference with offset - another complex MEM address */
        uint32_t *ptr = ptr_array + index;
        uint32_t ptr_val = *ptr;
        
        /* 7. Mixed operations that may generate SUBREG */
        /* Cast to smaller type and back */
        uint16_t half_val = (uint16_t)ptr_val;
        uint32_t extended = (uint32_t)half_val + temp;
        
        /* 8. Bit-field extraction from memory */
        /* Extract specific bits from the value */
        uint32_t extracted_bits = (extended >> 4) & 0x7;
        
        /* 9. Update accumulator with all computed values */
        accumulator += temp + array_val + ptr_val + extracted_bits;
        
        /* 10. Modify base value for next iteration */
        base_value ^= (array_val << 4) | (extracted_bits << 8);
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Result: %u\n", accumulator);
    printf("Packed field2: %u, field3: %u\n", 
           (unsigned int)global_packed.field2,
           (unsigned int)global_packed.field3);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
