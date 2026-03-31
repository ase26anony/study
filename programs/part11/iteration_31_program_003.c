/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int small : 3;
    unsigned int medium : 12;
    unsigned int large : 16;
    unsigned int flag2 : 1;
} __attribute__((packed));

/* Union for type-punning to force complex memory operations */
union type_pun {
    unsigned long long ull;
    double dbl;
    struct {
        unsigned int low;
        unsigned int high;
    } words;
};

/* Volatile variables to prevent optimization */
volatile struct bitfield_pack bf;
volatile unsigned long long multi_word;
volatile double fp_multi_word;
volatile int array[256];
volatile int *volatile ptr_array;

/* Function with inline assembly to force specific patterns */
static void asm_bit_ops(unsigned int *val, unsigned int mask) {
    /* Inline asm that might generate STRICT_LOW_PART-like operations */
    __asm__ volatile (
        "andl %1, %0\n\t"
        "orl  %1, %0\n\t"
        : "+r" (*val)
        : "r" (mask)
        : "cc"
    );
}

/* Function to access array with complex addressing */
static int complex_array_access(int idx, int stride) {
    /* Complex addressing mode that might generate MEM with complex XEXP */
    return array[(idx * stride + 7) & 0xFF];
}

int main(int argc, char **argv) {
    volatile int i, j, limit;
    volatile unsigned int temp;
    volatile union type_pun pun;
    volatile long long result = 0;
    
    /* Use argc to make execution data-dependent */
    limit = (argc > 1) ? (atoi(argv[1]) & 0xF) + 5 : 10;
    
    /* Initialize variables */
    bf.flag1 = 1;
    bf.small = 5;
    bf.medium = 1234;
    bf.large = 45678;
    bf.flag2 = 0;
    
    multi_word = 0x123456789ABCDEF0ULL;
    fp_multi_word = 3.141592653589793;
    
    for (i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    ptr_array = (volatile int *)array;
    
    /* Main loop with mixed operations */
    for (i = 0; i < limit; i++) {
        /* 1. Bit-field operations that may generate ZERO_EXTRACT/STRICT_LOW_PART */
        
        /* Extract and manipulate bit-fields */
        temp = bf.medium;
        
        /* This assignment might generate STRICT_LOW_PART */
        bf.small = (temp >> 4) & 0x7;
        
        /* Complex bit-field combination */
        bf.flag1 = (bf.large >> 8) & 1;
        
        /* Masked assignment that could be ZERO_EXTRACT */
        bf.medium = (bf.medium & 0xF0F) | ((i << 4) & 0x0F0);
        
        /* 2. Multi-word operations that may generate SUBREG */
        
        /* Operations on long long (multi-register on 32-bit) */
        multi_word += (long long)bf.medium * 0x10001ULL;
        
        /* Bitwise operations that work on parts */
        multi_word = (multi_word << 16) | (multi_word >> 48);
        
        /* Double precision operations */
        fp_multi_word += (double)i * 0.1;
        
        /* Type punning through union */
        pun.dbl = fp_multi_word;
        pun.words.low ^= pun.words.high;
        
        /* 3. Complex memory addressing */
        
        /* Array access with complex index calculation */
        j = complex_array_access(i, bf.small + 2);
        
        /* Pointer arithmetic with bit-field derived offset */
        temp = ptr_array[(bf.medium & 0x3F) + i];
        
        /* Read-modify-write with masking */
        array[(i * 13 + 7) & 0xFF] = (temp & 0xFF00FF) | (j << 8);
        
        /* 4. Conditional control flow based on bit operations */
        
        if (bf.flag1 ^ (bf.large & 1)) {
            /* Branch 1: More bit-field manipulations */
            bf.large = (bf.large + multi_word) & 0xFFFF;
            
            /* Inline assembly for explicit low-part operations */
            asm_bit_ops((unsigned int *)&temp, bf.medium);
            
            /* Access misaligned data through pointer casting */
            if (i & 1) {
                char *byte_ptr = (char *)&multi_word;
                byte_ptr[1] = byte_ptr[3] ^ byte_ptr[5];
            }
        } else {
            /* Branch 2: Different set of operations */
            /* Check high vs low word of long long */
            if ((multi_word >> 32) > (multi_word & 0xFFFFFFFF)) {
                bf.flag2 = 1;
                
                /* Force SUBREG for double word access */
                unsigned int low_part = (unsigned int)multi_word;
                unsigned int high_part = (unsigned int)(multi_word >> 32);
                multi_word = ((long long)high_part << 32) | (low_part ^ 0xAAAA);
            }
            
            /* Complex array update */
            int idx = (i * bf.small) & 0xFF;
            array[idx] = array[idx] + (int)(fp_multi_word * 100.0);
        }
        
        /* Switch based on bit-field value */
        switch (bf.small & 0x3) {
            case 0:
                multi_word -= 0x100000000ULL;
                break;
            case 1:
                fp_multi_word *= 1.01;
                break;
            case 2:
                bf.medium = (bf.medium << 1) | bf.flag1;
                break;
            case 3:
                /* Mixed operation */
                result += (long long)array[i & 0xFF] * bf.large;
                break;
        }
    }
    
    /* Aggregate results to prevent elimination */
    unsigned int final_result = 0;
    final_result += bf.flag1;
    final_result += bf.small << 1;
    final_result += bf.medium << 4;
    final_result += bf.large << 8;
    final_result += (unsigned int)(result & 0xFFFFFFFF);
    final_result += (unsigned int)((result >> 32) & 0xFFFFFFFF);
    
    for (i = 0; i < 16; i++) {
        final_result ^= array[i * 16];
    }
    
    /* Print to prevent dead code elimination */
    printf("Result: %u\n", final_result);
    
    return (int)(final_result & 0xFF);
}
