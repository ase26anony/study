/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int small : 3;
    unsigned int medium : 12;
    unsigned int large : 16;
} __attribute__((packed));

/* Union for mixed-type access to same memory */
union mixed_access {
    struct bitfield_pack bits;
    unsigned int full;
    unsigned short halves[2];
};

/* Function with complex operations to generate target RTL */
int process_resources(int argc, char **argv) {
    volatile struct bitfield_pack bf = {0};
    volatile union mixed_access ma = {0};
    
    /* Volatile long long for SUBREG generation on 32-bit targets */
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    
    /* Volatile array with complex indexing */
    volatile int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    int result = 0;
    
    /* Main loop with mixed operations */
    for (volatile int i = 0; i < iterations; i++) {
        /* ===== BIT-FIELD OPERATIONS ===== */
        /* These should generate ZERO_EXTRACT/STRICT_LOW_PART */
        
        /* Assignment to bit-field - potential STRICT_LOW_PART */
        bf.flag1 = (i & 1);
        bf.small = (i & 0x7);
        bf.medium = (i * 3) & 0xFFF;
        bf.large = (i * 5) & 0xFFFF;
        
        /* Extract bit-field values with masking - potential ZERO_EXTRACT */
        unsigned int extracted = ((unsigned int)bf.medium << 4) | bf.small;
        
        /* Access through union with type punning */
        ma.bits = bf;
        unsigned int masked = ma.full & 0x7FF;  /* Mask lower 11 bits */
        
        /* Complex bit-field expression */
        unsigned int combined = (bf.large >> 8) | ((bf.medium & 0xF) << 8);
        
        /* ===== LONG LONG OPERATIONS ===== */
        /* These should generate SUBREG expressions on 32-bit targets */
        
        /* Operations that work on high/low parts separately */
        ll_var += (long long)extracted;
        ll_var ^= (long long)masked << 32;  /* Affects high 32 bits */
        
        /* Extract high and low parts */
        unsigned int low_part = (unsigned int)(ll_var & 0xFFFFFFFF);
        unsigned int high_part = (unsigned int)(ll_var >> 32);
        
        /* Double precision operations (also multi-word on 32-bit) */
        dbl_var += (double)low_part / 1000.0;
        dbl_var -= (double)high_part / 1000000.0;
        
        /* ===== COMPLEX ARRAY ACCESS ===== */
        /* Generate MEM with complex address expressions */
        
        /* Non-trivial indexing pattern */
        int idx = ((extracted * 7) + (combined * 3) + i) & 0xFF;
        
        /* Read-modify-write with bit operations */
        array[idx] = (array[idx] & 0xFFFF) | ((masked & 0xFFFF) << 16);
        
        /* Another complex access pattern */
        int idx2 = ((low_part ^ high_part) + i * 13) & 0xFF;
        array[idx2] ^= (extracted << 8) | (combined & 0xFF);
        
        /* ===== CONTROL FLOW ===== */
        /* Branch based on bit-field and multi-word comparisons */
        
        if (bf.flag1) {
            /* When flag1 is set, do different operations */
            ll_var -= (long long)(bf.large << 16);
            dbl_var *= 1.0001;
            
            /* Access array with stride */
            for (int j = 0; j < 4; j++) {
                int stride_idx = (idx + j * 17) & 0xFF;
                array[stride_idx] += j * 100;
            }
        } else {
            /* Alternative path */
            if ((low_part & 0x80000000) != (high_part & 0x80000000)) {
                ll_var = ~ll_var;
            }
            
            /* Nested condition based on bit-field parity */
            if (bf.small & 1) {
                dbl_var = -dbl_var;
                
                /* Pointer arithmetic with casting */
                volatile int *ptr = &array[idx];
                for (int k = 0; k < 3; k++) {
                    *(ptr + k * 9) &= 0x7FFFFFFF;
                }
            }
        }
        
        /* Switch based on extracted bit-field value */
        switch (extracted & 0x7) {
            case 0:
                ll_var >>= 1;
                break;
            case 1:
                ll_var <<= 2;
                break;
            case 2:
                ll_var |= 0x5555555555555555LL;
                break;
            case 3:
                ll_var &= 0xAAAAAAAAAAAAAAAALL;
                break;
            default:
                ll_var ^= (long long)array[idx];
                break;
        }
        
        /* Accumulate result */
        result += (low_part & 0xFF) + (array[idx] & 0xFF);
    }
    
    /* Final aggregation to prevent dead code elimination */
    result += (int)(ll_var & 0xFFFFFFFF);
    result += (int)(ll_var >> 32);
    result += (int)(dbl_var * 1000);
    result += bf.medium + bf.large;
    
    /* Complex final computation using all variables */
    for (int i = 0; i < 16; i++) {
        result += array[(i * 17) & 0xFF];
    }
    
    return result;
}

/* Inline assembly to force specific register constraints */
void asm_operations(volatile long long *ll, volatile int *arr) {
    /* Inline asm that operates on low 32 bits of a 64-bit value */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+m" (*ll)
        : "r" (*arr)
        : "%eax", "cc"
    );
    
    /* Another asm with explicit low-part operation */
    unsigned int low, high;
    asm volatile (
        "movl %2, %%eax\n\t"
        "movl %3, %%edx\n\t"
        "addl %%eax, %0\n\t"
        "adcl %%edx, %1\n\t"
        : "+m" (low), "+m" (high)
        : "r" ((int)(*ll & 0xFFFFFFFF)), "r" ((int)(*ll >> 32))
        : "%eax", "%edx", "cc"
    );
}

/* Main driver function */
int main(int argc, char **argv) {
    int result = process_resources(argc, argv);
    
    /* Additional operations with inline assembly */
    volatile long long ll_temp = 0x9876543210ABCDEFLL;
    volatile int arr_temp[4] = {1, 2, 3, 4};
    
    asm_operations(&ll_temp, &arr_temp[0]);
    
    /* Mix results */
    result ^= (int)(ll_temp & 0xFFFFFFFF);
    
    /* Print to prevent optimization */
    printf("Result: %d\n", result);
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
