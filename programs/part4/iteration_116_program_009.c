#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t field1 : 4;
    volatile uint32_t field2 : 8;
    volatile uint32_t field3 : 12;
    volatile uint32_t field4 : 8;
};

/* Global array with volatile elements to prevent optimization */
volatile int global_array[16];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile int result = 0;
    volatile int index = 0;
    
    /* Base variable that will undergo arithmetic operations */
    volatile uint32_t base_value = 0x12345678;
    
    /* Loop to create data dependencies and complex addressing */
    for (int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        base_value = base_value + i * 3;
        
        /* 2. Bit-field assignments - likely to generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.field1 = (base_value >> 0) & 0xF;   /* Extract bits 0-3 */
        bf.field2 = (base_value >> 4) & 0xFF;  /* Extract bits 4-11 */
        bf.field3 = (base_value >> 12) & 0xFFF; /* Extract bits 12-23 */
        
        /* 3. Mixed bitwise operations on the base value */
        base_value = (base_value & 0xFFFF0000) | (i & 0xFFFF);
        
        /* 4. Complex memory addressing with pointer arithmetic */
        index = (i * 7 + base_value) % 16;
        global_array[index] = base_value + bf.field1 + bf.field2;
        
        /* 5. More bit manipulation with volatile */
        bf.field4 = (global_array[index] >> 16) & 0xFF;
        
        /* 6. Create data dependency chain */
        result += base_value + bf.field1 + bf.field2 + bf.field3 + bf.field4;
        
        /* 7. Additional memory access with computed address */
        volatile int* ptr = &global_array[(i + 1) % 16];
        *ptr = result & 0xFF;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", (int)result);
    
    /* Also print bit-field values to ensure they're used */
    printf("Bitfields: %u %u %u %u\n", 
           (unsigned)bf.field1, 
           (unsigned)bf.field2, 
           (unsigned)bf.field3, 
           (unsigned)bf.field4);
    
    return 0;
}
