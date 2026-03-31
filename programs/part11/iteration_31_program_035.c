/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT and STRICT_LOW_PART */
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
    volatile unsigned int full;
};

/* Function to force complex addressing modes */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Inline assembly to mimic STRICT_LOW_PART operations */
static inline unsigned int asm_low_part(unsigned long long val) {
    unsigned int result;
    /* Operate on low 32 bits, implicitly preserving high bits */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl $0x1234, %0"
        : "=r" (result)
        : "r" ((unsigned int)val)
        : "cc"
    );
    return result;
}

int main(int argc, char *argv[]) {
    volatile int i, j;
    volatile union bitfield_union bf;
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int temp_results[4] = {0};
    int final_result = 0;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Initialize bit-field structure */
    bf.bits.a = 5;
    bf.bits.b = 2047;
    bf.bits.c = 1;
    bf.bits.d = 0x55;
    bf.bits.e = 0xAA;
    
    /* Loop controlled by volatile variable to prevent optimization */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 10) loop_limit = 10;
    if (loop_limit > 1000) loop_limit = 1000;
    
    for (i = 0; i < loop_limit; i++) {
        /* ===== BIT-FIELD OPERATIONS (ZERO_EXTRACT/STRICT_LOW_PART) ===== */
        
        /* 1. Direct bit-field assignment - may generate STRICT_LOW_PART */
        bf.bits.a = (bf.bits.a + i) & 0x7;  /* Keep within 3 bits */
        
        /* 2. Bit-field extraction with masking - may generate ZERO_EXTRACT */
        unsigned int extracted = (bf.full >> 3) & 0xFFF;  /* Extract 12-bit field b */
        
        /* 3. Combined bit-field operations */
        bf.bits.d = (bf.bits.d ^ extracted) & 0xFF;
        bf.bits.e = (bf.bits.e + bf.bits.a) & 0xFF;
        
        /* 4. Complex bit-field manipulation */
        if (bf.bits.c) {
            /* Toggle bit-field c based on extracted value */
            bf.bits.c = (extracted & 1) ^ 1;
        }
        
        /* ===== MULTI-WORD OPERATIONS (SUBREG generation) ===== */
        
        /* 1. Long long operations on 32-bit target */
        ll_var = ll_var + 0x100000001LL;  /* Add to both high and low parts */
        
        /* 2. Double precision operations */
        dbl_var = dbl_var * 1.01 - 0.5;
        
        /* 3. Compare high vs low word of long long */
        unsigned int low_word = (unsigned int)ll_var;
        unsigned int high_word = (unsigned int)(ll_var >> 32);
        
        /* 4. Cross-type operations that may require SUBREG */
        if (low_word > high_word) {
            /* Use inline assembly that operates on low part */
            low_word = asm_low_part(ll_var);
            ll_var = ((long long)high_word << 32) | low_word;
        }
        
        /* ===== COMPLEX MEMORY ADDRESSING ===== */
        
        /* 1. Array access with complex indexing */
        int idx = complex_index(i, 7, bf.bits.a);
        idx = idx & 0xFF;  /* Ensure within bounds */
        
        /* 2. Read-modify-write with bit-field influence */
        array[idx] = array[idx] + (bf.bits.b & 0x3F);
        
        /* 3. Access with pointer arithmetic and casting */
        volatile char *byte_ptr = (volatile char *)&array[idx];
        int byte_sum = 0;
        for (j = 0; j < 4; j++) {
            byte_sum += byte_ptr[j];
        }
        
        /* 4. Misaligned access simulation */
        if (idx % 4 != 0) {
            volatile int *misaligned = (volatile int *)(byte_ptr + 1);
            /* This may generate complex MEM expressions */
            *misaligned = *misaligned ^ 0x00FF00FF;
        }
        
        /* ===== CONTROL FLOW BASED ON OPERATIONS ===== */
        
        /* Branch based on bit-field parity */
        if (bf.bits.a & 1) {
            /* Path 1: More bit-field manipulations */
            bf.bits.b = (bf.bits.b << 1) | (bf.bits.b >> 11);
            bf.bits.b &= 0xFFF;  /* Keep within 12 bits */
            
            /* Complex array update */
            int idx2 = (i * 13 + bf.bits.d) & 0xFF;
            array[idx2] = array[idx2] - byte_sum;
        } else {
            /* Path 2: Different operations */
            bf.bits.e = bf.bits.e ^ bf.bits.d;
            
            /* Update array with different pattern */
            int idx3 = (i * 17 + bf.bits.e) & 0xFF;
            array[idx3] = array[idx3] | 0x5500;
        }
        
        /* Switch based on multiple conditions */
        switch ((bf.full >> 16) & 0x3) {
            case 0:
                temp_results[0] += low_word & 0xF;
                break;
            case 1:
                temp_results[1] += high_word & 0xF;
                break;
            case 2:
                temp_results[2] += bf.bits.a;
                break;
            case 3:
                temp_results[3] += bf.bits.b & 0xF;
                break;
        }
        
        /* Prevent loop unrolling */
        volatile int dummy;
        __asm__ volatile ("" : "=r" (dummy) : "0" (i));
    }
    
    /* ===== AGGREGATE RESULTS ===== */
    
    /* Sum array elements */
    for (i = 0; i < 256; i++) {
        final_result += array[i];
    }
    
    /* Incorporate bit-field values */
    final_result += bf.bits.a;
    final_result += bf.bits.b;
    final_result += bf.bits.c * 1000;
    final_result += bf.bits.d;
    final_result += bf.bits.e;
    
    /* Incorporate temp results */
    for (i = 0; i < 4; i++) {
        final_result += temp_results[i];
    }
    
    /* Incorporate parts of long long */
    final_result += (int)(ll_var & 0xFFFFFFFF);
    final_result += (int)(ll_var >> 32);
    
    /* Print to prevent dead code elimination */
    printf("Final result: %d\n", final_result);
    
    return final_result & 0xFF;
}
