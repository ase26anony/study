#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full;
    struct {
        uint32_t low : 8;
        uint32_t mid : 12;
        uint32_t high : 12;
    } bits;
};

/* Global array with volatile elements to prevent optimization */
volatile int global_array[16];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile int result = 0;
    volatile int *ptr_array[4];
    int indices[4] = {1, 3, 5, 7};
    
    /* Initialize pointer array with addresses to global_array elements */
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &global_array[indices[i]];
    }
    
    /* Main loop to generate complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic on base integer (potential for SUBREG) */
        uint32_t base = i * 37 + 123;
        
        /* 2. Bit-field assignment (potential for ZERO_EXTRACT/STRICT_LOW_PART) */
        bf.bits.low = (base >> 0) & 0xFF;    /* May generate ZERO_EXTRACT */
        bf.bits.mid = (base >> 8) & 0xFFF;   /* May generate ZERO_EXTRACT */
        bf.bits.high = (base >> 20) & 0xFFF; /* May generate ZERO_EXTRACT */
        
        /* 3. Mixed bitwise operations on volatile */
        volatile uint32_t temp = bf.full;
        temp &= ~0x00FF0000;  /* Clear middle bits */
        temp |= (i << 16);    /* Set middle bits from loop counter */
        bf.full = temp;       /* Assignment to volatile bit-field struct */
        
        /* 4. Complex memory addressing with pointer arithmetic */
        int idx = (i * 7 + 3) % 4;
        volatile int *ptr = ptr_array[idx] + (i & 3);  /* Pointer arithmetic */
        
        /* 5. Memory store with complex address (MEM_P path) */
        *ptr = bf.bits.low + bf.bits.mid;
        
        /* 6. Additional arithmetic to create data dependencies */
        int array_idx = ((bf.full >> 8) + i) & 0xF;
        global_array[array_idx] += *ptr;
        
        /* 7. Accumulate result to prevent optimization */
        result += bf.full + global_array[array_idx];
    }
    
    /* Print result to ensure all operations are observable */
    printf("Result: %d\n", (int)result);
    
    return 0;
}
