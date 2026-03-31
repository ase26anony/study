/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL expressions */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 3;    /* 3-bit field */
    unsigned int b : 12;   /* 12-bit field */
    unsigned int c : 1;    /* 1-bit field */
    unsigned int d : 8;    /* 8-bit field */
    unsigned int e : 8;    /* 8-bit field */
} __attribute__((packed));

/* Union for accessing full word and bit-fields */
union bitfield_union {
    struct bitfield_struct bits;
    volatile uint32_t full;
};

/* Function to force complex addressing modes */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Function with inline assembly to force specific patterns */
static void asm_bit_ops(volatile uint64_t *val) {
    /* Inline asm that operates on low 32 bits */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0xFFFF, %%eax\n\t"  /* Operate on low 16 bits */
        "movl %%eax, %0\n\t"
        : "=r" (*val)
        : "r" (*val)
        : "%eax", "cc"
    );
}

int main(int argc, char *argv[]) {
    volatile union bitfield_union bf = {0};
    volatile uint64_t ll_var = 0x123456789ABCDEF0ULL;
    volatile double dbl_var = 3.141592653589793;
    volatile int32_t array[256];
    volatile int loop_limit = (argc > 1) ? 100 : 50;
    int i, result = 0;
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Main loop with mixed operations */
    for (i = 0; i < loop_limit; i++) {
        /* 1. Bit-field operations for ZERO_EXTRACT/STRICT_LOW_PART */
        bf.bits.a = (i & 0x7);                     /* 3-bit field */
        bf.bits.b = (i * 7) & 0xFFF;               /* 12-bit field */
        bf.bits.c = (i & 0x1);                     /* 1-bit field */
        
        /* Extract and combine bit-fields using masks */
        uint32_t temp = bf.full;
        uint32_t extracted = (temp >> 4) & 0xFFF;  /* Extract 12-bit field */
        uint32_t masked = temp & 0x7;              /* Extract 3-bit field */
        
        /* 2. Multi-word operations for SUBREG generation */
        ll_var += (uint64_t)extracted * 0x10001ULL;
        ll_var ^= (uint64_t)masked << 32;          /* Mix high/low parts */
        
        /* Double operations (may use multiple registers) */
        dbl_var = dbl_var * 1.01 + (double)(i & 0xF);
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = complex_index(i, 7, bf.bits.b & 0xFF);
        if (idx >= 0 && idx < 256) {
            /* Read-modify-write with bit manipulation */
            array[idx] = (array[idx] & 0xFFFF0000) | (extracted & 0xFFFF);
            array[idx] ^= (masked << 16);
        }
        
        /* 4. Conditional based on bit-field and multi-word comparisons */
        if (bf.bits.c) {  /* Based on 1-bit field */
            /* Compare high vs low word of 64-bit variable */
            uint32_t low_word = (uint32_t)(ll_var & 0xFFFFFFFF);
            uint32_t high_word = (uint32_t)(ll_var >> 32);
            
            if (low_word > high_word) {
                /* Use inline assembly for bit operations */
                asm_bit_ops(&ll_var);
                
                /* Access array with different pattern */
                int alt_idx = complex_index(i, 3, low_word & 0x7F);
                if (alt_idx >= 0 && alt_idx < 256) {
                    array[alt_idx] += high_word;
                }
            } else {
                /* Different bit-field manipulation */
                bf.bits.d = (low_word >> 8) & 0xFF;
                bf.bits.e = (high_word >> 16) & 0xFF;
                
                /* Combine bit-fields */
                uint32_t combined = (bf.bits.d << 8) | bf.bits.e;
                ll_var |= (uint64_t)combined << 24;
            }
        }
        
        /* Force memory barrier to prevent optimization */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (i = 0; i < 256; i++) {
        result += array[i] & 0xFF;
    }
    
    result += (int)(bf.full & 0xFFFF);
    result += (int)(ll_var & 0xFFFF);
    result += (int)(ll_var >> 48);
    result += (int)(dbl_var * 1000);
    
    /* Use result to affect return value */
    return result & 0xFF;
}
