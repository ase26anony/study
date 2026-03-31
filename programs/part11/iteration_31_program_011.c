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
    unsigned int pad : 31;  /* Force alignment for 64-bit operations */
} __attribute__((packed));

/* Union for type-punning to access specific bits */
union bit_access {
    unsigned long long full;
    struct {
        unsigned int low;
        unsigned int high;
    } parts;
    struct bitfield_pack bits;
};

/* Function to force complex addressing modes */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile struct bitfield_pack bf = {0};
    volatile union bit_access accessor = {0};
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int temp_result = 0;
    int i, j;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Main loop with mixed operations */
    for (i = 0; i < loop_limit; i++) {
        /* 1. Bit-field operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.flag1 = i & 1;
        bf.small = (i >> 1) & 0x7;
        bf.medium = (i * 7) & 0xFFF;
        bf.large = (i * 13) & 0xFFFF;
        bf.flag2 = (i >> 2) & 1;
        
        /* Manual bit extraction using shifts and masks (may generate ZERO_EXTRACT) */
        unsigned int extracted = ((unsigned int)bf.medium << 4) | bf.small;
        
        /* 2. Multi-word operations to generate SUBREG patterns */
        /* Operations on long long (64-bit) on 32-bit targets */
        ll_var = ll_var + (extracted * 0x10001LL);
        ll_var = ll_var ^ (0xF0F0F0F0F0F0F0F0LL >> bf.small);
        ll_var = ll_var | ((long long)bf.large << 32);
        
        /* Double operations that may use multiple registers */
        dbl_var = dbl_var * 1.01 + (double)extracted / 1000.0;
        
        /* 3. Complex memory addressing with bit-field derived index */
        /* This may create MEM with complex address expressions */
        int idx = complex_index(i & 0xF, 13, bf.medium & 0xFF);
        if (idx >= 0 && idx < 256) {
            /* Read-modify-write with bit manipulation */
            array[idx] = (array[idx] & ~0xFFF) | (extracted & 0xFFF);
            array[idx] ^= (bf.flag1 << 31);
            
            /* Access with pointer arithmetic and casting */
            volatile unsigned char *byte_ptr = (volatile unsigned char *)&array[idx];
            for (j = 0; j < 4; j++) {
                byte_ptr[j] ^= (extracted >> (j * 8)) & 0xFF;
            }
        }
        
        /* 4. Conditional branching based on bit-field and multi-word results */
        /* Check parity using bit-field */
        if (bf.flag1 ^ bf.flag2) {
            /* Branch 1: Operations on low part */
            accessor.full = ll_var;
            accessor.parts.low = accessor.parts.low + extracted;
            accessor.parts.high = accessor.parts.high ^ bf.large;
            ll_var = accessor.full;
            
            /* Force STRICT_LOW_PART pattern */
            unsigned short low_word = (unsigned short)(ll_var & 0xFFFF);
            low_word = low_word * 3 + 1;
            ll_var = (ll_var & ~0xFFFFLL) | low_word;
        } else {
            /* Branch 2: Different operations */
            /* Compare high vs low word of long long */
            accessor.full = ll_var;
            if (accessor.parts.high > accessor.parts.low) {
                dbl_var = dbl_var / 1.5;
            } else {
                dbl_var = dbl_var * 1.5;
            }
            
            /* More bit manipulation */
            unsigned int mask = (1 << bf.small) - 1;
            extracted = extracted & mask;
            bf.medium = extracted;
        }
        
        /* 5. Inline assembly to force specific register constraints */
        /* This asm operates on low 32 bits of a 64-bit value */
        unsigned int low_part;
        __asm__ volatile (
            "movl %1, %0\n\t"
            "andl $0xFFFFF, %0\n\t"
            "addl $0x1000, %0"
            : "=r" (low_part)
            : "r" ((unsigned int)(ll_var & 0xFFFFFFFF))
            : "cc"
        );
        
        /* Use the result */
        temp_result += low_part;
        
        /* Prevent loop unrolling */
        __asm__ volatile ("" : : "r" (i) : "memory");
    }
    
    /* Aggregate results to prevent dead code elimination */
    int final_result = temp_result;
    for (i = 0; i < 256; i++) {
        final_result ^= array[i];
    }
    
    final_result += (int)(dbl_var * 1000);
    final_result += (int)(ll_var & 0xFFFFFFFF);
    final_result += bf.medium + bf.large;
    
    printf("Result: %d\n", final_result);
    return final_result & 0xFF;
}
