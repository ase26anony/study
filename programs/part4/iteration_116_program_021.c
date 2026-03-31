#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t field1 : 8;
    volatile uint32_t field2 : 12;
    volatile uint32_t field3 : 4;
    volatile uint32_t field4 : 8;
};

/* Global array to create complex MEM addresses */
static volatile int global_array[32];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t base_value = 0x12345678;
    volatile uint32_t mask_value = 0x0000FF00;
    volatile uint32_t result = 0;
    
    /* Local array with pointer arithmetic */
    volatile int local_array[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 16; i++) {
        local_array[i] = i * 2;
    }
    
    /* Loop to create data dependencies and complex RTL */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        base_value = base_value + (i & 0xFF);
        
        /* 2. Bitwise assignment to specific bits - may generate ZERO_EXTRACT */
        /* Mask out bits 8-15, then set new value */
        base_value = (base_value & ~mask_value) | ((i * 7) & mask_value);
        
        /* 3. Bit-field assignments in packed structure - likely ZERO_EXTRACT */
        bf.field1 = (base_value >> 0) & 0xFF;
        bf.field2 = (base_value >> 8) & 0xFFF;
        bf.field3 = (base_value >> 20) & 0xF;
        bf.field4 = (base_value >> 24) & 0xFF;
        
        /* 4. Complex array access with pointer arithmetic - creates MEM with complex address */
        /* Use bit manipulation to compute index */
        int idx = (base_value ^ (i * 3)) & 0x1F;
        
        /* Array access with computed index - forces complex addressing mode */
        int temp = global_array[idx];
        
        /* Further pointer manipulation */
        volatile int *ptr = &local_array[0];
        ptr += (idx & 0x0F);
        
        /* Dereference with offset - encourages MEM_P(x) path */
        temp += *ptr;
        temp += *(ptr + ((i & 0x3) * 2));
        
        /* 5. Mixed operations on the bit-field structure */
        /* Access bit-field, modify, and assign back */
        uint32_t temp_field = bf.field2;
        temp_field = (temp_field + i) & 0xFFF;  /* Keep within bit-field range */
        bf.field2 = temp_field;
        
        /* 6. Accumulate results to prevent optimization */
        result ^= base_value;
        result += bf.field1 + bf.field2 + bf.field3 + bf.field4;
        result += temp;
        
        /* Additional volatile write to ensure all operations are observable */
        global_array[idx & 0x1F] = result & 0x7FFF;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %u\n", (unsigned int)result);
    printf("Bitfield values: %u %u %u %u\n", 
           (unsigned int)bf.field1, 
           (unsigned int)bf.field2,
           (unsigned int)bf.field3,
           (unsigned int)bf.field4);
    
    return 0;
}
