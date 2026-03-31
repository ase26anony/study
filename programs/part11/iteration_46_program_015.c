/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P(x) with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations */
static volatile int sink;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bf = {0};
    
    /* Source variables in registers */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.field4 = (a & 0xF) + (b & 0xF);          /* 4-bit field with computation */
    bf.field8 = (a >> 4) & 0xFF;                /* 8-bit extraction */
    bf.field12 = ((b & 0xFFF) ^ (c & 0xFFF));   /* 12-bit XOR operation */
    
    /* Bit manipulation builtins on sub-word data */
    unsigned char byte_val = (unsigned char)a;
    bf.field8 = __builtin_popcount(byte_val);   /* May involve bit extraction */
    
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: SUBREG operations through type narrowing */
void test_subreg_operations(void) {
    /* Volatile destination to prevent optimization */
    volatile short vs;
    volatile char vc;
    
    /* Register sources of different sizes */
    register int int_src = 0x12345678;
    register short short_src = 0xABCD;
    register long long_src = 0x123456789ABCDEF0ULL;
    
    /* Explicit narrowing casts that may generate SUBREG */
    vs = (short)int_src;                        /* int -> short */
    vc = (char)(short_src + 0x11);              /* short -> char with arithmetic */
    
    /* Implicit narrowing through arithmetic */
    char c1 = 100, c2 = 50;
    vc = c1 + c2;                               /* char addition with truncation */
    
    /* 64-bit to 32-bit narrowing */
    volatile int vi;
    vi = (int)long_src;                         /* long long -> int */
    
    sink = vs + vc + vi;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Local arrays with restrict to prevent aliasing assumptions */
    int arr1[256] __attribute__((aligned(16)));
    short arr2[512] __attribute__((aligned(8)));
    
    /* Multi-dimensional access simulation */
    int stride = 16;
    register int val1 = 0xDEADBEEF;
    register short val2 = 0xCAFE;
    
    /* Complex index calculations */
    for (int i = 0; i < 16; i++) {
        /* Non-linear index computation */
        int idx1 = (i * 3 + 7) & 0xFF;
        int idx2 = (i * 5 + 13) & 0x1FF;
        
        /* Store with complex addressing */
        arr1[idx1] = val1 + i;                  /* Array with computed index */
        arr2[idx2] = (short)(val2 - i);         /* Short array with narrowing */
    }
    
    /* Pointer arithmetic with multiple offsets */
    int *ptr1 = arr1 + 32;
    *(ptr1 + 8) = 0x12345678;                   /* Base + offset */
    
    /* Struct-like access through pointer */
    struct {
        int header;
        short data[32];
    } s __attribute__((aligned(8)));
    
    s.data[10] = 0x55AA;                        /* Struct member array access */
    
    /* Compute checksum to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += arr1[i] + arr2[i];
    }
    sink = sum + s.data[10];
}

/* Test 4: Combined patterns in single assignments */
void test_combined_patterns(void) {
    /* Struct with mixed members */
    volatile struct {
        unsigned int flags : 8;
        short values[8];
        int counter;
    } combined __attribute__((aligned(8)));
    
    register int r1 = 0x89ABCDEF;
    register short r2 = 0x5678;
    
    /* Combined: bitfield store with computation */
    combined.flags = (r1 & 0xFF) | ((r2 >> 4) & 0xF);
    
    /* Combined: array store with narrowing and complex index */
    int idx = (r1 & 0x7) * 2;                   /* 0, 2, 4, 6, 8, 10, 12, 14 */
    combined.values[idx] = (short)(r1 >> 16);   /* Narrowing store to array */
    
    /* Inline assembly for direct RTL influence */
    int temp_array[4] = {0};
    int complex_idx = (r1 & 0x3) * 3;
    
    /* asm with memory output constraint and complex addressing */
    asm volatile (
        "# Force memory store with addressing\n"
        : "=m" (temp_array[complex_idx])        /* Complex memory destination */
        : 
        : "memory"
    );
    
    sink = combined.flags + combined.values[0] + temp_array[0];
}

/* Test 5: Additional patterns for coverage */
void test_additional_patterns(void) {
    /* STRICT_LOW_PART may appear for partial word stores */
    volatile uint32_t word;
    volatile uint16_t halfword;
    
    register uint32_t reg32 = 0x87654321;
    
    /* Store low 16 bits - may generate STRICT_LOW_PART */
    halfword = (uint16_t)reg32;
    
    /* Bitfield with exactly half register size */
    struct {
        uint32_t low16 : 16;
        uint32_t high16 : 16;
    } split __attribute__((aligned(4)));
    
    split.low16 = reg32 & 0xFFFF;
    split.high16 = reg32 >> 16;
    
    /* Memory store with scaled index */
    int matrix[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = (i * 4 + j) * 2;     /* 2D array with computation */
        }
    }
    
    sink = halfword + split.low16 + matrix[2][2];
}

int main(void) {
    int total = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    total += sink;
    
    test_subreg_operations();
    total += sink;
    
    test_complex_addressing();
    total += sink;
    
    test_combined_patterns();
    total += sink;
    
    test_additional_patterns();
    total += sink;
    
    /* Final checksum to ensure all code executed */
    printf("Final checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
