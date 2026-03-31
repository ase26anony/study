#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) packed_data {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
};

/* Volatile variables to prevent optimization */
volatile int global_result = 0;
volatile struct packed_data global_packed;

int main(void) {
    /* Stack variables with volatile to force memory operations */
    volatile int base = 0x12345678;
    volatile int array[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    volatile struct packed_data local_packed;
    
    /* Pointer to manipulate for complex MEM addresses */
    volatile int *ptr = array;
    
    /* Loop to create data dependencies and complex addressing */
    for (int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        base = base + i * 3;
        
        /* 2. Bit-field manipulation on packed structure 
           This is a prime candidate for ZERO_EXTRACT/STRICT_LOW_PART */
        local_packed.a = (i & 0x0F);           /* 4-bit field */
        local_packed.b = ((i * 2) & 0xFF);     /* 8-bit field */
        local_packed.c = ((i * 3) & 0xFFF);    /* 12-bit field */
        
        /* 3. Complex memory addressing with pointer arithmetic */
        int idx = (i & 0x07) + ((base >> 4) & 0x01);
        ptr = array + idx;
        
        /* 4. Mixed operation: modify specific bits of base */
        /* This may generate ZERO_EXTRACT for bit-field-like operation */
        base = (base & ~0x00000F00) | ((i << 8) & 0x00000F00);
        
        /* 5. Access through pointer with offset (complex MEM address) */
        *ptr = *ptr + local_packed.b;
        
        /* 6. Global packed structure access */
        global_packed.c = (global_packed.c + i) & 0xFFF;
        
        /* 7. Accumulate results to prevent optimization */
        global_result += base + *ptr + local_packed.a + global_packed.c;
        
        /* 8. Additional pointer manipulation for addressing complexity */
        if (i & 1) {
            ptr = (volatile int*)((char*)ptr + sizeof(int) - 1);
        }
    }
    
    /* Final output to ensure all operations are observable */
    printf("Result: %d\n", global_result);
    printf("Base: 0x%08x\n", base);
    printf("Packed: a=%u, b=%u, c=%u\n", 
           local_packed.a, local_packed.b, local_packed.c);
    
    return 0;
}
