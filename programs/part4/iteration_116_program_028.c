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
volatile int global_base = 0x12345678;
volatile int global_index = 0;
static volatile int array[16] = {0};

int main(void) {
    struct bitfield_struct bf = {0};
    volatile int result = 0;
    volatile int *ptr = array;
    
    /* Mixed arithmetic and bitwise operations to create SUBREG and complex MEM patterns */
    for (int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        int temp = global_base + i * 3;
        
        /* 2. Bit-field assignment - potential ZERO_EXTRACT/STRICT_LOW_PART */
        bf.field2 = (temp & 0xFFF);  /* 12-bit field */
        bf.field3 = ((temp >> 12) & 0xF);  /* 4-bit field */
        
        /* 3. Complex memory addressing with pointer arithmetic */
        int idx = (i * 7 + global_index) & 0xF;
        volatile int *addr = ptr + idx;
        
        /* 4. Mixed operation: modify specific bits of memory location */
        *addr = (*addr & ~0x3F) | (temp & 0x3F);
        
        /* 5. Additional bit manipulation on the same memory */
        *addr ^= 1 << (i & 0x7);
        
        /* 6. Access through different pointer with offset */
        *(addr + 1) = *(addr + 1) + (bf.field2 & 0xFF);
        
        /* 7. Update global_index with bitwise operation */
        global_index = (global_index * 13 + 1) & 0xFF;
        
        /* Accumulate results to prevent optimization */
        result ^= temp ^ bf.field2 ^ *addr ^ idx;
        
        /* Force memory barrier-like behavior */
        asm volatile("" : : "r"(&bf), "r"(addr) : "memory");
    }
    
    /* Additional loop with nested bit operations */
    for (int i = 0; i < 50; i++) {
        /* Create data dependency chain */
        volatile int chain = result;
        
        /* Multiple bit-field assignments in sequence */
        bf.field1 = (chain >> 16) & 0xFF;
        bf.field4 = (chain >> 8) & 0xFF;
        
        /* Array access with computed index */
        int offset = (bf.field1 * bf.field4) & 0xF;
        array[offset] = array[offset] | (1 << (i & 0x1F));
        
        /* Update result with complex expression */
        result = result + (array[offset] & 0xFFFF);
        
        /* Pointer chasing with bit manipulation */
        volatile int *p = &array[(i * 3) & 0xF];
        *p = (*p & 0xFF00FF) | ((*p + i) & 0xFF00FF00);
    }
    
    printf("Result: %d\n", (int)result);
    printf("Bitfield values: %d %d %d %d\n", 
           (int)bf.field1, (int)bf.field2, (int)bf.field3, (int)bf.field4);
    
    return 0;
}
