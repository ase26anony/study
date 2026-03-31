#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t field1 : 8;
    volatile uint32_t field2 : 12;
    volatile uint32_t field3 : 4;
    volatile uint32_t field4 : 8;
};

/* Global variables to force memory operations */
static volatile int global_base = 0x12345678;
static volatile int global_index = 0;
static volatile int global_result = 0;

/* Array with volatile elements to prevent optimization */
static volatile int mem_array[16];

int main(void) {
    struct bitfield_struct bf = {0};
    volatile int temp_result = 0;
    volatile int *ptr_array[4];
    
    /* Initialize pointer array with addresses to different memory locations */
    ptr_array[0] = &mem_array[0];
    ptr_array[1] = &mem_array[4];
    ptr_array[2] = &mem_array[8];
    ptr_array[3] = &mem_array[12];
    
    /* Initialize array with pattern */
    for (int i = 0; i < 16; i++) {
        mem_array[i] = i * 0x11111111;
    }
    
    /* Main loop to generate complex RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        int base = global_base + i * 0x10001;
        
        /* 2. Bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.field1 = (base >> 0) & 0xFF;        /* Likely ZERO_EXTRACT */
        bf.field2 = (base >> 8) & 0xFFF;       /* Likely ZERO_EXTRACT */
        bf.field3 = (base >> 20) & 0xF;        /* Likely ZERO_EXTRACT */
        bf.field4 = (base >> 24) & 0xFF;       /* Likely ZERO_EXTRACT */
        
        /* 3. Complex memory addressing - may generate MEM with non-trivial address */
        int idx = (i * 13 + 7) & 0x3;  /* Complex index calculation */
        volatile int *ptr = ptr_array[idx];
        
        /* 4. Mixed operations: arithmetic + bitwise */
        int array_val = *ptr;
        array_val += (bf.field1 << 16) | (bf.field2 << 4) | bf.field3;
        
        /* 5. Bitwise assignment to memory location */
        *ptr = array_val & 0x00FFFFFF;  /* May generate ZERO_EXTRACT for memory dest */
        
        /* 6. More complex addressing with pointer arithmetic */
        int offset = (i & 0x1) ? 1 : 2;
        volatile int *complex_ptr = ptr + offset;
        
        /* 7. Another bit-field like operation on memory */
        *complex_ptr = (*complex_ptr & 0xFF00FF00) | 
                      ((array_val & 0x000000FF) << 8) |
                      ((array_val & 0x00FF0000) >> 8);
        
        /* 8. Accumulate results to prevent optimization */
        temp_result += bf.field1 + bf.field2 + bf.field3 + bf.field4;
        temp_result += *ptr + *complex_ptr;
        
        /* 9. Update global index with bit manipulation */
        global_index = (global_index << 3) | (i & 0x7);
    }
    
    /* Final computation using all modified data */
    for (int i = 0; i < 16; i++) {
        global_result ^= mem_array[i];
    }
    
    global_result += temp_result;
    global_result += global_index;
    
    /* Print result to ensure all operations are observable */
    printf("Result: 0x%08x\n", global_result);
    
    return 0;
}
