/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag : 1;
    unsigned int mode : 3;
    unsigned int data : 12;
    unsigned int count : 8;
    unsigned int pad : 8;
} __attribute__((packed));

/* Union for mixed-width access */
union mixed_access {
    struct bitfield_pack bits;
    unsigned int word;
    unsigned short halves[2];
};

/* Function to force complex addressing modes */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile struct bitfield_pack bf = {0};
    volatile union mixed_access ma = {0};
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int limit = (argc > 1) ? atoi(argv[1]) : 100;
    int i, temp, result = 0;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Main loop with mixed operations */
    for (i = 0; i < limit; i++) {
        /* 1. Bit-field operations (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        bf.flag = i & 1;                    /* Single bit assignment */
        bf.mode = (i >> 1) & 0x7;           /* 3-bit field */
        bf.data = (bf.data + i) & 0xFFF;    /* 12-bit field with wrap */
        bf.count = (bf.count + 1) & 0xFF;   /* 8-bit counter */
        
        /* Extract using bit masking (may generate ZERO_EXTRACT) */
        temp = (ma.word >> 4) & 0xFFF;      /* Extract bits 4-15 */
        
        /* Combined bit-field access */
        ma.bits.mode = (ma.bits.mode + bf.mode) & 0x7;
        
        /* 2. Multi-word operations (potential SUBREG generation) */
        /* Operations on long long (64-bit) on 32-bit targets */
        ll_var = ll_var + (long long)(i * 0x10001LL);
        ll_var = ll_var ^ (0xF0F0F0F0F0F0F0F0LL >> (i & 0x3F));
        
        /* Double precision operations */
        dbl_var = dbl_var * 1.01 + (double)i;
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = complex_index(i, bf.data & 0x3F, bf.mode * 4);
        if (idx >= 0 && idx < 256) {
            /* Read-modify-write with bit manipulation */
            array[idx] = (array[idx] + (bf.flag ? 1 : -1)) & 0xFFFF;
            
            /* Access with byte offset (potential misaligned access) */
            volatile char *byte_ptr = (volatile char *)&array[idx];
            byte_ptr[1] = byte_ptr[1] ^ (i & 0xFF);
        }
        
        /* 4. Control flow based on bit-field and multi-word results */
        if (bf.flag) {
            /* Branch 1: Operate on low part of long long */
            unsigned int low_part = (unsigned int)(ll_var & 0xFFFFFFFF);
            array[i & 0xFF] = array[i & 0xFF] ^ low_part;
            
            /* Force STRICT_LOW_PART pattern */
            ma.halves[0] = (ma.halves[0] + low_part) & 0xFFFF;
        } else {
            /* Branch 2: Operate on high part */
            unsigned int high_part = (unsigned int)(ll_var >> 32);
            array[(i + 128) & 0xFF] = array[(i + 128) & 0xFF] + high_part;
            
            /* More bit-field manipulation */
            bf.data = (bf.data ^ high_part) & 0xFFF;
        }
        
        /* Conditional based on comparison of high/low words */
        unsigned int low = (unsigned int)(ll_var & 0xFFFFFFFF);
        unsigned int high = (unsigned int)(ll_var >> 32);
        if (low > high) {
            /* Swap operation using temp variable */
            temp = array[i & 0xFF];
            array[i & 0xFF] = array[(i + 1) & 0xFF];
            array[(i + 1) & 0xFF] = temp;
        }
        
        /* Mix in double comparison */
        if (dbl_var > 100.0) {
            dbl_var = dbl_var / 2.0;
        }
    }
    
    /* Aggregate results to prevent optimization */
    for (i = 0; i < 256; i++) {
        result += array[i];
    }
    result += bf.flag + bf.mode + bf.data + bf.count;
    result += (int)(ll_var & 0xFFFFFFFF) + (int)(ll_var >> 32);
    result += (int)dbl_var;
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}

/* Additional function with inline assembly for specific patterns */
void asm_helpers(void) {
    volatile long long ll_input = 0x123456789ABCDEF0LL;
    volatile int output;
    
    /* Inline asm that operates on low 32 bits (potential STRICT_LOW_PART) */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl $0x1000, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (output)
        : "r" ((int)(ll_input & 0xFFFFFFFF))
        : "%eax"
    );
    
    /* Another asm with bit extraction */
    volatile unsigned int value = 0x87654321;
    volatile unsigned int extracted;
    
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "shrl $8, %%eax\n\t"
        "andl $0xFFF, %%eax\n\t"  /* Extract bits 8-19 */
        "movl %%eax, %0\n\t"
        : "=r" (extracted)
        : "r" (value)
        : "%eax"
    );
}
