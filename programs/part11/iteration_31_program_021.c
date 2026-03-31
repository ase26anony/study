/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag : 1;
    unsigned int mode : 3;
    unsigned int data : 12;
    unsigned int count : 8;
    unsigned int pad : 8;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_pack bits;
    uint32_t word;
    uint16_t halves[2];
    uint8_t bytes[4];
};

/* Function with complex operations to generate target RTL patterns */
int process_resources(int argc, char **argv) {
    volatile struct bitfield_pack bf = {0};
    volatile union mixed_access mix = {0};
    
    /* Use volatile to prevent optimization */
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Control loop with volatile to prevent unrolling */
    volatile int iterations = (argc > 1) ? 10 : 5;
    int result = 0;
    
    /* Main loop with operations designed to generate target RTL */
    for (volatile int loop = 0; loop < iterations; loop++) {
        /* ============================================
         * Operations to generate ZERO_EXTRACT/STRICT_LOW_PART
         * ============================================ */
        
        /* Bit-field assignments - may generate STRICT_LOW_PART */
        bf.flag = loop & 1;
        bf.mode = (loop >> 1) & 0x7;
        bf.data = (loop * 37) & 0xFFF;
        bf.count = (bf.count + 1) & 0xFF;
        
        /* Manual bit extraction using masks - may generate ZERO_EXTRACT */
        uint32_t temp = mix.word;
        uint32_t extracted = (temp >> 4) & 0xFFF;  /* 12-bit extract */
        uint32_t masked = temp & 0xFFFF;          /* Strict low 16 bits */
        
        /* Combine bit-field operations */
        mix.bits.data = extracted ^ masked;
        mix.bits.mode = (mix.bits.mode + bf.mode) & 0x7;
        
        /* ============================================
         * Operations to generate SUBREG for multi-word types
         * ============================================ */
        
        /* Operations on long long - may generate SUBREG for high/low parts */
        ll_var = ll_var + 0x100000001LL;
        ll_var = ll_var ^ (0xFFFFFFFFLL << (loop & 0x1F));
        ll_var = ll_var | ((long long)bf.data << 20);
        
        /* Double operations on 32-bit arch may use SUBREG */
        dbl_var = dbl_var * 1.01;
        dbl_var = dbl_var + (double)loop;
        
        /* Compare high vs low parts of long long */
        uint32_t ll_low = (uint32_t)(ll_var & 0xFFFFFFFF);
        uint32_t ll_high = (uint32_t)(ll_var >> 32);
        
        /* ============================================
         * Complex memory addressing with bit-field derived indices
         * ============================================ */
        
        /* Array access with complex index calculation */
        int idx = (bf.data * 7 + loop * 13) & 0xFF;
        int stride = (bf.mode + 1) * 4;
        
        /* Multiple array operations with different addressing */
        array[idx] = array[idx] + bf.count;
        array[(idx + stride) & 0xFF] = array[idx] ^ extracted;
        array[(idx + stride * 2) & 0xFF] = array[(idx + stride) & 0xFF] | masked;
        
        /* Pointer arithmetic with casting */
        uint8_t *byte_ptr = (uint8_t *)&array[idx];
        uint16_t *half_ptr = (uint16_t *)&array[(idx + 64) & 0xFF];
        
        /* Misaligned/special access patterns */
        *byte_ptr = (uint8_t)(mix.bytes[0] ^ mix.bytes[2]);
        *half_ptr = (uint16_t)(mix.halves[0] + mix.halves[1]);
        
        /* ============================================
         * Control flow based on bit-field results
         * ============================================ */
        
        /* Branch on bit-field parity */
        if (bf.flag) {
            /* Path 1: More bit manipulation */
            for (int i = 0; i < 4; i++) {
                mix.bytes[i] = (mix.bytes[i] + array[idx + i]) & 0xFF;
            }
            ll_var = ll_var - 0x5555555555555555LL;
        } else {
            /* Path 2: Different operations */
            mix.word = mix.word ^ array[idx];
            dbl_var = dbl_var - 1.0;
        }
        
        /* Branch on high/low word comparison */
        if (ll_high > ll_low) {
            array[idx] = array[idx] << (bf.mode + 1);
        } else if (ll_high < ll_low) {
            array[idx] = array[idx] >> (bf.data & 0x3);
        }
        
        /* Switch based on bit-field mode */
        switch (bf.mode) {
            case 0:
                result += array[idx];
                break;
            case 1:
                result += ll_low;
                break;
            case 2:
                result += (int)dbl_var;
                break;
            case 3:
                result += mix.word;
                break;
            default:
                result += bf.data;
                break;
        }
        
        /* Inline assembly to potentially influence register allocation */
        asm volatile (
            "/* Dummy assembly to affect resource tracking */"
            : 
            : "r" (ll_var), "r" (mix.word)
            : "memory"
        );
    }
    
    /* Final aggregation to prevent dead code elimination */
    int final_sum = result;
    for (int i = 0; i < 256; i += 16) {
        final_sum += array[i];
    }
    final_sum += (int)(ll_var >> 32) + (int)(ll_var & 0xFFFFFFFF);
    final_sum += (int)dbl_var;
    final_sum += mix.word;
    
    return final_sum;
}

/* Main driver */
int main(int argc, char **argv) {
    int result = process_resources(argc, argv);
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    
    return result & 0xFF;
}
