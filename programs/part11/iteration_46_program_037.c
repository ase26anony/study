/* test_resource_marking.c
 * Designed to generate RTL patterns that exercise uncovered lines in resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P(x) with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations in registers */
#define KEEP_REGISTER __attribute__((noinline))

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
KEEP_REGISTER static unsigned int test_bitfields(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int padding : 8;
    } bf = {0};
    
    /* Register variables to force register-to-memory operations */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.field4 = (a & 0xF) + (b & 0x1);      /* 4-bit extract and store */
    bf.field8 = ((a >> 4) & 0xFF) ^ (c & 0xFF); /* 8-bit extract with computation */
    bf.field12 = (a + b) & 0xFFF;           /* 12-bit masked store */
    
    /* Use bit-manipulation builtins on sub-word data */
    unsigned int parity_val = __builtin_parity(bf.field8);
    bf.field4 = parity_val & 0xF;
    
    /* Read back to prevent elimination */
    return bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: Sub-word type operations to generate SUBREG */
KEEP_REGISTER static unsigned int test_subreg(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    
    /* Register sources of different sizes */
    register int ri1 = 0x12345678;
    register int ri2 = 0x9ABCDEF0;
    register short rs1 = 0x1234;
    register char rc1 = 0x56;
    
    /* Explicit casts that may generate SUBREG in SET_DEST */
    vs1 = (short)ri1;                     /* int -> short truncation */
    vs2 = (short)(ri1 + ri2);             /* arithmetic with truncation */
    
    /* Implicit narrowing with overflow */
    vc1 = rc1 + 0x80;                     /* char addition with truncation */
    vc2 = (rs1 >> 4) & 0xFF;              /* shift and mask to char */
    
    /* Combined operation with multiple SUBREGs */
    short temp = (short)((ri1 & 0xFFFF) + (ri2 & 0xFFFF));
    vs1 = temp;
    
    /* Read back to prevent elimination */
    return vs1 + vs2 + vc1 + vc2;
}

/* Test 3: Complex memory addressing to trigger MEM_P(x) path */
KEEP_REGISTER static unsigned int test_complex_mem(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8] __attribute__((aligned(64)));
    
    /* Restrict pointer to avoid aliasing assumptions */
    int *restrict ptr = &arr[0][0];
    
    register int r1 = 0x11111111;
    register int r2 = 0x22222222;
    register int r3 = 0x33333333;
    
    /* Complex address calculations */
    for (int i = 0; i < 8; i++) {
        /* Non-linear index computation */
        int idx = i * 7 + 3;                     /* i*stride + offset */
        ptr[idx] = r1 + i;                       /* Store with complex address */
        
        /* Multi-dimensional access */
        arr[i][(i * 3) % 8] = r2 - i;            /* 2D array with modulo */
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr + i * 8 + (i & 3)) = r3 ^ i;       /* base + offset1 + offset2 */
    }
    
    /* Compute checksum */
    unsigned int sum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    return sum;
}

/* Test 4: Combined patterns in single assignments */
KEEP_REGISTER static unsigned int test_combined(void) {
    /* Struct with mixed members */
    struct combined {
        volatile unsigned int bitfield : 6;
        volatile short short_array[16];
        int padding;
    } cmb __attribute__((aligned(32)));
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        cmb.short_array[i] = 0;
    }
    
    register int reg_val = 0x89ABCDEF;
    register short reg_short = 0x1234;
    register int idx_reg = 5;
    
    /* Combined: bitfield store with computation */
    cmb.bitfield = (reg_val & 0x3F) + (idx_reg & 0xF);
    
    /* Combined: sub-word array store with complex index */
    int complex_idx = (idx_reg * 3 + 7) & 0xF;    /* Non-linear computation */
    cmb.short_array[complex_idx] = (short)(reg_val >> 8);  /* SUBREG + complex MEM */
    
    /* Additional complex addressing through pointer */
    volatile short *ptr = &cmb.short_array[0];
    ptr += (complex_idx ^ 3);                     /* Pointer arithmetic */
    *ptr = reg_short;                             /* Store through computed pointer */
    
    /* Read back all values */
    unsigned int sum = cmb.bitfield;
    for (int i = 0; i < 16; i++) {
        sum += cmb.short_array[i];
    }
    return sum;
}

/* Test 5: Inline assembly for direct RTL influence */
KEEP_REGISTER static unsigned int test_asm(void) {
    int array[32] __attribute__((aligned(32)));
    register int rval = 0xDEADBEEF;
    unsigned int sum = 0;
    
    /* Use inline assembly to create complex memory destinations */
    for (int i = 0; i < 8; i++) {
        int idx = i * 3 + 1;
        
        /* Assembly with memory output and complex addressing */
        asm volatile (
            "# Force memory store with complex address\n"
            : "=m" (array[idx])      /* Complex addressing in output constraint */
            : "r" (rval + i)         /* Register input */
            : "memory"
        );
        
        /* Another with bit manipulation */
        asm volatile (
            "# Another store pattern\n"
            : "=m" (array[(i * 5) & 31])
            : "r" (rval - i)
            : "memory"
        );
    }
    
    /* Compute sum */
    for (int i = 0; i < 32; i++) {
        sum += array[i];
    }
    return sum;
}

int main(void) {
    unsigned int checksum = 0;
    
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Run all tests sequentially */
    checksum += test_bitfields();
    printf("Bitfield test complete\n");
    
    checksum += test_subreg();
    printf("SUBREG test complete\n");
    
    checksum += test_complex_mem();
    printf("Complex memory test complete\n");
    
    checksum += test_combined();
    printf("Combined test complete\n");
    
    checksum += test_asm();
    printf("Assembly test complete\n");
    
    /* Final checksum to ensure all code executed */
    printf("Final checksum: 0x%08X\n", checksum);
    
    /* Use checksum to prevent dead code elimination */
    volatile unsigned int sink = checksum;
    (void)sink;
    
    return 0;
}
