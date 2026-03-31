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
    volatile uint8_t array[16];
};

/* Global variables to create complex addressing modes */
static volatile int global_index = 3;
static volatile int global_offset = 7;

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    volatile uint32_t *ptr_array[4];
    
    /* Initialize pointer array with different offsets */
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &bf.array[i * 2];
    }
    
    /* Main loop creating data dependencies and complex RTL */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic creating potential SUBREG in RTL */
        uint32_t base = i * 17 + 123;
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.parts.low = (base & 0xFF);          /* Likely ZERO_EXTRACT */
        bf.parts.mid = ((base >> 8) & 0xFFF);  /* Likely ZERO_EXTRACT */
        
        /* 3. Complex memory addressing with pointer arithmetic */
        int idx = (i + global_index) & 3;
        int offset = (i ^ global_offset) & 1;
        
        /* This creates MEM with complex address: [reg1 + reg2*scale + offset] */
        volatile uint8_t *target = ptr_array[idx] + offset;
        
        /* 4. Mixed operation: modify memory through complex address */
        *target = (*target ^ base) & 0x7F;
        
        /* 5. Another bit-field operation after memory access */
        bf.parts.high = (bf.parts.high + *target) & 0xFFF;
        
        /* 6. Additional arithmetic that might use SUBREG */
        bf.full = bf.full + (base << 3);
        
        /* 7. Bitwise assignment to volatile - may generate ZERO_EXTRACT */
        bf.full &= 0x00FFFFFF;  /* Clear top 8 bits */
        
        /* Accumulate results to prevent optimization */
        accumulator ^= bf.full;
        accumulator += bf.parts.low;
        accumulator += bf.parts.mid;
        accumulator += bf.parts.high;
        accumulator += *target;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %u\n", accumulator);
    
    /* Additional test case with direct bit manipulation */
    {
        volatile uint32_t test_var = 0x12345678;
        
        /* These operations often generate ZERO_EXTRACT in RTL */
        test_var = (test_var & 0xFFFF0000) | 0xABCD;
        test_var ^= 0x00FF00FF;
        
        /* Access specific bytes - may create SUBREG */
        uint8_t *byte_ptr = (uint8_t*)&test_var;
        for (int j = 0; j < 4; j++) {
            byte_ptr[j] = byte_ptr[j] + j;
        }
        
        printf("Test var: 0x%08X\n", test_var);
    }
    
    return 0;
}
