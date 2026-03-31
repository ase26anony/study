/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 12;
    unsigned int pad : 16;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union data_union {
    struct bitfield_pack bf;
    uint32_t raw;
    uint16_t halves[2];
};

/* Function with complex operations to generate target RTL patterns */
int process_resources(int argc, char **argv) {
    volatile struct bitfield_pack bf_data = {0};
    volatile union data_union data_union = {0};
    
    /* Multi-word operations for SUBREG generation */
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    
    /* Array with complex addressing */
    volatile int32_t data_array[256];
    for (int i = 0; i < 256; i++) {
        data_array[i] = i * 3;
    }
    
    /* Loop with volatile counter to prevent optimization */
    volatile int loop_limit = (argc > 1) ? 10 : 5;
    int result = 0;
    
    for (volatile int i = 0; i < loop_limit; i++) {
        /* ===== BIT-FIELD OPERATIONS ===== */
        /* These should generate ZERO_EXTRACT/STRICT_LOW_PART */
        
        /* Assignment to bit-field - potential STRICT_LOW_PART */
        bf_data.flag1 = (i & 1);
        bf_data.flag2 = (i & 7);
        bf_data.value = (i * 13) & 0xFFF;
        
        /* Extract bit-field using masking - potential ZERO_EXTRACT */
        uint32_t extracted = (data_union.raw >> 4) & 0xFFF;  /* Extract 12-bit field */
        
        /* Complex bit-field manipulation */
        data_union.bf.flag1 = extracted & 1;
        data_union.bf.flag2 = (extracted >> 1) & 7;
        
        /* ===== MULTI-WORD OPERATIONS ===== */
        /* These should generate SUBREG expressions on 32-bit targets */
        
        /* Operations on long long - may be split into high/low parts */
        ll_var = ll_var + (long long)(i * 0x10001);
        ll_var = ll_var ^ (0xF0F0F0F0F0F0F0F0LL >> (i & 7));
        
        /* Access high and low parts separately */
        uint32_t ll_low = (uint32_t)(ll_var & 0xFFFFFFFF);
        uint32_t ll_high = (uint32_t)(ll_var >> 32);
        
        /* Double operations - may use multiple registers */
        dbl_var = dbl_var * 1.01 + (double)i;
        
        /* ===== COMPLEX MEMORY ADDRESSING ===== */
        /* Generate MEM with complex address expressions */
        
        /* Non-trivial array indexing */
        int idx = ((data_union.bf.value * i) + 17) & 0xFF;
        
        /* Read-modify-write with bit operations */
        data_array[idx] = data_array[idx] | (1 << (i & 0x1F));
        
        /* Misaligned access simulation */
        uint8_t *byte_ptr = (uint8_t *)&data_array[idx];
        uint32_t misaligned_read = *(uint32_t *)(byte_ptr + 1);  /* Potential unaligned */
        
        /* ===== CONTROL FLOW ===== */
        /* Conditional based on bit-field and multi-word results */
        
        if ((data_union.bf.flag1) || (ll_low > ll_high)) {
            /* Branch 1: More bit-field operations */
            data_union.bf.value = (data_union.bf.value + misaligned_read) & 0xFFF;
            
            /* Inline assembly to hint at register constraints */
            asm volatile (
                "/* asm comment: operating on low part */"
                : "+r" (ll_low)
                : "r" (data_union.bf.value)
                : "cc"
            );
        } else {
            /* Branch 2: Different operations */
            data_array[idx] = data_array[idx] ^ 0x55555555;
            
            /* Another inline asm hint */
            asm volatile (
                "/* asm comment: double operation */"
                : "+m" (dbl_var)
                :
                : "memory"
            );
        }
        
        /* Switch based on bit-field value */
        switch (data_union.bf.flag2) {
            case 0:
                ll_var = ll_var << 1;
                break;
            case 1:
                ll_var = ll_var >> 1;
                break;
            case 2:
                dbl_var = dbl_var / 2.0;
                break;
            default:
                /* Complex addressing with pointer arithmetic */
                int *ptr = &data_array[(i * 7) & 0xFF];
                for (int j = 0; j < 4; j++) {
                    ptr[j] = ptr[j] + (j * data_union.bf.value);
                }
                break;
        }
        
        /* Accumulate result */
        result += data_union.bf.value + (ll_low & 0xFF) + (int)dbl_var;
    }
    
    /* Final aggregation to prevent dead code elimination */
    int final_sum = result;
    for (int i = 0; i < 256; i += 16) {
        final_sum += data_array[i];
    }
    final_sum += bf_data.value + (int)(ll_var & 0xFFFFFFFF);
    
    return final_sum;
}

/* Main driver */
int main(int argc, char **argv) {
    int result = process_resources(argc, argv);
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
