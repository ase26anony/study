/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag : 1;
    unsigned int mode : 3;
    unsigned int value : 12;
    unsigned int pad : 16;
} __attribute__((packed));

/* Union for type-punning and bit manipulation */
union data_union {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Function with inline assembly to force specific RTL patterns */
static inline uint32_t manipulate_bits(uint32_t x, uint32_t mask, int shift) {
    uint32_t result;
    /* Inline asm that might generate ZERO_EXTRACT-like patterns */
    __asm__ volatile (
        "andl %2, %1\n\t"
        "shrl %3, %1\n\t"
        "movl %1, %0"
        : "=r" (result)
        : "r" (x), "r" (mask), "i" (shift)
        : "cc"
    );
    return result;
}

/* Function to access misaligned data - may generate complex MEM addresses */
static uint32_t read_misaligned(const void *ptr) {
    uint32_t value;
    /* Force misaligned access through byte-wise copy */
    const unsigned char *bytes = (const unsigned char *)ptr;
    value = (bytes[0]) | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    return value;
}

int main(int argc, char *argv[]) {
    volatile struct bitfield_pack bf = {0};
    volatile union data_union du = {0};
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int i, j, result = 0;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    volatile int iterations = (argc > 1) ? (argc & 0xF) + 5 : 10;
    
    /* Main loop with mixed operations to generate complex RTL */
    for (i = 0; i < iterations; i++) {
        /* 1. Bit-field manipulations (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        bf.flag = i & 1;
        bf.mode = (i >> 1) & 0x7;
        bf.value = (bf.value + i * 7) & 0xFFF;
        
        /* Extract bit-field using mask and shift (may generate ZERO_EXTRACT) */
        uint32_t extracted = ((*(volatile uint32_t*)&bf) >> 4) & 0xFFF;
        
        /* 2. Multi-word operations (potential SUBREG generation) */
        /* Operations on long long - may be split into high/low parts on 32-bit */
        ll_var = ll_var + (extracted * 0x10001LL);
        ll_var = ll_var ^ ((long long)bf.value << 32);
        
        /* Compare high vs low parts (forces SUBREG usage) */
        int high_part = (int)(ll_var >> 32);
        int low_part = (int)(ll_var & 0xFFFFFFFF);
        
        /* 3. Complex memory addressing with bit-field derived index */
        /* Non-trivial array indexing */
        int idx = (bf.value * bf.mode + i) & 0xFF;
        array[idx] = array[idx] + extracted;
        
        /* More complex addressing with stride */
        int stride_idx = ((i * 13) + (bf.flag * 7)) & 0xFF;
        array[stride_idx] = array[(stride_idx + 1) & 0xFF] - array[(stride_idx + 2) & 0xFF];
        
        /* 4. Double operations (may use multiple registers/SUBREG) */
        dbl_var = dbl_var * 1.01 + (double)extracted / 1000.0;
        
        /* 5. Use inline assembly for bit manipulation */
        du.full = (uint32_t)ll_var;
        du.parts.low = manipulate_bits(du.parts.low, 0x0F0F, 4);
        
        /* 6. Conditional branching based on bit-field results */
        if (bf.flag) {
            /* Branch 1: More bit-field operations */
            bf.value = (bf.value >> 1) | ((bf.value & 1) << 11);
            ll_var = ll_var >> 1;
        } else {
            /* Branch 2: Different operations */
            bf.value = (bf.value << 1) & 0xFFF;
            ll_var = ll_var << 1;
        }
        
        /* 7. Strict low-part like operation using masking */
        /* This may generate STRICT_LOW_PART RTL */
        uint32_t temp = du.full;
        temp = (temp & 0xFFFF) | ((temp + 1) & 0xFFFF0000);
        du.full = temp;
        
        /* 8. Misaligned memory access (complex MEM address) */
        if (i & 2) {
            uint32_t misaligned = read_misaligned((const void*)(&array[idx] + 1));
            result += (misaligned & 0xFF);
        }
        
        /* 9. Switch statement for control flow variety */
        switch (bf.mode) {
            case 0:
                array[i & 0xFF] += high_part;
                break;
            case 1:
                array[i & 0xFF] -= low_part;
                break;
            case 2:
                array[i & 0xFF] ^= extracted;
                break;
            default:
                array[i & 0xFF] |= (high_part ^ low_part);
                break;
        }
    }
    
    /* Aggregate results to prevent optimization */
    for (j = 0; j < 256; j++) {
        result += array[j];
    }
    
    result += bf.value + (int)(ll_var >> 32) + (int)(ll_var & 0xFFFFFFFF);
    result += (int)(dbl_var * 1000);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result & 0xFF;
}
