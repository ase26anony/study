/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 12;
    unsigned int flag3 : 4;
    unsigned int pad : 12;
} __attribute__((packed));

/* Union for mixed-type access to same memory */
union mixed_access {
    struct bitfield_pack bits;
    unsigned int raw;
    volatile unsigned int volatile_raw;
};

/* Function to force complex addressing modes */
static int complex_index(int *arr, int i, int stride, int offset) {
    return arr[i * stride + offset];
}

/* Function with inline assembly to simulate specific patterns */
static void asm_bit_ops(volatile unsigned long long *val) {
    /* Inline asm that operates on low 32 bits */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0xFFFFF, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (*val)
        : "r" (*val)
        : "%eax"
    );
}

int main(int argc, char **argv) {
    volatile int i, j;
    volatile int result = 0;
    
    /* Variables that may generate SUBREG operations on 32-bit targets */
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    
    /* Bit-field structure with volatile access */
    volatile union mixed_access bf_data;
    bf_data.raw = 0;
    
    /* Array for complex memory addressing */
    volatile int arr[256];
    for (i = 0; i < 256; i++) {
        arr[i] = i * 3;
    }
    
    /* Loop counter - volatile to prevent optimization */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 10) loop_limit = 10;
    if (loop_limit > 1000) loop_limit = 1000;
    
    /* Main loop with mixed operations */
    for (i = 0; i < loop_limit; i++) {
        /* 1. Bit-field operations - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf_data.bits.flag1 = i & 1;
        bf_data.bits.flag2 = (i >> 1) & 0x7;
        bf_data.bits.value = (bf_data.bits.value + i) & 0xFFF;
        bf_data.bits.flag3 = (bf_data.bits.flag3 ^ bf_data.bits.flag2) & 0xF;
        
        /* Extract using shift/mask - may create ZERO_EXTRACT */
        unsigned int extracted = (bf_data.raw >> 4) & 0xFFF;
        
        /* 2. Multi-word operations - may generate SUBREG */
        ll_var = ll_var + (extracted * 1000LL);
        dbl_var = dbl_var * 1.01 + (extracted * 0.001);
        
        /* Split long long into high/low parts - forces SUBREG handling */
        unsigned int low_part = (unsigned int)(ll_var & 0xFFFFFFFF);
        unsigned int high_part = (unsigned int)(ll_var >> 32);
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = (extracted + i) & 0xFF;
        int stride = (bf_data.bits.flag2 + 1) * 2;
        int offset = bf_data.bits.flag3;
        
        /* Access array with complex addressing mode */
        volatile int *ptr = &arr[idx];
        *ptr = *ptr + low_part;
        
        /* Another complex access pattern */
        arr[(idx * stride + offset) & 0xFF] = high_part;
        
        /* 4. Conditional based on bit-field and multi-word comparisons */
        if (bf_data.bits.flag1) {
            /* When flag1 is set, operate on low part only */
            ll_var = (ll_var & 0xFFFFFFFF00000000LL) | 
                    ((ll_var & 0xFFFFFFFF) ^ 0xAAAAAAAA);
            
            /* This might generate STRICT_LOW_PART */
            arr[idx] = arr[idx] & 0xFFFF;
        } else {
            /* When flag1 is clear, use the full long long */
            if (high_part > low_part) {
                ll_var = ll_var >> 1;
            } else {
                ll_var = ll_var << 1;
            }
            
            /* Access with byte offset - may create SUBREG of MEM */
            char *byte_ptr = (char *)&arr[idx];
            byte_ptr[1] = (low_part >> 8) & 0xFF;
            byte_ptr[3] = (low_part >> 24) & 0xFF;
        }
        
        /* 5. Use inline assembly for specific patterns */
        if ((i % 17) == 0) {
            asm_bit_ops(&ll_var);
        }
        
        /* Mix in some floating point operations */
        if ((i % 23) == 0) {
            /* Cast between double and long long - may create interesting RTL */
            ll_var = (long long)dbl_var;
            dbl_var = (double)(ll_var & 0x7FFFFFFFFFFFFFFFLL);
        }
        
        /* Accumulate result to prevent dead code elimination */
        result += extracted + low_part + arr[idx & 0xF];
    }
    
    /* Final computation using all variables */
    result += (int)(ll_var >> 32) + (int)(ll_var & 0xFFFFFFFF);
    result += (int)dbl_var;
    result += bf_data.raw;
    
    /* Use result to affect control flow */
    if (result > 1000000) {
        printf("Large result: %d\n", result);
    } else {
        printf("Result: %d\n", result);
    }
    
    return result & 0xFF;
}
