#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) bit_packed {
    volatile uint32_t full;
    struct {
        uint32_t low : 8;
        uint32_t mid : 12;
        uint32_t high : 12;
    } parts;
};

/* Global array with volatile elements to prevent optimization */
volatile uint32_t global_array[16];

int main(void) {
    struct bit_packed bp = {0};
    volatile uint32_t result = 0;
    uint32_t i, index;
    
    /* Initialize global array */
    for (i = 0; i < 16; i++) {
        global_array[i] = i * 3;
    }
    
    /* Main loop creating complex RTL patterns */
    for (i = 0; i < 100; i++) {
        /* 1. Arithmetic on base integer (potential SUBREG in RTL) */
        uint32_t base = i * 7;
        
        /* 2. Bit-field assignments (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        /* Modify specific bits using bitwise operations */
        bp.parts.low = (base & 0xFF);           /* Could generate ZERO_EXTRACT */
        bp.parts.mid = ((base >> 8) & 0xFFF);   /* Bit-field assignment */
        bp.parts.high = ((base >> 20) & 0xFFF); /* Another bit-field */
        
        /* 3. Complex memory addressing with pointer arithmetic */
        /* Create non-trivial address expression [base + offset] */
        index = (i * 13 + bp.parts.low) & 0xF;  /* Computed index */
        
        /* Access array with computed index (complex MEM address) */
        uint32_t temp = global_array[index];
        
        /* 4. Mixed operations to create data dependencies */
        /* Combine arithmetic with bitwise manipulation */
        temp = temp + bp.full;                  /* Arithmetic operation */
        
        /* Mask specific bits (potential for ZERO_EXTRACT in RTL) */
        temp = temp & ~(0xFF << 8);             /* Clear bits 8-15 */
        temp = temp | (bp.parts.mid << 8);      /* Set bits with bit-field */
        
        /* 5. Store back with different addressing */
        global_array[(index + 1) & 0xF] = temp; /* Another complex MEM */
        
        /* 6. Accumulate result to prevent optimization */
        result ^= temp + bp.parts.high + index;
        
        /* 7. Additional bit manipulation on volatile */
        /* Direct bit manipulation on volatile may generate desired RTL */
        bp.full = (bp.full & 0xFFFF0000) | (result & 0xFFFF);
    }
    
    /* Print result to ensure all operations are observable */
    printf("Result: %u\n", (unsigned int)result);
    
    /* Additional test with pointer dereferencing */
    {
        volatile uint32_t *ptr = &global_array[0];
        for (i = 0; i < 8; i++) {
            /* Pointer arithmetic followed by dereference */
            uint32_t val = *(ptr + i + (result & 0x3));
            
            /* Bit-field style operation on loaded value */
            uint32_t low_bits = val & 0xF;
            uint32_t high_bits = (val >> 28) & 0xF;
            
            /* Combine and store back with offset */
            *(ptr + i) = (high_bits << 4) | low_bits;
            
            result += val;
        }
    }
    
    printf("Final result: %u\n", (unsigned int)result);
    return 0;
}
