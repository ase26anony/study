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
KEEP_REGISTER static void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int padding : 8;
    } bf = {0};
    
    /* Variables to force register usage */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.field4 = (a & 0xF) + (b & 0x7);      /* 4-bit field with computation */
    bf.field8 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF);  /* 8-bit field */
    bf.field12 = (c & 0xFFF) | ((a & 0xF) << 8);        /* 12-bit field */
    
    /* Use __builtin_popcount on sub-word data */
    unsigned char byte_val = (unsigned char)(a & 0xFF);
    int popcnt = __builtin_popcount(byte_val);
    bf.field4 = popcnt & 0xF;  /* Store popcount result in bitfield */
    
    /* Prevent optimization */
    volatile unsigned int dummy = bf.field4 + bf.field8 + bf.field12;
    (void)dummy;
}

/* Test 2: SUBREG operations for narrowing stores */
KEEP_REGISTER static void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs;
    volatile signed char vc;
    
    /* Register sources with different sizes */
    register int ri = 0x12345678;
    register long rl = 0x9ABCDEF012345678UL;
    
    /* Explicit narrowing casts that may generate SUBREG */
    vs = (short)ri;                     /* int -> short */
    vc = (signed char)(ri >> 8);        /* int -> char */
    
    /* Arithmetic with implicit narrowing */
    unsigned char uc1 = 200;
    unsigned char uc2 = 100;
    unsigned char sum = uc1 + uc2;      /* May generate SUBREG for truncation */
    vc = sum;                           /* Store narrowed result */
    
    /* More complex narrowing */
    vs = (short)((ri * 3) / 7);         /* Computation then narrowing */
    
    /* Prevent optimization */
    volatile int dummy = vs + vc + sum;
    (void)dummy;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
KEEP_REGISTER static void test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8] = {{0}};
    
    /* Struct with array member */
    struct {
        int data[32];
        int offset;
    } s = {{0}, 5};
    
    /* Pointer with restrict to avoid aliasing assumptions */
    int *restrict ptr = arr[0];
    
    register int rval1 = 0xDEADBEEF;
    register int rval2 = 0xCAFEBABE;
    
    /* Complex address calculations */
    for (int i = 0; i < 8; i++) {
        /* Non-linear index calculation */
        int idx = i * 7 + 3;
        
        /* Store with complex addressing - may create MEM with complex address */
        arr[idx % 64][i] = rval1 + i;           /* 2D array access */
        ptr[idx * 2 + s.offset] = rval2 - i;    /* Pointer arithmetic with offset */
        
        /* Struct member access through pointer */
        int *sptr = s.data;
        sptr[(i * 3) % 32] = rval1 ^ rval2;     /* Array in struct with computed index */
    }
    
    /* Prevent optimization */
    volatile int dummy = arr[0][0] + arr[7][3] + s.data[0];
    (void)dummy;
}

/* Test 4: Combined patterns */
KEEP_REGISTER static void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 16;
        short values[16];
        unsigned int : 16; /* padding */
    } combined = {0};
    
    /* Register variables */
    register unsigned int reg_flags = 0xF0F0;
    register int reg_val = 0x12345678;
    
    /* Combined assignment: bitfield store (ZERO_EXTRACT) */
    combined.flags = (reg_flags & 0xFF) | ((reg_flags >> 8) & 0xFF);
    
    /* Combined assignment: array with narrowing (SUBREG) */
    for (int i = 0; i < 8; i++) {
        /* Complex index calculation */
        int idx = (i * 5 + 3) % 16;
        
        /* Store narrowed value to array element */
        combined.values[idx] = (short)(reg_val + i * 256);
        
        /* Additional bitfield assignment in loop */
        combined.flags = (combined.flags + 1) & 0xFFFF;
    }
    
    /* Prevent optimization */
    volatile int dummy = combined.flags + combined.values[0];
    (void)dummy;
}

/* Test 5: Inline assembly for direct RTL influence */
KEEP_REGISTER static void test_inline_asm(void) {
    int array[32] = {0};
    register int rval = 0x12345678;
    
    /* Inline asm with complex memory addressing */
    for (int i = 0; i < 4; i++) {
        int idx = i * 7 + 2;
        
        /* Direct memory store with complex addressing */
        asm volatile (
            "# Force memory store with complex address\n"
            : "=m" (array[idx * 2 % 32])  /* Complex addressing */
            : "r" (rval + i)              /* Register input */
            : "memory"
        );
    }
    
    /* Bitfield-like operation using inline asm */
    volatile unsigned int bitfield_target = 0;
    unsigned int source = 0xABCD;
    
    asm volatile (
        "# Bitfield-like store\n"
        : "=m" (*(volatile unsigned int(*)[1])&bitfield_target)
        : "r" (source & 0xFFF)  /* Mask to 12 bits */
        : "memory"
    );
    
    /* Prevent optimization */
    volatile int dummy = array[0] + bitfield_target;
    (void)dummy;
}

/* Main function with checksum to prevent dead code elimination */
int main(void) {
    unsigned int checksum = 0;
    
    /* Execute all tests */
    test_bitfield_operations();
    checksum += 1;
    
    test_subreg_operations();
    checksum += 2;
    
    test_complex_addressing();
    checksum += 3;
    
    test_combined_patterns();
    checksum += 4;
    
    test_inline_asm();
    checksum += 5;
    
    /* Print checksum to ensure all code executes */
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
