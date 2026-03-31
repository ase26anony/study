#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bit_packed {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
};

/* Volatile variables to prevent optimization */
static volatile struct bit_packed g_packed;
static volatile int g_array[16];
static volatile int g_result = 0;

int main(void) {
    volatile int temp;
    volatile int index;
    volatile int base = 0x12345678;
    
    /* Loop to create data dependencies and complex addressing */
    for (int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        base = base + i * 3;
        
        /* 2. Bit-field assignment - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        /* Access bit-field members with volatile qualifier */
        g_packed.a = (base >> 0) & 0xF;
        g_packed.b = (base >> 4) & 0xFF;
        g_packed.c = (base >> 12) & 0xFFF;
        
        /* 3. Complex memory addressing with pointer arithmetic */
        /* Create non-trivial address expression for MEM_P(x) path */
        index = (i * 7 + base) & 0xF;
        
        /* Array access with computed index - creates complex MEM address */
        temp = g_array[index];
        
        /* 4. Mixed operations: arithmetic + bitwise */
        /* Modify specific bits of the loaded value */
        temp = temp & ~(0xFF << 8);    /* Clear bits 8-15 */
        temp = temp | ((base & 0xFF) << 8); /* Set bits 8-15 from base */
        
        /* Store back with modified bits - may generate ZERO_EXTRACT */
        g_array[index] = temp;
        
        /* 5. Additional bit manipulation on the packed structure */
        /* This creates another potential ZERO_EXTRACT destination */
        g_packed.d = (temp >> 4) & 0xFF;
        
        /* Accumulate results to prevent elimination */
        g_result += g_packed.a + g_packed.b + g_packed.c + g_packed.d + temp;
    }
    
    /* Use pointer arithmetic to create more complex MEM addresses */
    volatile int *ptr = &g_array[0];
    for (int i = 0; i < 8; i++) {
        /* Pointer arithmetic followed by dereference */
        int val = *(ptr + i * 2);
        
        /* Bit-field assignment on the packed structure */
        g_packed.b = val & 0xFF;
        
        /* Complex addressing: array with offset */
        g_array[(i + 3) & 0xF] = g_packed.b + g_packed.c;
        
        g_result += val;
    }
    
    printf("Result: %d\n", (int)g_result);
    return 0;
}
