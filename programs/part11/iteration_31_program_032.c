/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 12;
    unsigned int pad   : 16;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_struct bits;
    unsigned int raw;
    unsigned short parts[2];
};

/* Function to force complex addressing modes */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile struct bitfield_struct bf = {0, 0, 0, 0};
    volatile union mixed_access ma = {{0, 0, 0, 0}};
    
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
    
    /* Loop with volatile control to prevent optimization */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 10) loop_limit = 10;
    if (loop_limit > 1000) loop_limit = 1000;
    
    /* Main loop with mixed operations */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* ============================================
         * BIT-FIELD OPERATIONS for ZERO_EXTRACT/STRICT_LOW_PART
         * ============================================ */
        
        /* Direct bit-field assignment - may generate STRICT_LOW_PART */
        bf.flag1 = (i & 1);
        bf.flag2 = (i & 7);
        bf.value = (i * 3) & 0xFFF;
        
        /* Bit-field extraction using masking - may generate ZERO_EXTRACT */
        unsigned int extracted = (ma.bits.value << 4) & 0xFFF0;
        
        /* Complex bit-field manipulation */
        ma.bits.flag1 = (extracted >> 11) & 1;
        ma.bits.flag2 = (ma.raw >> 1) & 0x7;
        
        /* Manual bit extraction and insertion */
        unsigned int mask = 0xF00;
        unsigned int field = (ma.raw & mask) >> 8;
        ma.raw = (ma.raw & ~mask) | ((field + 1) << 8);
        
        /* ============================================
         * MULTI-WORD OPERATIONS for SUBREG generation
         * ============================================ */
        
        /* Operations on long long - may generate SUBREG for 32-bit targets */
        ll_var = ll_var + 0x100000001LL;
        ll_var = ll_var | 0x5555555555555555LL;
        
        /* Access high and low parts separately */
        unsigned int low_part = (unsigned int)(ll_var & 0xFFFFFFFF);
        unsigned int high_part = (unsigned int)(ll_var >> 32);
        
        /* Swap halves and reconstruct */
        ll_var = ((long long)low_part << 32) | high_part;
        
        /* Double precision operations */
        dbl_var = dbl_var * 1.01;
        dbl_var = dbl_var + (double)i;
        
        /* Mix double and integer operations */
        ll_var = (long long)(dbl_var * 100.0);
        
        /* ============================================
         * COMPLEX MEMORY ADDRESSING
         * ============================================ */
        
        /* Array access with complex indexing */
        int idx = complex_index(i, 7, 3) & 0xFF;
        int idx2 = complex_index(i, 5, ma.bits.value) & 0xFF;
        
        /* Read-modify-write with bit manipulation */
        array[idx] = array[idx] ^ (1 << (i & 0xF));
        array[idx2] = array[idx2] + ((ma.raw >> 4) & 0xF);
        
        /* Misaligned access simulation via pointer arithmetic */
        unsigned char *byte_ptr = (unsigned char *)&array[idx];
        unsigned int word = (byte_ptr[0] << 24) | (byte_ptr[1] << 16) | 
                           (byte_ptr[2] << 8) | byte_ptr[3];
        word = (word >> ma.bits.flag2) | (word << (32 - ma.bits.flag2));
        
        /* ============================================
         * CONTROL FLOW based on bit-field results
         * ============================================ */
        
        /* Branch on bit-field conditions */
        if (ma.bits.flag1) {
            /* When flag1 is set */
            temp_results[0] += array[idx];
            ll_var = ll_var - 0x1000;
        } else {
            /* When flag1 is clear */
            temp_results[1] += array[idx2];
            ll_var = ll_var + 0x1000;
        }
        
        /* Branch on comparison of long long halves */
        if (low_part > high_part) {
            dbl_var = dbl_var / 2.0;
            temp_results[2] ^= (int)ll_var;
        } else if (low_part < high_part) {
            dbl_var = dbl_var * 2.0;
            temp_results[3] |= (int)(ll_var >> 32);
        }
        
        /* Switch based on bit-field value */
        switch (ma.bits.flag2) {
            case 0:
                array[idx] = array[idx] * 2;
                break;
            case 1:
                array[idx] = array[idx] / 2;
                break;
            case 2:
                array[idx] = array[idx] ^ 0xAAAA;
                break;
            case 3:
                array[idx] = array[idx] | 0x5555;
                break;
            default:
                array[idx] = array[idx] & 0xFF;
                break;
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : : "r"(i) : "memory");
    }
    
    /* ============================================
     * AGGREGATE RESULTS to prevent dead code elimination
     * ============================================ */
    
    /* Compute checksum of results */
    unsigned int checksum = 0;
    
    /* Include bit-field state */
    checksum ^= ma.raw;
    checksum ^= bf.flag1 << 0;
    checksum ^= bf.flag2 << 1;
    checksum ^= bf.value << 4;
    
    /* Include long long state */
    checksum ^= (unsigned int)ll_var;
    checksum ^= (unsigned int)(ll_var >> 32);
    
    /* Include array elements */
    for (int i = 0; i < 16; i++) {
        checksum ^= array[i * 13];
    }
    
    /* Include temp results */
    for (int i = 0; i < 4; i++) {
        checksum += temp_results[i];
    }
    
    /* Include double via integer representation */
    checksum ^= *((unsigned int *)&dbl_var);
    checksum ^= *(((unsigned int *)&dbl_var) + 1);
    
    /* Print result to prevent optimization */
    printf("Result checksum: %u\n", checksum);
    
    return (int)(checksum & 0xFF);
}

/* Additional function to force more RTL patterns */
void __attribute__((noinline)) extra_patterns(void) {
    volatile struct {
        unsigned int a : 5;
        unsigned int b : 10;
        unsigned int c : 17;
    } s = {0, 0, 0};
    
    volatile long long ll1 = 0, ll2 = 0;
    
    /* Force ZERO_EXTRACT patterns */
    for (int i = 0; i < 50; i++) {
        s.a = (s.a + 1) & 0x1F;
        s.b = (s.b * 3) & 0x3FF;
        s.c = (s.c + s.a + s.b) & 0x1FFFF;
        
        /* Extract and combine bit-fields */
        unsigned int combined = (s.a << 22) | (s.b << 12) | s.c;
        
        /* Force SUBREG operations */
        ll1 = (ll1 << 1) | (combined & 1);
        ll2 = ll2 + (ll1 & 0xFFFFFFFF);
        
        /* Complex memory operation with bit extraction */
        volatile unsigned int *ptr = &combined;
        *ptr = (*ptr & ~0xFF) | ((*ptr >> 8) & 0xFF);
    }
}
