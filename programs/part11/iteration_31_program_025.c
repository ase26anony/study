/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    volatile unsigned int a : 3;    /* 3-bit field */
    volatile unsigned int b : 12;   /* 12-bit field */
    volatile unsigned int c : 1;    /* 1-bit field */
    volatile unsigned int d : 8;    /* 8-bit field */
    volatile unsigned int e : 8;    /* 8-bit field */
};

/* Union for accessing same memory as different types */
union data_union {
    volatile uint64_t full;
    struct {
        volatile uint32_t low;
        volatile uint32_t high;
    } parts;
    volatile double as_double;
};

/* Function to force complex addressing modes */
static inline uint32_t complex_index(volatile uint32_t *arr, int idx, int stride) {
    /* Complex addressing: arr[idx*stride + (idx & 7)] */
    return arr[idx * stride + (idx & 7)];
}

/* Function with inline assembly to mimic STRICT_LOW_PART */
static inline uint32_t asm_low_part(uint64_t val) {
    uint32_t result;
    /* Assembly that operates only on low 32 bits */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl $0x1234, %0"
        : "=r" (result)
        : "r" ((uint32_t)val)
        : "cc"
    );
    return result;
}

int main(int argc, char *argv[]) {
    volatile struct bitfield_struct bf = {0};
    volatile union data_union data = {0};
    volatile uint64_t big_val = 0x123456789ABCDEF0ULL;
    volatile double dbl_val = 3.141592653589793;
    
    /* Array with complex access patterns */
    volatile uint32_t array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    volatile int iterations = (argc > 1) ? (argc & 15) + 5 : 10;
    
    uint32_t accumulator = 0;
    
    /* Main loop with mixed operations */
    for (volatile int i = 0; i < iterations; i++) {
        /* 1. Bit-field operations (ZERO_EXTRACT/STRICT_LOW_PART) */
        bf.a = (i & 0x7);                     /* 3-bit field */
        bf.b = (i * 17) & 0xFFF;              /* 12-bit field */
        bf.c = (i ^ (i >> 1)) & 1;            /* 1-bit field (Gray code LSB) */
        
        /* Extract bit-field values with masking (may generate ZERO_EXTRACT) */
        uint32_t extracted = ((uint32_t)bf.b << 3) | bf.a;
        
        /* Bit-field assignment with wider source (STRICT_LOW_PART potential) */
        bf.d = extracted & 0xFF;
        bf.e = (extracted >> 8) & 0xFF;
        
        /* 2. Multi-word operations (SUBREG generation) */
        /* Operations on 64-bit values on 32-bit targets generate SUBREG */
        big_val = big_val * 0x1234567ULL + i;
        data.full = big_val ^ 0xF0F0F0F0F0F0F0F0ULL;
        
        /* Mix with double operations (different register class) */
        dbl_val = dbl_val * 1.1 + (double)i;
        data.as_double = dbl_val;
        
        /* Access high/low parts separately (forces SUBREG usage) */
        uint32_t low_part = data.parts.low;
        uint32_t high_part = data.parts.high;
        
        /* 3. Complex memory addressing */
        int stride = (extracted & 0x3) + 2;  /* Variable stride: 2-5 */
        uint32_t array_val = complex_index(array, i & 0xFF, stride);
        
        /* Read-modify-write with bit-field result */
        array[i & 0xFF] = array_val ^ extracted;
        
        /* 4. Control flow based on bit-field and multi-word results */
        if (bf.c) {  /* Branch based on 1-bit field */
            /* When bit is set, use inline assembly */
            accumulator += asm_low_part(big_val);
            
            /* Additional bit manipulation */
            bf.d = (bf.d ^ bf.e) & 0x7F;
        } else {
            /* When bit is clear, compare high/low words */
            if (data.parts.high > data.parts.low) {
                accumulator += data.parts.high - data.parts.low;
            } else {
                accumulator += data.parts.low - data.parts.high;
            }
            
            /* More bit-field operations */
            bf.e = (bf.d + bf.a) & 0xFF;
        }
        
        /* Switch based on extracted value */
        switch (extracted & 0x7) {  /* 3-bit switch */
            case 0:
                big_val = big_val >> 1;
                break;
            case 1:
                big_val = big_val << 1;
                break;
            case 2:
                big_val = big_val ^ data.full;
                break;
            case 3:
                /* Access misaligned data through pointer casting */
                uint8_t *byte_ptr = (uint8_t*)&big_val;
                uint32_t word = *(uint32_t*)(byte_ptr + 1);  /* Misaligned */
                accumulator += word & 0xFFFF;
                break;
            default:
                /* Mixed operation */
                dbl_val = dbl_val / 2.0;
                break;
        }
        
        /* Accumulate results from array access */
        accumulator += array[i & 0xFF] & 0xFF;
    }
    
    /* Final aggregation to prevent optimization */
    uint32_t final_result = accumulator;
    final_result += bf.a + (bf.b << 3) + (bf.c << 15);
    final_result += (uint32_t)(data.parts.low ^ data.parts.high);
    final_result += (uint32_t)(dbl_val * 1000.0);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %u\n", final_result);
    
    return (int)(final_result & 0xFF);
}
