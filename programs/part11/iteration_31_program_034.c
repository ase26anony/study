/* test_resource.c - Designed to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int small : 3;
    unsigned int medium : 12;
    unsigned int large : 16;
} __attribute__((packed));

/* Union for mixed-type access */
union mixed_data {
    unsigned long long ull;
    double dbl;
    struct bitfield_pack bf;
    unsigned int words[2];
};

/* Function to force complex addressing */
static inline unsigned int complex_index(volatile unsigned int *arr, 
                                         int idx, int stride) {
    return arr[idx * stride + 3];  /* Non-trivial addressing */
}

/* Main driver function */
int main(int argc, char **argv) {
    volatile struct bitfield_pack bf_var = {0};
    volatile union mixed_data mix_var = {0};
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    
    /* Array with volatile elements for complex memory addressing */
    #define ARRAY_SIZE 64
    volatile unsigned int data_array[ARRAY_SIZE];
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data_array[i] = i * 7 + 1;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    int sum_result = 0;
    
    /* Main loop with mixed operations */
    for (volatile int i = 0; i < iterations; i++) {
        /* ===== BIT-FIELD OPERATIONS ===== */
        /* These may generate ZERO_EXTRACT/STRICT_LOW_PART RTL */
        
        /* Assignment to bit-field - potential STRICT_LOW_PART */
        bf_var.flag1 = (i & 1);           /* Single bit assignment */
        bf_var.small = (i & 0x7);         /* 3-bit field */
        bf_var.medium = (i * 13) & 0xFFF; /* 12-bit field */
        bf_var.large = (i * 17) & 0xFFFF; /* 16-bit field */
        
        /* Extract and combine bit-fields - potential ZERO_EXTRACT */
        unsigned int extracted = 0;
        extracted |= (bf_var.flag1 << 0);
        extracted |= (bf_var.small << 1);
        extracted |= (bf_var.medium << 4);
        extracted |= (bf_var.large << 16);
        
        /* Manual bit manipulation mimicking bit-field extract */
        unsigned int mask = (1 << 12) - 1;  /* 12-bit mask */
        unsigned int masked = (extracted >> 4) & mask;  /* Like ZERO_EXTRACT */
        
        /* ===== MULTI-WORD OPERATIONS ===== */
        /* These may generate SUBREG RTL on 32-bit targets */
        
        /* Operations on long long - may be split into SUBREGs */
        ll_var = ll_var + (0x100000001LL * i);
        ll_var = ll_var | (0x00000000FFFFFFFFLL);
        ll_var = ll_var ^ (0x5555555555555555LL);
        
        /* Access high and low parts separately */
        unsigned int low_part = (unsigned int)(ll_var & 0xFFFFFFFF);
        unsigned int high_part = (unsigned int)(ll_var >> 32);
        
        /* Double operations - also multi-word on 32-bit */
        dbl_var = dbl_var * 1.01 + (double)i;
        mix_var.dbl = dbl_var;  /* Store in union */
        
        /* ===== COMPLEX MEMORY ADDRESSING ===== */
        /* May generate MEM with complex address expressions */
        
        /* Access array with complex indexing */
        int idx = (i * 11 + masked) % (ARRAY_SIZE / 4);
        unsigned int val = complex_index(data_array, idx, 4);
        
        /* Read-modify-write with bit manipulation */
        data_array[idx * 4 + 3] = (val & ~0xFF) | (extracted & 0xFF);
        
        /* Misaligned access simulation via pointer arithmetic */
        unsigned char *byte_ptr = (unsigned char *)&data_array[idx];
        unsigned int word = *(unsigned int *)(byte_ptr + 1);  /* Potential unaligned */
        
        /* ===== CONTROL FLOW BASED ON OPERATIONS ===== */
        /* Forces analysis of all paths */
        
        if (bf_var.flag1) {
            /* Branch 1: More bit-field operations */
            bf_var.large = bf_var.large ^ bf_var.medium;
            sum_result += bf_var.small;
        } else {
            /* Branch 2: More multi-word operations */
            ll_var = ll_var >> 1;
            sum_result += low_part & 0xF;
        }
        
        /* Switch based on extracted bits */
        switch (masked & 0x7) {  /* Use lowest 3 bits */
            case 0:
                dbl_var += 1.0;
                break;
            case 1:
                ll_var += high_part;
                break;
            case 2:
                bf_var.medium = (bf_var.medium * 3) & 0xFFF;
                break;
            default:
                /* Complex array update */
                data_array[(i * 7) % ARRAY_SIZE] = word;
                break;
        }
        
        /* Compare high vs low parts of long long */
        if (high_part > low_part) {
            /* Potential for SUBREG comparisons */
            ll_var = ll_var - ((long long)high_part << 32);
        } else if (high_part == low_part) {
            bf_var.flag1 = !bf_var.flag1;
        }
        
        /* Accumulate results */
        sum_result += (int)(val & 0xFF);
        sum_result += (int)(ll_var & 0xFF);
        sum_result += (int)(dbl_var * 100) % 256;
    }
    
    /* Final aggregation to prevent dead code elimination */
    unsigned int final_result = 
        (bf_var.flag1 << 0) |
        (bf_var.small << 1) |
        (bf_var.medium << 4) |
        (bf_var.large << 16) |
        (sum_result & 0xFF) << 24;
    
    /* Use inline assembly to potentially generate specific patterns */
    /* This asm operates on low 32 bits of ll_var, leaving high bits "constrained" */
    unsigned int asm_result;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0xFFFFF, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (asm_result)
        : "r" ((unsigned int)(ll_var & 0xFFFFFFFF))
        : "%eax"
    );
    
    final_result ^= asm_result;
    
    /* Also force memory barrier */
    __asm__ volatile ("" : : : "memory");
    
    /* Print to prevent optimization */
    printf("Result: 0x%08X\n", final_result);
    
    return (final_result & 0xFF);
}
