/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int field3 : 3;
    unsigned int field12 : 12;
    unsigned int field16 : 16;
    unsigned int padding : 32 - (1+3+12+16);
} __attribute__((packed));

/* Union for type-punning to force complex memory accesses */
union type_pun {
    unsigned long long ull;
    double dbl;
    struct bitfield_struct bf;
    unsigned int words[2];
};

/* Complex addressing helper */
#define COMPLEX_INDEX(i, stride, offset) ((i) * (stride) + (offset))

/* Inline assembly to force specific register usage patterns */
static inline unsigned int get_low_part(unsigned long long val) {
    unsigned int low;
    /* This asm may generate STRICT_LOW_PART-like patterns */
    __asm__ volatile ("movl %1, %0" 
                      : "=r" (low) 
                      : "r" ((unsigned int)val)
                      : /* no clobber */);
    return low;
}

static inline unsigned int get_high_part(unsigned long long val) {
    unsigned int high;
    /* Access high part - may generate SUBREG patterns */
    __asm__ volatile ("movl %1, %0" 
                      : "=r" (high) 
                      : "r" ((unsigned int)(val >> 32))
                      : /* no clobber */);
    return high;
}

/* Main function with complex operations */
int main(int argc, char **argv) {
    volatile struct bitfield_struct bf_var = {0, 0, 0, 0, 0};
    volatile union type_pun pun_var;
    volatile unsigned long long ll_var = 0x123456789ABCDEF0ULL;
    volatile double dbl_var = 3.141592653589793;
    
    /* Array with complex access patterns */
    volatile unsigned int array[256];
    volatile int i, j;
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    unsigned int result = 0;
    
    /* Main loop with complex operations */
    for (i = 0; i < iterations; i++) {
        /* 1. Bit-field operations - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf_var.flag1 = (i & 1);                     /* Single bit assignment */
        bf_var.field3 = (i & 0x7);                  /* 3-bit field */
        bf_var.field12 = (i * 7) & 0xFFF;           /* 12-bit field */
        bf_var.field16 = (i * 13) & 0xFFFF;         /* 16-bit field */
        
        /* Extract using masks and shifts - forces ZERO_EXTRACT patterns */
        unsigned int extracted = (bf_var.field12 << 4) | (bf_var.field3 << 1) | bf_var.flag1;
        
        /* 2. Multi-word operations - may generate SUBREG patterns */
        ll_var += (unsigned long long)extracted * 0x100000001ULL;
        ll_var ^= (unsigned long long)i << 32;
        
        /* Double operations on 32-bit targets need multiple registers */
        dbl_var += (double)i * 0.01;
        dbl_var = dbl_var * 1.0001;
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = COMPLEX_INDEX(i, bf_var.field3 + 1, bf_var.field12 & 0xFF);
        idx = idx & 0xFF;  /* Ensure within bounds */
        
        /* Read-modify-write with complex addressing */
        array[idx] = (array[idx] + extracted) & 0xFFFF;
        
        /* 4. Type-punning through union - forces MEM with complex addressing */
        pun_var.ull = ll_var;
        pun_var.bf = bf_var;
        
        /* Access misaligned data through pointer arithmetic */
        unsigned char *byte_ptr = (unsigned char *)&pun_var;
        unsigned int word_from_bytes = 0;
        for (j = 0; j < 4; j++) {
            word_from_bytes |= (byte_ptr[j + 1] << (j * 8));
        }
        
        /* 5. Conditional control flow based on bit-field and multi-word results */
        if (bf_var.flag1) {
            /* When flag1 is set, operate on low part */
            unsigned int low = get_low_part(ll_var);
            array[(idx + 1) & 0xFF] ^= low;
        } else {
            /* When flag1 is clear, operate on high part */
            unsigned int high = get_high_part(ll_var);
            array[(idx + 2) & 0xFF] += high;
        }
        
        /* Compare high and low words of long long */
        unsigned int low_word = (unsigned int)ll_var;
        unsigned int high_word = (unsigned int)(ll_var >> 32);
        
        if (low_word > high_word) {
            /* Complex addressing with pointer arithmetic */
            unsigned int *ptr = (unsigned int *)&array[idx];
            *ptr = (*ptr & 0xFFFF0000) | (low_word & 0xFFFF);
        } else if (high_word > 0) {
            /* Another complex memory operation */
            array[(idx + 3) & 0xFF] = (array[(idx + 3) & 0xFF] & high_word) | (low_word & 0xFF);
        }
        
        /* Accumulate results */
        result += extracted + low_word + (unsigned int)dbl_var;
    }
    
    /* Final aggregation to prevent dead code elimination */
    unsigned int final_result = result;
    for (i = 0; i < 256; i += 16) {
        final_result ^= array[i];
    }
    
    /* Mix in bit-field values */
    final_result += bf_var.field12;
    final_result += (unsigned int)(ll_var & 0xFFFFFFFF);
    
    /* Print to prevent optimization */
    printf("Result: %u\n", final_result);
    
    return (int)(final_result & 0xFF);
}
