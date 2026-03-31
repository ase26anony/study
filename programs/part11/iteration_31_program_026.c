/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields of varying widths to trigger ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int flag : 1;      /* 1-bit field */
    unsigned int mode : 3;      /* 3-bit field */
    unsigned int value : 12;    /* 12-bit field */
    unsigned int pad : 16;      /* padding to 32 bits */
} __attribute__((packed));

/* Union for accessing the same memory as bit-fields or raw integer */
union bitfield_union {
    struct bitfield_struct bits;
    volatile unsigned int raw;
};

/* Function to force complex addressing modes */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Main function with operations designed to generate target RTL patterns */
int main(int argc, char *argv[]) {
    volatile union bitfield_union bf;
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int temp_result = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Initialize bit-field structure */
    bf.bits.flag = 1;
    bf.bits.mode = 5;
    bf.bits.value = 2047;
    
    /* Main loop with operations designed to trigger specific RTL patterns */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* ============================================
         * Operations to trigger ZERO_EXTRACT RTL
         * ============================================ */
        
        /* Extract bit-field using mask and shift - may generate ZERO_EXTRACT */
        unsigned int extracted = (bf.raw >> 4) & 0xFFF;  /* Extract 12-bit value field */
        
        /* Direct bit-field assignment - may generate STRICT_LOW_PART */
        bf.bits.mode = (extracted & 0x7);  /* Assign to 3-bit field */
        
        /* Complex bit-field manipulation */
        bf.bits.value = (bf.bits.value + i) & 0xFFF;  /* 12-bit field arithmetic */
        
        /* Toggle flag based on extracted value */
        bf.bits.flag = (extracted & 1);
        
        /* ============================================
         * Operations to trigger SUBREG RTL for multi-word types
         * ============================================ */
        
        /* Operations on long long - may generate SUBREG for high/low parts */
        ll_var = ll_var + 0x100000001LL;  /* Add to both high and low 32-bit parts */
        
        /* Bitwise operations that work on partial words */
        ll_var = ll_var | 0x00000000FFFFFFFFLL;  /* Set low 32 bits */
        ll_var = ll_var & 0xFFFFFFFF00000000LL;  /* Clear low 32 bits */
        
        /* Double precision operations - may use multiple registers */
        dbl_var = dbl_var * 2.0 - 1.0;
        
        /* Mix long long and double through integer conversion */
        if ((i & 3) == 0) {
            ll_var = (long long)(dbl_var * 1000000.0);
        }
        
        /* ============================================
         * Operations to trigger complex MEM addressing
         * ============================================ */
        
        /* Complex array indexing - may generate MEM with complex address */
        int idx = complex_index(i, 7, bf.bits.value & 0x1F);
        idx = idx & 0xFF;  /* Ensure within bounds */
        
        /* Read-modify-write with bit-field extraction from memory */
        array[idx] = (array[idx] & ~0xFFF) | (bf.bits.value & 0xFFF);
        
        /* Access with pointer arithmetic and casting */
        volatile int *ptr = &array[idx];
        *ptr = *ptr + ((bf.raw >> 16) & 0xFFFF);
        
        /* ============================================
         * Control flow based on bit-field and multi-word operations
         * ============================================ */
        
        /* Branch based on bit-field parity */
        if (bf.bits.flag) {
            /* When flag is 1, operate on high word of long long */
            unsigned int high_word = (unsigned int)(ll_var >> 32);
            array[idx] = array[idx] ^ high_word;
        } else {
            /* When flag is 0, operate on low word of long long */
            unsigned int low_word = (unsigned int)(ll_var & 0xFFFFFFFF);
            array[idx] = array[idx] + low_word;
        }
        
        /* Branch based on comparison of high vs low words */
        unsigned int high = (unsigned int)(ll_var >> 32);
        unsigned int low = (unsigned int)(ll_var & 0xFFFFFFFF);
        
        if (high > low) {
            /* Use inline assembly to force specific register usage */
            __asm__ volatile (
                "addl %1, %0\n\t"
                : "+r" (array[idx])
                : "r" (bf.bits.mode)
                : "cc"
            );
        } else if (high < low) {
            /* Another inline asm with different constraints */
            __asm__ volatile (
                "subl %1, %0\n\t"
                : "+r" (array[idx])
                : "r" (bf.bits.value)
                : "cc"
            );
        } else {
            /* Complex bit manipulation when equal */
            bf.raw = (bf.raw << 1) | (bf.raw >> 31);  /* Rotate right */
        }
        
        /* Accumulate results to prevent optimization */
        temp_result += array[idx] + bf.bits.value + (int)(ll_var & 0xFF);
    }
    
    /* Additional operations outside loop to ensure all code paths are used */
    
    /* Force SUBREG generation with 64-bit operations on result */
    long long final_ll = (long long)temp_result * 0x100000001LL;
    
    /* Final bit-field extraction and manipulation */
    unsigned int final_bits = bf.raw;
    final_bits = ((final_bits & 0x0000FFFF) << 16) | ((final_bits & 0xFFFF0000) >> 16);
    
    /* Complex memory access pattern */
    int sum = 0;
    for (int i = 0; i < 128; i += 2) {
        /* Access with stride and offset */
        sum += array[complex_index(i, 3, final_bits & 0x3F) & 0xFF];
    }
    
    /* Mix all results */
    int final_result = temp_result + (int)(final_ll & 0xFFFFFFFF) + 
                      (int)(final_ll >> 32) + sum + final_bits;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF;  /* Return non-zero to indicate success */
}
