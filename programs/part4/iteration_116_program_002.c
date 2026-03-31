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
    volatile uint8_t control;
};

/* Global array with volatile elements to prevent optimization */
static volatile int global_array[16];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile int result = 0;
    volatile int *ptr_array[4];
    int i, j;
    
    /* Initialize pointer array with addresses to global_array */
    for (i = 0; i < 4; i++) {
        ptr_array[i] = &global_array[i * 3];
    }
    
    /* Main loop creating complex RTL patterns */
    for (i = 0; i < 100; i++) {
        /* 1. Arithmetic creating potential SUBREG patterns */
        uint32_t base = i * 7 + 12345;
        
        /* 2. Bit-field assignments - likely to generate ZERO_EXTRACT */
        bf.parts.low = (base >> 0) & 0xFF;      /* ZERO_EXTRACT candidate */
        bf.parts.mid = (base >> 8) & 0xFFF;     /* ZERO_EXTRACT candidate */
        bf.parts.high = (base >> 20) & 0xFFF;   /* ZERO_EXTRACT candidate */
        
        /* 3. Mixed bitwise operations on volatile */
        bf.full ^= (1 << (i % 16));             /* Bit manipulation */
        bf.full &= ~(0xFF << 8);                /* Mask operation */
        
        /* 4. Complex memory addressing - encourages MEM_P(x) path */
        for (j = 0; j < 4; j++) {
            /* Compute index with arithmetic to prevent simple addressing */
            int idx = (i + j * 3) & 0xF;
            
            /* Array access with computed index - complex MEM address */
            global_array[idx] += bf.parts.low + (j * 256);
            
            /* Pointer dereference with offset - another complex MEM */
            *(ptr_array[j] + (idx & 1)) = bf.parts.mid;
        }
        
        /* 5. Additional bit manipulation on memory */
        bf.control = (bf.control & 0xF0) | (i & 0x0F);  /* ZERO_EXTRACT candidate */
        
        /* Accumulate results to prevent optimization */
        result += bf.full + bf.parts.low + global_array[i & 0xF];
    }
    
    /* Final computation using all modified values */
    for (i = 0; i < 16; i++) {
        result ^= global_array[i];
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", (int)result);
    
    return 0;
}
