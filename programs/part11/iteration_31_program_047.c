/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL expressions */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    volatile unsigned int a : 1;    /* 1-bit field */
    volatile unsigned int b : 3;    /* 3-bit field */
    volatile unsigned int c : 12;   /* 12-bit field */
    volatile unsigned int d : 16;   /* 16-bit field */
    volatile unsigned int pad : 0;  /* force alignment */
};

/* Union for accessing same memory as different types */
union mixed_access {
    volatile uint32_t word;
    volatile struct bitfield_pack bits;
    volatile uint8_t bytes[4];
};

/* Function to force complex addressing modes */
static inline uint32_t complex_index(volatile uint32_t *arr, int idx, int stride) {
    /* Complex addressing: arr[idx*stride + (idx & 3)] */
    return arr[idx * stride + (idx & 3)];
}

/* Function with inline assembly to mimic STRICT_LOW_PART */
static inline uint32_t asm_low_part(uint64_t val) {
    uint32_t result;
    /* Assembly that operates only on low 32 bits */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl $0x1234, %0"
        : "=r" (result)
        : "r" ((uint32_t)val)  /* Cast ensures only low part used */
        : "cc"
    );
    return result;
}

int main(int argc, char *argv[]) {
    volatile union mixed_access data;
    volatile uint64_t big_val = 0x123456789ABCDEF0ULL;
    volatile double fp_val = 3.141592653589793;
    volatile uint32_t array[256];
    volatile int i, j;
    uint32_t total = 0;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i * 0x01010101;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    volatile int iterations = (argc > 1) ? (argc & 31) : 16;
    if (iterations < 4) iterations = 4;
    
    /* Main loop with mixed operations */
    for (i = 0; i < iterations; i++) {
        /* 1. Bit-field operations (ZERO_EXTRACT/STRICT_LOW_PART) */
        data.bits.a = i & 1;
        data.bits.b = (i >> 1) & 0x7;
        data.bits.c = (i * 0x123) & 0xFFF;
        data.bits.d = (data.bits.c << 4) | data.bits.b;
        
        /* Extract using masks and shifts (may generate ZERO_EXTRACT) */
        uint32_t extracted = ((data.word >> 4) & 0xFFF);  /* Extract 12-bit field */
        
        /* 2. Multi-word operations (SUBREG generation) */
        /* Operations on 64-bit values on 32-bit target generate SUBREG */
        big_val = big_val * 0x1234567ULL + (extracted << 16);
        fp_val = fp_val * 1.234567 - (double)(big_val & 0xFFFFFFFF);
        
        /* Split 64-bit operation into high/low parts */
        uint32_t low_part = big_val & 0xFFFFFFFF;
        uint32_t high_part = big_val >> 32;
        
        /* 3. Complex memory addressing with bit-field results */
        int idx = (data.bits.c * data.bits.b) & 0xFF;
        uint32_t val = complex_index(array, idx, 4);
        
        /* Modify array element using bit-field controlled mask */
        array[idx] = (val & ~0xFF) | (data.bits.b << data.bits.a);
        
        /* 4. Conditional based on bit-field parity and 64-bit comparison */
        if ((data.bits.a ^ (low_part & 1)) || (high_part > 0x10000000)) {
            /* Branch 1: More bit-field manipulation */
            data.bits.c = (data.bits.c + low_part) & 0xFFF;
            
            /* Use inline assembly for low-part operation */
            uint32_t asm_result = asm_low_part(big_val);
            total += asm_result & 0xFFFF;
        } else {
            /* Branch 2: Different operations */
            /* Force SUBREG by accessing 64-bit as two 32-bit parts */
            volatile uint32_t *parts = (volatile uint32_t*)&big_val;
            parts[0] += parts[1];  /* low += high */
            parts[1] ^= extracted;
            
            /* Array access with complex index */
            int idx2 = (idx + (low_part & 0xF)) & 0xFF;
            total += array[idx2] & 0xFF;
        }
        
        /* 5. Additional operations to create complex RTL patterns */
        /* Misaligned access through byte pointer */
        volatile uint8_t *byte_ptr = (volatile uint8_t*)&data.word;
        byte_ptr[1] = byte_ptr[2] ^ byte_ptr[3];
        
        /* Bit-field assignment that might generate STRICT_LOW_PART */
        data.bits.d = (data.bits.d & 0xFF00) | (extracted & 0xFF);
        
        /* Combine results */
        total = (total * 0x1234567) + data.word + (low_part & 0xFFFF);
    }
    
    /* Final aggregation to prevent dead code elimination */
    uint32_t final_result = total + (big_val & 0xFFFFFFFF) + (uint32_t)fp_val;
    
    /* Use result to affect return value */
    return (final_result & 0xFF) + argc;
}
