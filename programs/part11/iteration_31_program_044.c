/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int value : 12;
    unsigned int flag2 : 3;
    unsigned int pad : 16;
};

/* Union to force specific bit manipulations */
union bit_manipulator {
    unsigned int full;
    struct {
        unsigned int low16 : 16;
        unsigned int high16 : 16;
    } parts;
};

/* Function to create complex addressing patterns */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Main function with operations designed to generate target RTL patterns */
int main(int argc, char **argv) {
    volatile struct bitfield_pack bf = {0, 0, 0, 0};
    volatile union bit_manipulator manip = {0};
    
    /* Multi-word types for SUBREG generation */
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    
    /* Array for complex memory addressing */
    volatile int array[256];
    volatile int temp_results[4] = {0};
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Loop counter controlled by argc to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    int sum = 0;
    
    /* Main loop with mixed operations */
    for (volatile int i = 0; i < iterations; i++) {
        /* ===== BIT-FIELD OPERATIONS (ZERO_EXTRACT/STRICT_LOW_PART) ===== */
        
        /* Direct bit-field assignment - may generate STRICT_LOW_PART */
        bf.value = (i * 7) & 0xFFF;
        bf.flag1 = i & 1;
        bf.flag2 = (i >> 1) & 0x7;
        
        /* Extract bit-field using mask and shift - may generate ZERO_EXTRACT */
        unsigned int extracted = (bf.value >> 4) & 0x3F;
        
        /* Union-based bit manipulation */
        manip.full = i * 0x1234;
        manip.parts.low16 = (manip.parts.low16 + extracted) & 0xFFFF;
        
        /* Complex bit-field combination */
        unsigned int combined = (bf.flag1 << 15) | (bf.flag2 << 12) | (bf.value & 0xFFF);
        
        /* ===== MULTI-WORD OPERATIONS (SUBREG generation) ===== */
        
        /* Operations on long long - may generate SUBREG for 32-bit targets */
        ll_var = ll_var + (long long)(i * 0x10001);
        ll_var = ll_var ^ (0xF0F0F0F0F0F0F0F0LL >> (i & 0x3F));
        
        /* Compare high and low parts */
        int high_part = (int)(ll_var >> 32);
        int low_part = (int)(ll_var & 0xFFFFFFFF);
        
        /* Double operations - also multi-word */
        dbl_var = dbl_var * 1.01 + (double)i * 0.001;
        
        /* ===== COMPLEX MEMORY ADDRESSING ===== */
        
        /* Complex array indexing with bit-field derived offset */
        int idx = complex_index(i, 13, bf.value & 0x3F);
        idx = idx & 0xFF;  /* Ensure within bounds */
        
        /* Read-modify-write with bit manipulation */
        array[idx] = (array[idx] + combined) & 0x7FFFFFFF;
        
        /* Misaligned access simulation via pointer arithmetic */
        if ((i & 3) == 0) {
            char *byte_ptr = (char *)&array[idx];
            int *int_ptr = (int *)(byte_ptr + 1);  /* Potentially misaligned */
            *int_ptr = (*int_ptr ^ 0xAAAAAAAA) + i;
        }
        
        /* ===== CONTROL FLOW BASED ON BIT-FIELD RESULTS ===== */
        
        /* Branch based on bit-field parity */
        if (bf.flag1) {
            /* When flag1 is set */
            temp_results[0] += extracted;
            ll_var = ll_var | 0x00000000FFFFFFFFLL;
        } else {
            /* When flag1 is clear */
            temp_results[1] += bf.value;
            ll_var = ll_var & 0xFFFFFFFF00000000LL;
        }
        
        /* Switch based on flag2 */
        switch (bf.flag2) {
            case 0:
                array[(idx + 1) & 0xFF] = array[idx] * 2;
                break;
            case 1:
                array[(idx + 2) & 0xFF] = array[idx] / 2;
                break;
            case 2:
                dbl_var = dbl_var - 1.0;
                break;
            default:
                manip.parts.high16 = manip.parts.high16 ^ 0x5555;
                break;
        }
        
        /* Compare high vs low part of long long */
        if (high_part > low_part) {
            temp_results[2] += high_part - low_part;
        } else {
            temp_results[3] += low_part - high_part;
        }
        
        /* Accumulate sum for final result */
        sum += array[idx] + extracted + (int)dbl_var;
    }
    
    /* Final aggregation to prevent dead code elimination */
    int final_result = sum + temp_results[0] + temp_results[1] + 
                      temp_results[2] + temp_results[3] + 
                      (int)manip.full + (int)(ll_var >> 32) + 
                      (int)(ll_var & 0xFFFFFFFF) + (int)bf.value;
    
    /* Use result to affect return value */
    return final_result & 0xFF;
}

/* Additional function to create more RTL patterns during compilation */
void extra_patterns(void) {
    /* Inline assembly that might generate specific patterns */
    volatile long long ll_temp = 0x1122334455667788LL;
    volatile int result;
    
    /* Inline asm operating on low 32 bits only */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl $0x1234, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" ((int)ll_temp)
        : "%eax"
    );
    
    /* Structure with packed bit-fields */
    struct {
        unsigned short a : 5;
        unsigned short b : 6;
        unsigned short c : 5;
    } packed;
    
    packed.a = 0x1F;
    packed.b = 0x3F;
    packed.c = 0x1F;
    
    /* Access via volatile pointer */
    volatile unsigned short *ptr = (volatile unsigned short *)&packed;
    *ptr = (*ptr ^ 0xAAAA) & 0xFFFF;
}
