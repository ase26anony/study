#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bit_packed {
    volatile uint32_t field1 : 8;
    volatile uint32_t field2 : 12;
    volatile uint32_t field3 : 4;
    volatile uint32_t field4 : 8;
};

/* Global variables to force memory operations */
volatile int global_base = 0x12345678;
volatile int global_index = 0;
static volatile int result = 0;

int main(void) {
    struct bit_packed bp = {0};
    volatile int array[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    volatile int temp;
    volatile int *ptr;
    
    /* Loop to create data dependencies and complex addressing */
    for (int i = 0; i < 100; i++) {
        /* 1. Create SUBREG potential through arithmetic on mixed-size data */
        temp = global_base + i;
        
        /* 2. Bit-field assignments that may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Manipulate specific bits of temp using bitwise operations */
        temp = (temp & ~0xFF) | (i & 0xFF);          /* Modify lower 8 bits */
        temp = (temp & ~(0xF << 8)) | ((i & 0xF) << 8); /* Modify bits 8-11 */
        
        /* 3. Access packed structure bit-fields (ZERO_EXTRACT candidates) */
        bp.field1 = (temp >> 0) & 0xFF;
        bp.field2 = (temp >> 8) & 0xFFF;
        bp.field3 = (temp >> 20) & 0xF;
        bp.field4 = (temp >> 24) & 0xFF;
        
        /* 4. Complex memory addressing with pointer arithmetic */
        /* Create non-trivial address expression for MEM_P(x) path */
        global_index = (global_index + 1) & 0x7;
        ptr = &array[global_index];
        
        /* 5. Mixed operations to create complex RTL patterns */
        /* Array access with computed index */
        *ptr = *ptr + bp.field1 + bp.field2;
        
        /* 6. More bit manipulation on memory location */
        *ptr = (*ptr & ~0x3FF) | (temp & 0x3FF);
        
        /* 7. Additional pointer arithmetic for complex MEM addresses */
        ptr = &array[(i * 13 + 7) & 0x7];  /* Non-linear indexing */
        *ptr = *ptr - bp.field3;
        
        /* 8. Accumulate results to prevent optimization */
        result += temp + *ptr + bp.field4;
        
        /* 9. Force memory barrier-like behavior */
        asm volatile("" : : "r"(result) : "memory");
    }
    
    /* Print result to ensure all operations are observable */
    printf("Result: %d\n", result);
    
    return 0;
}
