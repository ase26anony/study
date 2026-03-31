#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full;
    struct {
        uint32_t low : 8;
        uint32_t mid : 12;
        uint32_t high : 12;
    } parts;
    volatile uint8_t array[16];
};

/* Global variables to force memory operations */
static volatile struct bitfield_struct g_data;
static volatile uint32_t g_index = 0;

int main(void) {
    struct bitfield_struct local_data;
    volatile uint32_t accumulator = 0;
    volatile uint32_t *ptr_array[4];
    
    /* Initialize pointer array with different offsets into local_data.array */
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = (volatile uint32_t*)&local_data.array[i * 2];
    }
    
    /* Initialize data */
    local_data.full = 0x12345678;
    local_data.parts.low = 0xAA;
    local_data.parts.mid = 0xBBB;
    local_data.parts.high = 0xCCC;
    
    for (int i = 0; i < 16; i++) {
        local_data.array[i] = i;
    }
    
    /* Main loop creating complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        uint32_t temp = local_data.full + i;
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Modify specific bits using bitwise operations */
        temp &= ~0xFF;          /* Clear lower 8 bits */
        temp |= (i & 0xFF);     /* Set lower 8 bits from loop counter */
        
        /* 3. Complex memory addressing with pointer arithmetic */
        /* Access array with computed index */
        uint32_t idx = (i * 3 + 1) % 16;
        volatile uint8_t *addr = &local_data.array[idx];
        
        /* 4. Mixed operations: arithmetic then bit manipulation on memory */
        *addr += (temp & 0xF);  /* Modify memory based on computed value */
        
        /* 5. Access through pointer array with computed index */
        uint32_t ptr_idx = (i >> 2) & 0x3;
        *ptr_array[ptr_idx] ^= temp;
        
        /* 6. Direct bit-field structure member manipulation */
        /* This is most likely to generate ZERO_EXTRACT/STRICT_LOW_PART */
        local_data.parts.mid = (local_data.parts.mid + i) & 0xFFF;
        
        /* 7. Global variable access with complex addressing */
        g_index = (g_index * 13 + 7) % 16;
        g_data.array[g_index] = local_data.array[idx];
        
        /* Accumulate results to prevent optimization */
        accumulator += temp + *addr + local_data.parts.mid;
    }
    
    /* Final computation using all modified data */
    uint32_t final_result = accumulator;
    for (int i = 0; i < 16; i++) {
        final_result += local_data.array[i];
        final_result ^= g_data.array[i];
    }
    final_result += local_data.full;
    
    /* Print result to ensure all operations are observable */
    printf("Result: %u\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
