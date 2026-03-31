#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t field1 : 8;
    volatile uint32_t field2 : 12;
    volatile uint32_t field3 : 4;
    volatile uint32_t field4 : 8;
};

/* Global variables to create complex addressing modes */
static volatile int global_array[16];
static volatile struct bitfield_struct bf_var;

int main(void) {
    volatile int result = 0;
    volatile int temp;
    volatile int *ptr;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 16; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Initialize bit-field structure */
    bf_var.field1 = 0xAB;
    bf_var.field2 = 0xCDE;
    bf_var.field3 = 0xF;
    bf_var.field4 = 0x12;
    
    /* Loop to create data dependencies and complex RTL */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic creating potential SUBREG in RTL */
        temp = i * 7 + 3;
        
        /* 2. Bitwise operations on arithmetic result - may generate ZERO_EXTRACT */
        /* Clear lower 4 bits */
        temp = temp & ~0xF;
        /* Set specific bits */
        temp = temp | 0x5A;
        
        /* 3. Complex memory addressing with pointer arithmetic */
        ptr = (volatile int*)global_array;
        ptr = ptr + (temp & 0x7);  /* Variable offset based on computation */
        
        /* 4. Memory store with complex address - may reach MEM_P(x) path */
        *ptr = temp;
        
        /* 5. Bit-field assignment - likely to generate ZERO_EXTRACT/STRICT_LOW_PART */
        if (i & 1) {
            bf_var.field2 = temp & 0xFFF;  /* 12-bit field assignment */
        } else {
            bf_var.field4 = (temp >> 4) & 0xFF;  /* 8-bit field assignment */
        }
        
        /* 6. Array access with computed index - complex MEM address */
        int idx = (i * 13 + temp) & 0xF;
        result += global_array[idx];
        
        /* 7. More bit manipulation on memory location */
        global_array[idx] = global_array[idx] ^ (temp << (i & 0x3));
    }
    
    /* Accumulate results from bit-fields */
    result += bf_var.field1;
    result += bf_var.field2;
    result += bf_var.field3;
    result += bf_var.field4;
    
    /* Final array checksum */
    for (int i = 0; i < 16; i++) {
        result += global_array[i];
    }
    
    printf("Result: %d\n", (int)result);
    return 0;
}
