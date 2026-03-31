/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations in registers */
#define KEEP_REGISTER __attribute__((noinline))

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
KEEP_REGISTER
static int test_bitfield_ops(void) {
    volatile struct {
        unsigned int field4 : 4;    /* Likely ZERO_EXTRACT */
        unsigned int field8 : 8;    /* For sub-byte extraction */
        unsigned int field12 : 12;  /* Multi-byte bitfield */
    } bf = {0};
    
    /* Use volatile source to prevent constant propagation */
    volatile unsigned int source = 0xABCDEF12;
    volatile unsigned int mask = 0xF;
    
    /* Multiple bitfield assignments that may generate ZERO_EXTRACT */
    bf.field4 = (source >> 0) & 0xF;      /* Extract low nibble */
    bf.field8 = (source >> 4) & 0xFF;     /* Extract byte */
    bf.field12 = (source >> 8) & 0xFFF;   /* Extract 12 bits */
    
    /* Bitfield assignment with computation */
    bf.field4 = ((source & 0xF) + (mask & 0xF)) & 0xF;
    
    /* Use bit manipulation builtins that may involve extraction */
    unsigned int temp = __builtin_popcount(source & 0xFF);
    bf.field8 = temp & 0xFF;
    
    return bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: SUBREG operations through type narrowing */
KEEP_REGISTER  
static int test_subreg_ops(void) {
    /* Volatile destination forces store generation */
    volatile short vs;
    volatile char vc;
    
    /* Register variables encourage SUBREG in SET_DEST */
    register int reg_int = 0x12345678;
    register short reg_short = 0xABCD;
    
    /* Narrowing assignments that may create SUBREG */
    vs = (short)reg_int;                     /* int -> short */
    vc = (char)(reg_int >> 16);              /* int -> char */
    
    /* Arithmetic with implicit narrowing */
    char c1 = 100, c2 = 50;
    vc = c1 + c2;                            /* char + char -> char */
    
    /* Complex expression with narrowing */
    vs = (short)((reg_int & 0xFFFF) + (reg_short & 0xFF));
    
    /* Pointer-based narrowing */
    int *int_ptr = &reg_int;
    vs = *(short *)int_ptr;                  /* Load as short */
    
    return vs + vc;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
KEEP_REGISTER
static int test_complex_mem_ops(void) {
    /* Multi-dimensional array for complex indexing */
    int arr[64][8] = {0};
    int sum = 0;
    
    /* Use restrict to help compiler with aliasing */
    int *restrict base = &arr[0][0];
    
    /* Complex address calculations */
    for (int i = 0; i < 16; i++) {
        /* Non-linear index calculation */
        int idx = (i * 3 + 7) & 63;
        int idx2 = (i * 5 + 11) & 7;
        
        /* Store with complex addressing */
        arr[idx][idx2] = i * 100;
        
        /* Pointer arithmetic with multiple offsets */
        *(base + idx * 8 + idx2) = i * 200;
    }
    
    /* Struct with array member */
    struct {
        int data[32];
        int offset;
    } s = {{0}, 8};
    
    /* Access through struct pointer */
    struct s *sp = &s;
    for (int i = 0; i < 16; i++) {
        sp->data[i * 2 + sp->offset] = i * 300;
    }
    
    /* Compute checksum to prevent elimination */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum;
}

/* Test 4: Combined patterns */
KEEP_REGISTER
static int test_combined_ops(void) {
    /* Struct containing both bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        short values[16];
    } combined = {0};
    
    int temp[16];
    register int reg_val = 0x89ABCDEF;
    
    /* Combined: bitfield assignment (ZERO_EXTRACT) */
    combined.flags = (reg_val >> 4) & 0xFF;
    
    /* Combined: array with complex index and narrowing (SUBREG) */
    for (int i = 0; i < 8; i++) {
        /* Complex index calculation */
        int idx = (i * 7 + 3) & 15;
        
        /* Narrowing store to short array (potential SUBREG) */
        combined.values[idx] = (short)(reg_val + i * 100);
        
        /* Also store to temp for checksum */
        temp[idx] = reg_val + i * 100;
    }
    
    /* Inline assembly to directly influence RTL generation */
    int dummy;
    asm volatile (
        /* Memory output with complex addressing */
        "movl %1, %0\n\t"
        : "=m" (combined.values[5])  /* Complex addressing via array */
        : "r" (reg_val)
        : "memory"
    );
    
    /* Compute checksum */
    int sum = combined.flags;
    for (int i = 0; i < 16; i++) {
        sum += combined.values[i];
    }
    
    return sum;
}

/* Test 5: Additional patterns for STRICT_LOW_PART */
KEEP_REGISTER
static int test_strict_low_part(void) {
    /* Operations that might generate STRICT_LOW_PART */
    volatile unsigned short low_part;
    volatile unsigned char low_byte;
    
    register unsigned int r1 = 0x87654321;
    register unsigned int r2 = 0xFEDCBA98;
    
    /* Operations that preserve only low parts */
    low_part = (unsigned short)(r1 + r2);      /* Only low 16 bits matter */
    low_byte = (unsigned char)(r1 * r2);       /* Only low 8 bits matter */
    
    /* Bitwise operations with masking */
    low_part = (r1 & 0xFFFF) | (r2 & 0xFF00);
    low_byte = (r1 & 0xFF) ^ (r2 & 0xFF);
    
    return low_part + low_byte;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Execute all tests sequentially */
    checksum += test_bitfield_ops();
    checksum += test_subreg_ops();
    checksum += test_complex_mem_ops();
    checksum += test_combined_ops();
    checksum += test_strict_low_part();
    
    printf("Final checksum: %d\n", checksum);
    printf("(Non-zero indicates all code executed)\n");
    
    return checksum != 0 ? 0 : 1;
}
