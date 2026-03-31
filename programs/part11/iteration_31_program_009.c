/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT and STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int small : 3;
    unsigned int medium : 12;
    unsigned int large : 16;
    unsigned int flag2 : 1;
} __attribute__((packed));

/* Union for mixed-type access to same memory */
union mixed_access {
    struct bitfield_pack bits;
    unsigned int raw;
    volatile unsigned char bytes[4];
};

/* Function to force complex addressing modes */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Function with inline assembly to mimic STRICT_LOW_PART behavior */
static unsigned long long asm_low_part(unsigned long long val) {
    unsigned int low, high;
    
    /* Split 64-bit value */
    low = (unsigned int)(val & 0xFFFFFFFF);
    high = (unsigned int)(val >> 32);
    
    /* Inline asm that operates on low 32 bits only */
    __asm__ volatile (
        "addl $0x1234, %0\n\t"
        "movl %0, %1\n\t"
        : "+r" (low), "=r" (high)
        : "0" (low), "1" (high)
        : "cc"
    );
    
    return ((unsigned long long)high << 32) | low;
}

int main(int argc, char **argv) {
    volatile struct bitfield_pack bf = {0};
    volatile union mixed_access mix = {0};
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int i, j, limit;
    int result = 0;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (limit <= 0) limit = 100;
    
    /* Main loop with mixed operations */
    for (j = 0; j < limit; j++) {
        /* 1. Bit-field operations (ZERO_EXTRACT/STRICT_LOW_PART) */
        bf.flag1 = j & 1;
        bf.small = (j >> 1) & 0x7;
        bf.medium = (j * 7) & 0xFFF;
        bf.large = (j * 13) & 0xFFFF;
        bf.flag2 = (j >> 2) & 1;
        
        /* Access via union to force different views of same memory */
        mix.bits = *(struct bitfield_pack*)&bf;
        unsigned int temp = mix.raw;
        
        /* Manual bit extraction (may generate ZERO_EXTRACT) */
        unsigned int extracted = (temp >> 4) & 0xFFF;  /* Extract 12-bit field */
        
        /* 2. Multi-word operations (SUBREG generation) */
        /* Operations on long long (64-bit) on 32-bit targets */
        ll_var += (long long)extracted;
        ll_var ^= (long long)temp << 32;
        ll_var -= (long long)j * 0x100000000LL;
        
        /* Double operations (may use multiple registers) */
        dbl_var += (double)extracted / 1000.0;
        dbl_var *= 1.0001;
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = complex_index(j, bf.small + 1, bf.medium & 0xFF);
        idx = idx & 0xFF;  /* Ensure within bounds */
        
        /* Read-modify-write with bit manipulation */
        array[idx] = (array[idx] & ~0xFFF) | extracted;
        array[(idx + 128) & 0xFF] ^= (temp << 16) | (temp >> 16);
        
        /* 4. Conditional branching based on bit-field and multi-word results */
        if (bf.flag1 ^ bf.flag2) {
            /* Check high vs low word of long long */
            unsigned int low_word = (unsigned int)(ll_var & 0xFFFFFFFF);
            unsigned int high_word = (unsigned int)(ll_var >> 32);
            
            if (high_word > low_word) {
                /* Use inline assembly for low-part operation */
                ll_var = asm_low_part(ll_var);
                
                /* More bit-field manipulation */
                bf.large = (bf.large + high_word - low_word) & 0xFFFF;
            } else {
                /* Access misaligned data through pointer casting */
                char *byte_ptr = (char*)&ll_var;
                int misaligned_int;
                /* Force potential unaligned access */
                __builtin_memcpy(&misaligned_int, byte_ptr + 1, sizeof(int));
                array[idx] += misaligned_int & 0xFFF;
            }
            
            /* Toggle bits using STRICT_LOW_PART-like pattern */
            mix.bytes[1] ^= 0x55;
            mix.bytes[2] ^= 0xAA;
        } else {
            /* Different path with shift/extract operations */
            unsigned int rotated = (temp << 8) | (temp >> 24);
            bf.medium = (rotated >> 4) & 0xFFF;  /* Another extract */
            
            /* Force SUBREG by accessing halves of double */
            volatile unsigned int *dbl_parts = (volatile unsigned int*)&dbl_var;
            dbl_parts[0] += rotated;
            dbl_parts[1] ^= rotated;
        }
        
        /* Complex array access pattern */
        for (int k = 0; k < bf.small + 1; k++) {
            int idx2 = (idx + k * 7) & 0xFF;
            /* Mix array access with bit-field extraction */
            array[idx2] = (array[idx2] & ~((1 << bf.small) - 1)) | 
                         (extracted & ((1 << bf.small) - 1));
        }
    }
    
    /* Aggregate results to prevent elimination */
    for (i = 0; i < 256; i++) {
        result += array[i];
    }
    
    result += bf.small + bf.medium + bf.large;
    result += (int)(ll_var & 0xFFFFFFFF) + (int)(ll_var >> 32);
    result += (int)(dbl_var * 1000);
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
