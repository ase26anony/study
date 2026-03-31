/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_packet {
    unsigned int flag : 1;
    unsigned int mode : 3;
    unsigned int data : 12;
    unsigned int pad : 16;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_packet bits;
    unsigned int word;
    unsigned short halves[2];
};

/* Function to force complex addressing modes */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Function with inline assembly to force specific register usage */
static void asm_bit_ops(unsigned long long *val) {
    /* Inline asm that operates on low 32 bits of a 64-bit value */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0xFFFFF, %%eax\n\t"  /* Operate on low 20 bits */
        "movl %%eax, %0\n\t"
        : "=r" (*val)
        : "r" (*val)
        : "%eax", "cc"
    );
}

int main(int argc, char *argv[]) {
    volatile struct bitfield_packet bf = {0};
    volatile union mixed_access ma = {0};
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int i, limit;
    int result = 0;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (limit <= 0) limit = 100;
    
    /* Main loop with mixed operations */
    for (i = 0; i < limit; i++) {
        /* 1. Bit-field operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.flag = i & 1;                    /* Single bit assignment */
        bf.mode = (i >> 1) & 0x7;          /* 3-bit field */
        bf.data = (bf.data + i) & 0xFFF;   /* 12-bit field with wrap */
        
        /* Access through union with type punning */
        ma.bits = bf;
        unsigned int temp = ma.word;
        
        /* Manual bit extraction (may generate ZERO_EXTRACT) */
        unsigned int extracted = (temp >> 4) & 0xFF;  /* Extract bits 4-11 */
        
        /* 2. Multi-word operations to generate SUBREG patterns */
        /* Operations on long long (64-bit) on 32-bit targets */
        ll_var = ll_var + (extracted * 0x10001LL);
        ll_var = ll_var ^ (0xF0F0F0F0F0F0F0F0LL >> (i & 0x3F));
        
        /* Double operations (may use multiple registers) */
        dbl_var = dbl_var * 1.01 + (extracted * 0.001);
        
        /* 3. Complex memory addressing with bit-field results */
        int idx = complex_index(i, bf.data & 0x3F, extracted & 0x1F);
        idx = idx & 0xFF;  /* Ensure within bounds */
        
        /* Read-modify-write with bit manipulation */
        array[idx] = (array[idx] + (temp & 0xFFF)) | (extracted << 16);
        
        /* 4. Conditional based on bit-field and multi-word results */
        if (bf.flag) {
            /* When flag is 1, operate on high part of long long */
            unsigned int high_part = (unsigned int)(ll_var >> 32);
            array[(idx + 128) & 0xFF] ^= high_part;
            
            /* STRICT_LOW_PART pattern through masking */
            unsigned int masked = temp & 0xFFFF;
            ma.halves[0] = masked;  /* Only affects low 16 bits */
        } else {
            /* When flag is 0, operate on low part */
            unsigned int low_part = (unsigned int)ll_var;
            array[(idx + 64) & 0xFF] += low_part;
            
            /* Different bit manipulation */
            ma.halves[1] = extracted & 0xFFFF;
        }
        
        /* 5. Additional SUBREG patterns through type casting */
        /* Cast between 64-bit and 32-bit types */
        if ((i & 0xF) == 0) {
            unsigned int *ptr = (unsigned int *)&ll_var;
            ptr[0] = ptr[0] ^ ptr[1];  /* Mix high and low words */
        }
        
        /* 6. Call function with inline assembly */
        if ((i % 7) == 0) {
            asm_bit_ops((unsigned long long *)&ll_var);
        }
        
        /* Prevent loop unrolling */
        __asm__ volatile ("" : : "r"(i) : "memory");
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (i = 0; i < 256; i++) {
        result ^= array[i];
    }
    result ^= ma.word;
    result ^= (int)ll_var;
    result ^= (int)(ll_var >> 32);
    result ^= (int)(dbl_var * 1000);
    
    /* Use result to affect return value */
    return result & 0xFF;
}
