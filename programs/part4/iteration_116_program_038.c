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
volatile uint32_t global_array[16];

int main(void) {
    struct bit_packed bp = {0};
    volatile uint32_t result = 0;
    volatile uint32_t index_reg = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 16; i++) {
        global_array[i] = i * 3;
    }
    
    /* Main loop to generate complex RTL patterns */
    for (volatile uint32_t i = 0; i < 100; i++) {
        /* 1. Arithmetic creating potential for SUBREG in RTL */
        uint32_t temp = i * 7;
        temp += (i << 3) | (i >> 2);  /* Mixed shifts encourage complex RTL */
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bp.parts.mid = (temp & 0xFFF);  /* Direct bit-field assignment */
        
        /* 3. Bitwise operations on volatile - may generate ZERO_EXTRACT */
        bp.full = (bp.full & ~(0xFF << 8)) | ((temp & 0xFF) << 8);
        
        /* 4. Complex memory addressing with variable index */
        index_reg = (i * 13 + temp) % 16;
        
        /* Array access with computed index - creates complex MEM addresses */
        uint32_t array_val = global_array[index_reg];
        
        /* 5. More bit manipulation with memory result */
        array_val ^= (bp.parts.low << 4);
        
        /* 6. Another potential ZERO_EXTRACT pattern */
        bp.parts.high = (array_val >> 4) & 0xFFF;
        
        /* 7. Mixed operation creating data dependencies */
        volatile uint32_t* ptr = (volatile uint32_t*)&global_array[0];
        ptr += (index_reg + 1) & 0xF;  /* Pointer arithmetic */
        
        /* 8. Memory store with complex address - may generate MEM_P(x) path */
        *ptr = (*ptr + temp) & 0xFFFFFF;
        
        /* Accumulate result to prevent optimization */
        result ^= bp.full + array_val + *ptr;
        
        /* Additional arithmetic to create SUBREG possibilities */
        uint64_t wide_val = (uint64_t)temp * (uint64_t)bp.full;
        result += (wide_val >> 16) & 0xFFFF;  /* Extract middle bits */
    }
    
    /* Print result to ensure all operations are observable */
    printf("Result: %u\n", (unsigned int)result);
    
    /* Additional bit-field operations outside loop */
    struct {
        volatile uint32_t a : 4;
        volatile uint32_t b : 4;
        volatile uint32_t c : 4;
    } small_fields = {0};
    
    for (int i = 0; i < 10; i++) {
        small_fields.a = i & 0xF;
        small_fields.b = (i + 1) & 0xF;
        small_fields.c = (i + 2) & 0xF;
        result += small_fields.a + small_fields.b + small_fields.c;
    }
    
    printf("Final result: %u\n", (unsigned int)result);
    return 0;
}
