/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 1;    /* 1-bit field */
    unsigned int b : 3;    /* 3-bit field */
    unsigned int c : 12;   /* 12-bit field */
    unsigned int d : 16;   /* 16-bit field */
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_struct bits;
    unsigned int full;
    unsigned short halves[2];
    unsigned char bytes[4];
};

/* Complex array with stride access pattern */
#define ARRAY_SIZE 128
#define STRIDE 3

/* Main function with operations designed to generate target RTL patterns */
int main(int argc, char **argv) {
    volatile int i, j, k;
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile union mixed_access data;
    volatile unsigned int array[ARRAY_SIZE];
    
    /* Initialize array with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i * 7 + 3;
    }
    
    /* Initialize bit-field structure */
    data.bits.a = 1;
    data.bits.b = 5;
    data.bits.c = 2047;
    data.bits.d = 0xABCD;
    
    /* Use argc to control loop iterations (prevents optimization) */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    volatile unsigned int result = 0;
    
    /* Main loop with operations that should generate target RTL patterns */
    for (i = 0; i < iterations; i++) {
        /* ============================================
         * BIT-FIELD OPERATIONS for ZERO_EXTRACT/STRICT_LOW_PART
         * ============================================ */
        
        /* Operation 1: Extract and manipulate bit-fields */
        unsigned int temp_a = data.bits.a;  /* Should generate ZERO_EXTRACT */
        unsigned int temp_b = data.bits.b;  /* Should generate ZERO_EXTRACT */
        unsigned int temp_c = data.bits.c;  /* Should generate ZERO_EXTRACT */
        
        /* Combine bit-fields with masking and shifting */
        unsigned int combined = (temp_a << 31) | (temp_b << 28) | (temp_c << 16);
        
        /* Modify bit-fields individually - may generate STRICT_LOW_PART */
        data.bits.b = (data.bits.b + 1) & 0x7;      /* Wrap 3-bit field */
        data.bits.c = (data.bits.c * 2) & 0xFFF;    /* Wrap 12-bit field */
        
        /* Complex bit-field assignment with side effects */
        data.bits.d = (data.bits.d ^ combined) & 0xFFFF;
        
        /* ============================================
         * LONG LONG OPERATIONS for SUBREG generation
         * ============================================ */
        
        /* Operations that may be split into high/low parts on 32-bit targets */
        ll_var = ll_var + 0x100000001LL;    /* Add to both halves */
        ll_var = ll_var << 1;               /* Shift entire 64-bit value */
        ll_var = ll_var | 0x80000000;       /* Set bit in low half */
        
        /* Compare high vs low parts - forces SUBREG usage */
        unsigned int low_part = (unsigned int)(ll_var & 0xFFFFFFFF);
        unsigned int high_part = (unsigned int)(ll_var >> 32);
        
        /* ============================================
         * DOUBLE OPERATIONS for FP SUBREG generation
         * ============================================ */
        
        /* Double operations that may use multiple registers */
        dbl_var = dbl_var * 1.0001;
        dbl_var = dbl_var + (double)i;
        
        /* ============================================
         * COMPLEX ARRAY ACCESS for MEM with complex addressing
         * ============================================ */
        
        /* Complex array indexing with stride */
        j = (i * STRIDE + data.bits.c) % ARRAY_SIZE;
        k = ((i + data.bits.b) * 2 + 1) % ARRAY_SIZE;
        
        /* Read-modify-write with complex addressing */
        array[j] = array[j] ^ (array[k] + combined);
        
        /* Misaligned access simulation via pointer arithmetic */
        if (j > 0 && j < ARRAY_SIZE - 1) {
            unsigned char *byte_ptr = (unsigned char *)&array[j];
            unsigned int misaligned_read;
            /* Read potentially misaligned word */
            misaligned_read = *((unsigned int *)(byte_ptr + 1));
            array[j] = array[j] + (misaligned_read & 0xFFFF);
        }
        
        /* ============================================
         * CONTROL FLOW based on operation results
         * ============================================ */
        
        /* Branch based on bit-field parity */
        if (data.bits.a ^ (data.bits.b & 1)) {
            /* Path 1: More bit-field manipulations */
            data.bits.c = data.bits.c >> 1;
            ll_var = ll_var - 0x5555555555555555LL;
            
            /* Additional complex array access */
            int idx = (low_part ^ high_part) % ARRAY_SIZE;
            array[idx] = array[idx] * 3 + 1;
        } else {
            /* Path 2: Different operations */
            data.bits.d = data.bits.d << 1;
            dbl_var = dbl_var / 1.0001;
            
            /* Array access with different pattern */
            int idx = (low_part + high_part * 2) % ARRAY_SIZE;
            array[idx] = array[idx] ^ 0xAAAAAAAA;
        }
        
        /* Switch based on bit-field value */
        switch (data.bits.b & 0x3) {
            case 0:
                ll_var = ll_var | 0xF0F0F0F0F0F0F0F0LL;
                break;
            case 1:
                data.bits.c = (data.bits.c + array[i % ARRAY_SIZE]) & 0xFFF;
                break;
            case 2:
                dbl_var = dbl_var - (double)data.bits.d;
                break;
            case 3:
                /* Inline assembly to force specific register usage */
                asm volatile (
                    "/* Force register constraints */"
                    : "+r" (low_part)
                    : "r" (high_part)
                    : "cc"
                );
                break;
        }
        
        /* Accumulate results to prevent optimization */
        result += data.bits.a + data.bits.b + data.bits.c + data.bits.d;
        result += low_part & 0xFF;
        result += (unsigned int)dbl_var;
        result += array[i % 8];
    }
    
    /* Final aggregation and output */
    unsigned int final_sum = result;
    for (i = 0; i < ARRAY_SIZE; i += 4) {
        final_sum += array[i];
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %u\n", final_sum);
    
    /* Return based on result to affect control flow */
    return (final_sum & 0xFF) == 0 ? 0 : 1;
}

/* Additional function with inline assembly for direct RTL influence */
void __attribute__((noinline)) asm_helper(unsigned long long *val) {
    unsigned int low, high;
    
    /* Inline assembly that operates on low 32 bits only */
    asm volatile (
        "movl %1, %0\n\t"           /* Get low part */
        "addl $0x1234, %0\n\t"      /* Modify low part */
        "movl %0, %1\n\t"           /* Store back to low part */
        : "=r" (low), "+m" (*val)
        :
        : "cc"
    );
    
    /* Another asm that might generate STRICT_LOW_PART */
    asm volatile (
        "andl $0xFFFF, %0"          /* Operate on low 16 bits */
        : "+r" (low)
        :
        : "cc"
    );
}
