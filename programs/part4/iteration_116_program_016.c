#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bit_packed {
    volatile uint32_t full;
    uint8_t a : 3;
    uint8_t b : 5;
    uint8_t c : 2;
    uint8_t d : 6;
    volatile uint16_t tail;
};

/* Global array to create complex MEM addresses */
static volatile int global_array[32];

int main(void) {
    struct bit_packed bp = {0};
    volatile uint32_t accumulator = 0;
    volatile int index;
    
    /* Initialize with non-zero values */
    bp.full = 0x12345678;
    bp.tail = 0xABCD;
    
    /* Loop to create data dependencies and prevent optimization */
    for (int i = 0; i < 100; i++) {
        /* 1. Arithmetic creating potential SUBREG in RTL */
        uint32_t temp = bp.full + i;
        
        /* 2. Bit-field assignments that may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Modify specific bits using bitwise operations */
        bp.a = (temp >> 0) & 0x7;      /* First 3 bits */
        bp.b = (temp >> 3) & 0x1F;     /* Next 5 bits */
        bp.c = (temp >> 8) & 0x3;      /* Next 2 bits */
        bp.d = (temp >> 10) & 0x3F;    /* Next 6 bits */
        
        /* 3. Complex MEM address calculation */
        /* Create index with arithmetic and bit manipulation */
        index = ((i * 7) ^ (temp & 0xFF)) % 32;
        
        /* Array access with computed index - creates complex addressing */
        global_array[index] = temp & 0xFFFF;
        
        /* 4. Mixed operations to encourage different RTL patterns */
        /* Combine arithmetic with bitwise assignment */
        bp.full = (bp.full + global_array[(i + 1) % 32]) | 0x1;
        
        /* 5. Pointer arithmetic for complex MEM addresses */
        volatile int *ptr = &global_array[0];
        ptr += (i & 0xF);  /* Variable offset */
        *ptr ^= (temp >> 16) & 0xFF;
        
        /* Accumulate results to prevent optimization */
        accumulator += bp.full + bp.a + bp.b + global_array[i % 32];
    }
    
    /* Final computation using all modified values */
    uint32_t result = accumulator + bp.tail;
    
    /* Print to ensure all operations are observable */
    printf("Result: %u\n", (unsigned int)result);
    printf("Bit field values: a=%u, b=%u, c=%u, d=%u\n",
           (unsigned int)bp.a, (unsigned int)bp.b,
           (unsigned int)bp.c, (unsigned int)bp.d);
    
    return 0;
}
