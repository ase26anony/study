/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bit_packed {
    unsigned int field1 : 3;   /* 3-bit field */
    unsigned int field2 : 12;  /* 12-bit field */
    unsigned int field3 : 1;   /* 1-bit field */
    unsigned int field4 : 8;   /* 8-bit field */
    unsigned int field5 : 7;   /* 7-bit field */
    unsigned int : 1;          /* padding */
};

/* Union for type-punning to force specific bit manipulations */
union data_manip {
    unsigned int full;
    struct {
        unsigned short low;
        unsigned short high;
    } parts;
};

/* Function to create complex addressing patterns */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile struct bit_packed bp = {0};
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile union data_manip dm = {0};
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Use argc to control loop iterations (prevents compile-time optimization) */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    int result = 0;
    
    /* Main loop with mixed operations */
    for (volatile int i = 0; i < iterations; i++) {
        /* 1. Bit-field manipulations (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        bp.field1 = (i & 0x7);                     /* 3-bit assignment */
        bp.field2 = (i * 17) & 0xFFF;              /* 12-bit assignment */
        bp.field3 = (i & 0x1);                     /* 1-bit assignment */
        
        /* Extract and combine bit-fields */
        unsigned int temp = bp.field2;
        temp = (temp << 3) | bp.field1;            /* Combine fields */
        bp.field4 = temp & 0xFF;                   /* 8-bit assignment */
        bp.field5 = (temp >> 8) & 0x7F;            /* 7-bit assignment */
        
        /* 2. Multi-word operations (potential SUBREG generation) */
        ll_var += (long long)(bp.field2 * 1001);
        ll_var ^= (long long)i << 32;              /* Mix high and low parts */
        
        dbl_var *= 1.01;
        dbl_var += (double)(i & 0xF) * 0.1;
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = complex_index(i, bp.field1 + 1, bp.field2 & 0x3F);
        idx &= 0xFF;                               /* Ensure in bounds */
        
        /* Read-modify-write with bit manipulation */
        array[idx] ^= (bp.field4 << 8) | bp.field5;
        array[(idx + 128) & 0xFF] += (bp.field3 ? 1 : -1) * i;
        
        /* 4. Type-punning with union (can create interesting RTL) */
        dm.full = array[idx];
        dm.parts.low ^= dm.parts.high;
        dm.parts.high += i;
        array[idx] = dm.full;
        
        /* 5. Conditional based on multi-word comparison */
        if ((ll_var & 0xFFFFFFFF) > (ll_var >> 32)) {
            /* Low word > high word */
            array[idx] >>= 1;
            bp.field1 = (bp.field1 + 1) & 0x7;
        } else {
            /* High word >= low word */
            array[idx] <<= 1;
            bp.field5 = (bp.field5 + 1) & 0x7F;
        }
        
        /* 6. Additional bit-field extraction with masking */
        unsigned int combined = (bp.field2 << 12) | (bp.field4 << 4) | bp.field1;
        result ^= combined;                        /* Accumulate result */
        
        /* 7. Force potential STRICT_LOW_PART through explicit masking */
        unsigned int mask_low = 0x0000FFFF;
        dm.full = (dm.full & mask_low) | ((i & 0xFFFF) << 16);
        array[(idx + 64) & 0xFF] = dm.full;
    }
    
    /* Final aggregation to prevent dead code elimination */
    for (int i = 0; i < 256; i++) {
        result += array[i];
    }
    
    result += (int)ll_var;
    result += (int)(ll_var >> 32);
    result += (int)(dbl_var * 1000);
    result += bp.field1 + bp.field2 + bp.field3 + bp.field4 + bp.field5;
    
    /* Print result to ensure side effects */
    printf("Result: %d\n", result);
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}

/* Additional function with inline assembly for specific patterns */
void asm_helpers(void) {
    volatile long long ll_input = 0x123456789ABCDEF0LL;
    volatile int output;
    
    /* Inline assembly that might generate SUBREG patterns */
    __asm__ volatile (
        "movl %1, %%eax\n\t"           /* Get low 32 bits */
        "addl $0x100, %%eax\n\t"       /* Operate on low part */
        "movl %%eax, %0\n\t"           /* Store result */
        : "=r" (output)
        : "m" (ll_input)
        : "%eax"
    );
    
    /* Another asm with explicit register constraints */
    volatile int a = 42, b = 17;
    __asm__ volatile (
        "imull %1, %0\n\t"             /* Multiply, might use SUBREG for 64-bit result */
        : "+r" (a)
        : "r" (b)
        : "cc"
    );
}
