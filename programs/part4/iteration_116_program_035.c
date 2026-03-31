#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full;
    struct {
        uint8_t low : 4;
        uint8_t high : 4;
        uint16_t middle : 12;
        uint8_t spare : 4;
    } bits;
    volatile uint8_t array[8];
};

/* Global variables to force memory operations */
static volatile int global_index = 0;
static volatile int global_mask = 0x0F;

int main(void) {
    struct bitfield_struct data;
    volatile uint32_t accumulator = 0;
    volatile int* volatile ptr_array[4];
    int temp_array[4] = {0};
    
    /* Initialize pointer array with addresses to create complex addressing */
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &temp_array[i];
    }
    
    /* Initialize data */
    data.full = 0x12345678;
    data.bits.low = 0x5;
    data.bits.high = 0xA;
    data.bits.middle = 0x789;
    
    for (int i = 0; i < 100; i++) {
        volatile int idx = i & 3;
        
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        data.full += i * 256 + 1;  /* Multi-byte operation */
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Modify specific bits using bitwise operations */
        data.full &= ~0xF00;      /* Clear bits 8-11 */
        data.full |= (i & 0xF) << 8; /* Set bits 8-11 from loop counter */
        
        /* 3. Direct bit-field structure member access */
        /* This is most likely to generate ZERO_EXTRACT for packed bit-fields */
        data.bits.low = (data.bits.low + 1) & 0xF;
        data.bits.high ^= 0x3;  /* Toggle bits */
        
        /* 4. Array access with computed index - complex MEM addressing */
        /* Pointer arithmetic creates [reg + offset] or [reg1 + reg2] addressing */
        volatile int* ptr = ptr_array[idx];
        *ptr += data.full & 0xFF;
        
        /* 5. More complex: array indexing with scaled offset */
        data.array[(i * 3) & 7] = (data.full >> 8) & 0xFF;
        
        /* 6. Mixed operation: arithmetic then bit manipulation */
        uint32_t temp = data.full;
        temp = temp + (temp << 2);  /* temp * 5 */
        /* Mask specific bits - may generate ZERO_EXTRACT as destination */
        temp &= ~((1 << 16) - 1);   /* Clear lower 16 bits */
        data.full ^= temp;          /* XOR with modified value */
        
        /* Accumulate results to prevent optimization */
        accumulator += data.full;
        accumulator += data.bits.low;
        accumulator += data.bits.high;
        accumulator += *ptr;
        accumulator += data.array[i & 7];
        
        /* Update global index to create dependencies */
        global_index = (global_index + i) & 7;
    }
    
    /* Final computation using all modified data */
    volatile uint32_t result = accumulator;
    result ^= data.full;
    result ^= data.bits.middle << 8;
    
    /* Access through pointer with offset - complex MEM address */
    volatile int* final_ptr = &temp_array[global_index & 3];
    result += *final_ptr;
    
    printf("Result: %u\n", (unsigned int)result);
    return 0;
}
