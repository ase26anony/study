#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bit_packed {
    volatile uint32_t full;
    struct {
        volatile uint16_t low : 4;
        volatile uint16_t mid : 8;
        volatile uint16_t high : 4;
    } parts;
};

/* Global array to create complex MEM addresses */
static volatile int32_t global_array[32];

int main(void) {
    struct bit_packed bp = {0};
    volatile uint32_t accumulator = 0;
    volatile uint32_t index_reg = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Main loop creating complex RTL patterns */
    for (volatile uint32_t i = 0; i < 100; i++) {
        /* 1. Arithmetic creating potential SUBREG in RTL */
        uint32_t temp = i * 7;
        temp += (i & 0xF) << 8;  /* Mix high and low bits */
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bp.parts.low = (temp >> 4) & 0xF;    /* Likely ZERO_EXTRACT */
        bp.parts.mid = (temp >> 8) & 0xFF;   /* Another bit-field */
        bp.parts.high = (temp >> 16) & 0xF;  /* And another */
        
        /* 3. Complex memory addressing - may create MEM with non-trivial address */
        /* Compute index with arithmetic to prevent simple addressing */
        index_reg = (temp + i) % 32;
        
        /* Array access with computed index - creates complex MEM expression */
        int32_t array_val = global_array[index_reg];
        
        /* 4. Mixed operations on the fetched value */
        /* Bitwise operation that might use ZERO_EXTRACT on result */
        array_val &= ~(0xFF << bp.parts.low);  /* Clear bits based on bit-field */
        array_val |= (temp & 0xFF) << bp.parts.mid; /* Set bits */
        
        /* 5. Write back to memory with complex address */
        global_array[index_reg] = array_val;
        
        /* 6. Accumulate results to prevent optimization */
        accumulator += bp.full + array_val + temp;
        
        /* 7. Additional pointer arithmetic for more complex MEM addresses */
        volatile int32_t *ptr = &global_array[0];
        ptr += (i & 0x3);  /* Variable pointer offset */
        
        /* Access through pointer with offset - another complex MEM */
        *ptr = *ptr + accumulator;
        
        /* 8. Structure member access through pointer */
        struct bit_packed *bp_ptr = &bp;
        bp_ptr->parts.low = (accumulator >> 2) & 0xF;  /* More bit-field ops */
    }
    
    /* Final computation using all modified data */
    uint32_t final_result = 0;
    for (int i = 0; i < 32; i++) {
        final_result ^= global_array[i];  /* XOR all array elements */
    }
    final_result += accumulator + bp.full;
    
    printf("Result: %u\n", (unsigned int)final_result);
    return (int)(final_result & 0x7FFFFFFF);  /* Return non-negative */
}
