/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 12;
    unsigned int pad   : 16;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_pack bits;
    unsigned int full;
    volatile unsigned char bytes[4];
};

/* Force generation of SUBREG for multi-word operations */
typedef volatile long long vllong;
typedef volatile double vdouble;

/* Complex array with stride access */
#define ARRAY_SIZE 128
#define STRIDE 3

int main(int argc, char *argv[]) {
    volatile int i, j, k;
    volatile unsigned int temp;
    volatile long long result = 0;
    
    /* Variables to force specific RTL patterns */
    union mixed_access data;
    vllong big_val = 0x123456789ABCDEF0LL;
    vdouble fp_val = 3.141592653589793;
    volatile unsigned int array[ARRAY_SIZE];
    
    /* Initialize array with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i * 7 + 1;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* Main loop with mixed operations */
    for (i = 0; i < iterations; i++) {
        /* 1. Bit-field operations for ZERO_EXTRACT/STRICT_LOW_PART */
        data.bits.flag1 = i & 1;
        data.bits.flag2 = (i >> 1) & 0x7;
        data.bits.value = (i * 3) & 0xFFF;
        
        /* Extract using bit operations (may generate ZERO_EXTRACT) */
        temp = data.bits.value;
        
        /* Mask and shift operations */
        unsigned int masked = data.full & 0x0FFF;  /* ZERO_EXTRACT pattern */
        unsigned int shifted = masked << 4;
        
        /* STRICT_LOW_PART pattern through explicit masking */
        data.full = (data.full & ~0xFF) | (shifted & 0xFF);
        
        /* 2. Multi-word operations for SUBREG generation */
        /* Operations on long long (needs multiple registers on 32-bit) */
        big_val = big_val + (temp * 0x10001LL);
        big_val = big_val ^ (big_val >> 32);  /* Mix high and low parts */
        
        /* Force SUBREG by accessing parts separately */
        unsigned int low_part = (unsigned int)(big_val & 0xFFFFFFFF);
        unsigned int high_part = (unsigned int)(big_val >> 32);
        
        /* Recombine with arithmetic */
        big_val = ((vllong)high_part << 32) | (vllong)(low_part + i);
        
        /* Floating point operations (also multi-word) */
        fp_val = fp_val * 1.01 + (double)temp;
        
        /* 3. Complex memory addressing with bit-field derived index */
        /* Non-trivial array indexing */
        j = ((data.bits.value * STRIDE + i) & 0x7F);
        k = ((data.bits.flag2 * 5 + data.bits.flag1 * 13) & 0x7F);
        
        /* Read-modify-write with complex address */
        array[j] = array[j] ^ array[k];
        array[k] = array[k] + (temp & 0xF);
        
        /* 4. Conditional control flow based on bit-field results */
        if (data.bits.flag1) {
            /* When flag1 is set, do different operations */
            big_val = big_val - 0x100000000LL;
            fp_val = fp_val / 1.5;
            
            /* Access array with different stride */
            unsigned int idx = (j * 2 + k) % ARRAY_SIZE;
            array[idx] = array[idx] | 0x80000000;
        } else {
            /* Alternative path */
            big_val = big_val | 0x5555555555555555LL;
            
            /* Misaligned access simulation through pointer arithmetic */
            unsigned int *ptr = &array[(i * 7) % (ARRAY_SIZE - 1)];
            unsigned int val = *ptr;
            *ptr = ((val << 16) | (val >> 16));  /* Byte swap */
        }
        
        /* Switch based on bit-field combination */
        switch (data.bits.flag2) {
            case 0:
                fp_val = fp_val + 1.0;
                break;
            case 1:
                big_val = big_val << 1;
                break;
            case 2:
                array[i % ARRAY_SIZE] = data.full;
                break;
            default:
                /* Complex operation mixing everything */
                temp = (unsigned int)(big_val & 0xFFFFFFFF) ^ data.full;
                array[(temp >> 4) % ARRAY_SIZE] = temp;
                break;
        }
        
        /* Check parity of low word (for branch prediction) */
        if ((low_part & 1) == 0) {
            /* Even: increment array elements */
            for (j = 0; j < 4; j++) {
                unsigned int idx = (i * 4 + j) % ARRAY_SIZE;
                array[idx] = array[idx] + 1;
            }
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        result += array[i];
    }
    
    result += data.full;
    result += (long long)fp_val;
    result += big_val;
    
    /* Use result to affect return value */
    return (int)(result & 0x7FFFFFFF);
}

/* Additional function with inline assembly for direct RTL influence */
void asm_helpers(void) {
    volatile long long ll_var = 0x1122334455667788LL;
    volatile unsigned int int_var = 0;
    
    /* Inline asm that might generate STRICT_LOW_PART-like behavior */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0xFFFF, %%eax\n\t"  /* Operate on low part only */
        "movl %%eax, %0\n\t"
        : "=r" (int_var)
        : "r" (ll_var)
        : "%eax"
    );
    
    /* Another asm operating on bit ranges */
    volatile unsigned int bits = 0x12345678;
    volatile unsigned int extracted;
    
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "shrl $8, %%eax\n\t"
        "andl $0xFFF, %%eax\n\t"  /* Extract bits 8-19 */
        "movl %%eax, %0\n\t"
        : "=r" (extracted)
        : "r" (bits)
        : "%eax"
    );
}
