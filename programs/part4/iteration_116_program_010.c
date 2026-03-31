#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full;
    struct {
        uint32_t low : 8;
        uint32_t mid : 12;
        uint32_t high : 12;
    } bits;
    volatile uint8_t array[16];
};

/* Global variables to force memory operations */
static volatile int global_index = 0;
static volatile int global_mask = 0xFF;

int main(void) {
    struct bitfield_struct data;
    volatile uint32_t accumulator = 0;
    volatile uint32_t *ptr_array[4];
    
    /* Initialize data */
    data.full = 0x12345678;
    data.bits.low = 0xAA;
    data.bits.mid = 0xBBB;
    data.bits.high = 0xCCC;
    
    for (int i = 0; i < 16; i++) {
        data.array[i] = i * 3;
    }
    
    /* Setup pointer array with different offsets */
    ptr_array[0] = &data.full;
    ptr_array[1] = (uint32_t*)&data.bits;
    ptr_array[2] = (uint32_t*)&data.array[0];
    ptr_array[3] = (uint32_t*)&data.array[8];
    
    /* Main loop to generate complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG */
        uint32_t temp = data.full + i;
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Modify specific bits using bitwise operations */
        temp = (temp & ~0xFF) | (i & 0xFF);          /* Replace low byte */
        temp = (temp & ~(0xFFF << 8)) | ((i * 3) << 8); /* Modify middle bits */
        
        /* Force the result back through volatile */
        data.full = temp;
        
        /* 3. Complex memory addressing with array indexing */
        /* Compute index with arithmetic to prevent simple addressing */
        int idx = (i * 7 + 3) % 4;
        
        /* Access through pointer array with offset - creates complex MEM */
        uint32_t mem_val = ptr_array[idx][global_index];
        
        /* 4. Bit-field structure member access */
        /* These assignments often generate ZERO_EXTRACT for packed bit-fields */
        data.bits.low = (data.bits.low + 1) & 0xFF;
        data.bits.mid = (data.bits.mid * 2) & 0xFFF;
        data.bits.high = (data.bits.high ^ mem_val) & 0xFFF;
        
        /* 5. Array access with computed index - complex addressing mode */
        int arr_idx = (i + global_index) % 16;
        data.array[arr_idx] = (data.array[arr_idx] + i) & global_mask;
        
        /* Accumulate results to prevent optimization */
        accumulator += data.full + data.bits.low + data.bits.mid + 
                      data.bits.high + mem_val + data.array[arr_idx];
        
        /* Modify global_index to vary addressing */
        global_index = (global_index + 1) % 2;
    }
    
    /* Print result to ensure all operations are observable */
    printf("Result: %u\n", accumulator);
    
    return 0;
}
