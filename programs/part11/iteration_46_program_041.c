/* test_resource_marking.c
 * Designed to generate RTL patterns that trigger specific uncovered lines
 * in GCC's resource.cc (lines 282-290)
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations in registers */
#define KEEP_REGISTER __attribute__((noinline))

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
KEEP_REGISTER
static int test_bitfield_ops(void) {
    volatile struct {
        unsigned int field4 : 4;    /* Likely ZERO_EXTRACT for 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bitfields = {0};
    
    /* Use volatile source to prevent constant propagation */
    volatile unsigned int source_a = 0xABCD;
    volatile unsigned int source_b = 0x1234;
    
    /* Complex bitfield assignment - may generate ZERO_EXTRACT in SET_DEST */
    bitfields.field4 = (source_a & 0xF) + (source_b & 0xF);
    bitfields.field8 = (source_a >> 4) & 0xFF;
    bitfields.field12 = ((source_a & 0xFFF) ^ (source_b & 0xFFF)) + 1;
    
    /* Read back to prevent elimination */
    return bitfields.field4 + bitfields.field8 + bitfields.field12;
}

/* Test 2: Sub-word type operations for SUBREG generation */
KEEP_REGISTER
static int test_subreg_ops(void) {
    volatile short vs1, vs2, vs3;
    volatile char vc1, vc2;
    
    /* Use register variables to force register operations */
    register int r1 asm("r12") = 0x12345678;
    register int r2 asm("r13") = 0x9ABCDEF0;
    register int r3 asm("r14") = 0;
    
    /* Explicit narrowing casts - may generate SUBREG in SET_DEST */
    vs1 = (short)r1;                    /* int -> short */
    vs2 = (short)(r1 + r2);             /* expression with narrowing */
    vs3 = (short)((r1 & 0xFFFF) | ((r2 >> 16) & 0xFFFF));
    
    /* char operations with implicit truncation */
    vc1 = (char)(r1 * 3);               /* multiplication with truncation */
    vc2 = (char)((r1 & 0xFF) + (r2 & 0xFF));
    
    /* Force r3 to be used */
    r3 = vs1 + vs2 + vs3 + vc1 + vc2;
    
    return r3;
}

/* Test 3: Complex memory addressing for MEM_P(x) path */
KEEP_REGISTER
static int test_complex_mem_ops(void) {
    /* Use restrict to help compiler with aliasing */
    int array[256] __attribute__((aligned(16)));
    int *restrict ptr = array;
    
    /* Initialize with pattern */
    for (int i = 0; i < 256; i++) {
        ptr[i] = i * 3;
    }
    
    register int rval asm("r15") = 0x87654321;
    int sum = 0;
    
    /* Complex addressing patterns */
    for (int i = 0; i < 64; i++) {
        /* Non-linear index calculation */
        int idx = (i * 7 + 13) & 0xFF;
        
        /* Store with complex address computation */
        ptr[idx * 2] = (short)rval;          /* SUBREG in MEM destination */
        ptr[(idx ^ 0x55) + 3] = rval & 0xFF; /* ZERO_EXTRACT in MEM? */
        
        /* Multi-dimensional style addressing */
        int idx2 = ((i & 0xF) << 4) | (i & 0xF);
        ptr[idx2] = ptr[idx2] + (rval >> (i & 0x1F));
    }
    
    /* Compute checksum */
    for (int i = 0; i < 256; i++) {
        sum += ptr[i];
    }
    
    return sum;
}

/* Test 4: Combined patterns in struct context */
KEEP_REGISTER
static int test_combined_patterns(void) {
    struct combined {
        volatile unsigned int flags : 16;
        volatile short data[8];
        volatile char small[16];
    } cmb;
    
    /* Initialize */
    for (int i = 0; i < 8; i++) cmb.data[i] = i * 100;
    for (int i = 0; i < 16; i++) cmb.small[i] = i;
    
    register int rbase asm("r11") = 0x13579BDF;
    int result = 0;
    
    /* Combined assignment: bitfield + array with complex index */
    for (int i = 0; i < 8; i++) {
        /* Update bitfield - may use ZERO_EXTRACT */
        cmb.flags = (cmb.flags ^ (rbase >> i)) & 0xFFFF;
        
        /* Array store with narrowing - may use SUBREG */
        int complex_idx = (i * 5 + 7) & 0x7;
        cmb.data[complex_idx] = (short)(rbase + i * 0x100);
        
        /* Char store with address computation */
        cmb.small[i * 2] = (char)((rbase >> (i * 4)) & 0xFF);
    }
    
    /* Compute result */
    result = cmb.flags;
    for (int i = 0; i < 8; i++) result += cmb.data[i];
    for (int i = 0; i < 16; i++) result += cmb.small[i];
    
    return result;
}

/* Test 5: Inline assembly for direct RTL influence */
KEEP_REGISTER
static int test_asm_patterns(void) {
    int array[32] __attribute__((aligned(8)));
    volatile short vs;
    volatile struct { unsigned int f:8; } bf;
    
    /* Initialize */
    for (int i = 0; i < 32; i++) array[i] = i;
    
    int sum = 0;
    
    /* Inline asm with complex memory destination */
    for (int i = 0; i < 16; i++) {
        int idx = (i * 3 + 5) & 0x1F;
        
        /* Memory output with complex addressing */
        asm volatile (
            "# Force memory store with complex address\n"
            : "=m" (array[idx * 2 + 1])  /* Complex index calculation */
            : 
            : "memory"
        );
        
        /* Another asm to hint at bitfield store */
        asm volatile (
            "# Hint at bitfield operation\n"
            : "=m" (bf.f)
            : 
            : "memory"
        );
    }
    
    /* Use the values */
    bf.f = 0xAA;
    vs = (short)array[10];
    
    for (int i = 0; i < 32; i++) sum += array[i];
    sum += bf.f + vs;
    
    return sum;
}

/* Test 6: Bit manipulation builtins with sub-word types */
KEEP_REGISTER
static int test_builtin_bitops(void) {
    volatile unsigned short data[4] = {0x1234, 0x5678, 0x9ABC, 0xDEF0};
    volatile unsigned char bytes[8];
    int result = 0;
    
    /* Builtins that may involve bit extraction */
    for (int i = 0; i < 4; i++) {
        /* __builtin_popcount on sub-word data */
        bytes[i*2] = __builtin_popcount(data[i] & 0xFF);
        bytes[i*2+1] = __builtin_parity(data[i] >> 8);
        
        /* Complex bitfield-like operation */
        unsigned short temp = data[i];
        temp = ((temp & 0xF0F0) >> 4) | ((temp & 0x0F0F) << 4);
        data[i] = temp;
    }
    
    /* Compute result */
    for (int i = 0; i < 4; i++) result += data[i];
    for (int i = 0; i < 8; i++) result += bytes[i];
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Run all tests */
    checksum += test_bitfield_ops();
    checksum += test_subreg_ops();
    checksum += test_complex_mem_ops();
    checksum += test_combined_patterns();
    checksum += test_asm_patterns();
    checksum += test_builtin_bitops();
    
    printf("Final checksum: %d\n", checksum);
    
    /* Use checksum to prevent dead code elimination */
    if (checksum > 0) {
        return 0;
    } else {
        return 1;
    }
}
