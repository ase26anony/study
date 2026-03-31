/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int small : 3;
    unsigned int medium : 12;
    unsigned int large : 16;
} __attribute__((packed));

/* Union for accessing the same memory as different types */
union mixed_access {
    struct bitfield_pack bits;
    unsigned int word;
    unsigned short halves[2];
    unsigned char bytes[4];
};

/* Function to force complex addressing modes */
static int complex_index(int *arr, int i, int stride, int offset) {
    return arr[i * stride + offset];
}

/* Function with inline assembly to hint at register constraints */
static void asm_hint(unsigned long long val) {
    /* Inline asm that operates on low 32 bits */
    unsigned int low, high;
    low = (unsigned int)val;
    high = (unsigned int)(val >> 32);
    
    __asm__ volatile (
        "addl $1, %0\n\t"
        "adcl $0, %1"
        : "+r" (low), "+r" (high)
        : 
        : "cc"
    );
    
    /* Use the results to prevent optimization */
    volatile unsigned int dummy = low + high;
    (void)dummy;
}

int main(int argc, char **argv) {
    volatile struct bitfield_pack bf = {0};
    volatile union mixed_access u = {0};
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int limit = (argc > 1) ? atoi(argv[1]) : 100;
    int i, temp;
    unsigned long long accumulator = 0;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Main loop with mixed operations */
    for (i = 0; i < limit; i++) {
        /* 1. Bit-field operations that may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.flag1 = i & 1;
        bf.small = (i >> 1) & 0x7;
        bf.medium = (i * 7) & 0xFFF;
        bf.large = (i * 13) & 0xFFFF;
        
        /* Access through union with type punning */
        u.bits = bf;
        temp = u.word;
        
        /* Manual bit extraction (may generate ZERO_EXTRACT) */
        unsigned int extracted = (temp >> 4) & 0xFFF;  /* Extract 12 bits */
        
        /* 2. Multi-word operations that may generate SUBREG */
        ll_var += (long long)extracted;
        ll_var ^= (long long)i << 32;
        
        /* Double precision operations on 32-bit target may use SUBREG */
        dbl_var += (double)i * 0.01;
        dbl_var = dbl_var * 1.0001;
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = ((extracted * 3 + i) & 0xFF);
        int stride = (bf.small + 1) * 2;
        int offset = bf.flag1 ? 4 : 8;
        
        /* Array access with complex addressing */
        int val = complex_index((int*)array, idx / stride, stride, offset % stride);
        
        /* Modify array element - may create MEM with complex address */
        array[idx] = (array[idx] + val) & 0xFFFF;
        
        /* 4. Conditional based on multi-word comparison */
        if ((ll_var & 0xFFFFFFFF) > (ll_var >> 32)) {
            /* Low word > high word */
            bf.flag1 = !bf.flag1;
            ll_var = ll_var >> 1;  /* Shift may generate SUBREG operations */
        } else {
            /* Use inline assembly hint */
            asm_hint(ll_var);
        }
        
        /* Switch based on bit-field value */
        switch (bf.small) {
            case 0:
                dbl_var -= 0.5;
                break;
            case 1:
            case 2:
                ll_var |= 0x100000000LL;
                break;
            case 3:
            case 4:
                /* Access misaligned data through pointer casting */
                unsigned char *ptr = (unsigned char*)&ll_var;
                ptr[1] = ptr[3] ^ ptr[5];
                break;
            default:
                /* Force STRICT_LOW_PART by operating on partial register */
                u.halves[0] = u.halves[0] + u.halves[1];
                break;
        }
        
        /* Accumulate results */
        accumulator += extracted + (ll_var & 0xFF) + (int)dbl_var;
    }
    
    /* Final aggregation to prevent dead code elimination */
    int result = (accumulator & 0xFFFFFFFF) ^ (accumulator >> 32);
    
    /* Mix in array elements */
    for (i = 0; i < 16; i++) {
        result ^= array[i * 16];
    }
    
    /* Add bit-field state */
    result += bf.medium + (bf.large << 12);
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
