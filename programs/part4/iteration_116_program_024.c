#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bit_packed {
    volatile uint32_t full;
    uint8_t a : 3;
    uint8_t b : 5;
    uint8_t c : 4;
    uint8_t d : 4;
    volatile uint16_t trailer;
};

/* Global array to create complex MEM addresses */
static volatile int global_array[32];

int main(void) {
    struct bit_packed bp = {0};
    volatile uint32_t accumulator = 0;
    volatile int *ptr_array[8];
    int indices[8];
    
    /* Initialize pointers with arithmetic to create complex addressing */
    for (int i = 0; i < 8; i++) {
        indices[i] = (i * 3 + 1) & 7;
        ptr_array[i] = &global_array[indices[i]];
    }
    
    /* Main loop to generate complex RTL patterns */
    for (volatile int outer = 0; outer < 100; outer++) {
        /* 1. Arithmetic on base integer (potential SUBREG) */
        uint32_t base = outer * 0x12345;
        
        /* 2. Bit-field assignments (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        bp.a = (base >> 0) & 0x7;      /* 3-bit field */
        bp.b = (base >> 3) & 0x1F;     /* 5-bit field */
        bp.c = (base >> 8) & 0xF;      /* 4-bit field */
        bp.d = (base >> 12) & 0xF;     /* 4-bit field */
        
        /* 3. Mixed operations on the full field */
        bp.full ^= 0xABCD;
        bp.full += outer;
        
        /* 4. Complex memory addressing with pointer arithmetic */
        int idx = (outer + (bp.a << 1)) & 7;
        volatile int *ptr = ptr_array[idx];
        
        /* Create dependency chain for addressing */
        ptr += (bp.b & 0x3);
        *ptr = (*ptr + outer) & 0xFF;
        
        /* 5. Bit manipulation on memory location (potential ZERO_EXTRACT) */
        uint32_t temp = *ptr;
        temp = (temp & ~0x0F00) | ((outer & 0xF) << 8);
        *ptr = temp;
        
        /* 6. Access packed structure through pointer with offset */
        uint8_t *byte_ptr = (uint8_t*)&bp;
        byte_ptr += 1 + (outer & 0x1);
        *byte_ptr ^= 0x55;
        
        /* 7. Accumulate results to prevent optimization */
        accumulator += bp.full + *ptr + bp.a + bp.b;
        
        /* 8. Additional bit-field operation on volatile */
        bp.trailer = (bp.trailer << 1) | ((accumulator >> 3) & 0x1);
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %u\n", accumulator);
    printf("Structure: a=%u b=%u c=%u d=%u full=%u trailer=%u\n",
           bp.a, bp.b, bp.c, bp.d, bp.full, bp.trailer);
    
    return 0;
}
