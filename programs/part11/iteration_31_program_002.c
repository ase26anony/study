/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL expressions */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields of varying widths to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int small : 3;
    unsigned int medium : 12;
    unsigned int large : 16;
} __attribute__((packed));

/* Union to force specific bit manipulations */
union bit_manipulator {
    struct bitfield_pack bits;
    unsigned int raw;
    volatile unsigned int vraw;
};

/* Function to create complex addressing patterns */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Main function with operations designed to generate target RTL patterns */
int main(int argc, char **argv) {
    volatile struct bitfield_pack bf = {0};
    volatile union bit_manipulator um = {0};
    
    /* Multi-word types for SUBREG generation */
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    
    /* Array for complex memory addressing */
    volatile int arr[256];
    for (int i = 0; i < 256; i++) {
        arr[i] = i;
    }
    
    /* Volatile loop control to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    int result = 0;
    
    /* Main loop with mixed operations */
    for (volatile int i = 0; i < iterations; i++) {
        /* ========== BIT-FIELD MANIPULATIONS ========== */
        /* These should generate ZERO_EXTRACT and STRICT_LOW_PART */
        
        /* Direct bit-field assignment */
        bf.flag1 = i & 1;
        bf.small = (i >> 1) & 0x7;
        bf.medium = (i * 3) & 0xFFF;
        bf.large = (i * 7) & 0xFFFF;
        
        /* Extract via bit-field reference */
        unsigned int temp = bf.medium;
        
        /* Manual bit extraction using shifts and masks (alternative path) */
        um.raw = (i * 11) & 0xFFFFFFFF;
        unsigned int extracted = (um.vraw >> 4) & 0xFFF;  /* ZERO_EXTRACT candidate */
        
        /* Combine bit-fields with masking */
        um.bits.small = (um.bits.small + bf.small) & 0x7;
        um.bits.medium = (um.bits.medium ^ temp) & 0xFFF;
        
        /* ========== MULTI-WORD OPERATIONS ========== */
        /* These should generate SUBREG expressions on 32-bit targets */
        
        /* Operations on long long (multi-register on 32-bit) */
        ll_var = ll_var + (long long)(i * 0x10001LL);
        ll_var = ll_var | ((long long)extracted << 32);
        
        /* Bitwise operations that might split into high/low parts */
        long long ll_mask = 0x00000000FFFFFFFFLL;
        ll_var = (ll_var & ~ll_mask) | ((ll_var + i) & ll_mask);
        
        /* Double operations (also multi-word) */
        dbl_var = dbl_var * 1.01 + (double)(i & 0xF);
        
        /* ========== COMPLEX MEMORY ADDRESSING ========== */
        /* Create complex address expressions */
        
        /* Non-trivial array indexing */
        int idx = complex_index(i & 0xF, 7, 3) % 256;
        
        /* Read-modify-write with bit manipulation */
        int arr_val = arr[idx];
        
        /* Operation that might create ZERO_EXTRACT from memory */
        arr_val = (arr_val & ~0xFF) | (extracted & 0xFF);
        arr_val = (arr_val ^ (i << 8)) & 0xFFFF;
        
        /* Write back with potential STRICT_LOW_PART */
        arr[idx] = arr_val;
        
        /* Misaligned access simulation via pointer casting */
        if (i & 1) {
            char *byte_ptr = (char *)&arr[idx];
            int *misaligned_int = (int *)(byte_ptr + 1);
            /* volatile read to force generation */
            volatile int misaligned_read = *misaligned_int;
            (void)misaligned_read; /* Suppress unused warning */
        }
        
        /* ========== CONTROL FLOW ========== */
        /* Conditional based on bit-field and multi-word values */
        
        /* Check parity of bit-field */
        if (bf.flag1) {
            /* When flag1 is set, use different operations */
            ll_var = ll_var - (long long)(bf.medium << 16);
            arr[idx] = arr[idx] * 2;
        } else {
            /* Otherwise use these operations */
            ll_var = ll_var + (long long)(bf.small * 0x1000000LL);
            arr[idx] = arr[idx] / 2;
        }
        
        /* Compare high vs low parts of long long */
        unsigned int low_part = (unsigned int)(ll_var & 0xFFFFFFFF);
        unsigned int high_part = (unsigned int)(ll_var >> 32);
        
        if (high_part > low_part) {
            /* Swap operations when high part is greater */
            unsigned int swap_temp = um.bits.medium;
            um.bits.medium = um.bits.large & 0xFFF;
            um.bits.large = (um.bits.large & 0xFFFF0000) | (swap_temp << 4);
        }
        
        /* Switch based on small bit-field */
        switch (bf.small & 0x3) {
            case 0:
                dbl_var += 0.5;
                break;
            case 1:
                dbl_var -= 0.25;
                break;
            case 2:
                dbl_var *= 0.99;
                break;
            case 3:
                dbl_var = -dbl_var;
                break;
        }
        
        /* Accumulate results */
        result += arr[idx];
        result += (int)(ll_var & 0xFF);
        result += bf.small;
    }
    
    /* Final aggregation to prevent dead code elimination */
    result += um.bits.medium;
    result += (int)(dbl_var * 1000);
    
    /* Use inline assembly to potentially generate specific patterns */
    /* This asm operates on low 32 bits of ll_var, potentially creating STRICT_LOW_PART */
    unsigned int low_ll;
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl $0x1234, %0\n\t"
        : "=r" (low_ll)
        : "m" (*(volatile unsigned int *)&ll_var)
        : "cc"
    );
    
    result += low_ll;
    
    /* Another asm that might generate ZERO_EXTRACT-like behavior */
    unsigned int extracted_bits;
    __asm__ volatile (
        "movl %1, %0\n\t"
        "shrl $8, %0\n\t"
        "andl $0xFFF, %0\n\t"
        : "=r" (extracted_bits)
        : "m" (um.raw)
        : "cc"
    );
    
    result += extracted_bits;
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
