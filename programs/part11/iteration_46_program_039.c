/* test_resource_coverage.c
 * Targets uncovered lines 282-290 in resource.cc
 * Generates RTL with ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P patterns
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations in registers */
#define KEEP_REGISTER __attribute__((noinline))

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
static KEEP_REGISTER void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bf = {0};
    
    /* Register variables for source computations */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.field4 = (a & 0xF) + (b & 0xF);          /* Should use ZERO_EXTRACT */
    bf.field8 = ((a >> 4) & 0xFF) ^ (c & 0xFF); /* Complex expression */
    bf.field12 = (a + b) & 0xFFF;               /* Masked sum */
    
    /* Use __builtin_popcount on sub-word data */
    unsigned char byte_val = (a >> 8) & 0xFF;
    bf.field4 = __builtin_popcount(byte_val) & 0xF;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(bf.field4), "r"(bf.field8), "r"(bf.field12));
}

/* Test 2: SUBREG patterns from type narrowing */
static KEEP_REGISTER void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs;
    volatile signed char vc;
    volatile unsigned char vuc;
    
    /* Register sources */
    register int r1 = 0x12345678;
    register unsigned int r2 = 0x9ABCDEF0;
    register long r3 = 0x1122334455667788LL;
    
    /* Explicit narrowing casts - may generate SUBREG in SET_DEST */
    vs = (short)r1;                    /* int -> short */
    vc = (signed char)(r1 + r2);       /* expression narrowing */
    vuc = (unsigned char)(r2 ^ 0xFF);  /* bitwise op with narrowing */
    
    /* Arithmetic with implicit narrowing */
    char c1 = 100, c2 = 50;
    volatile char vc2;
    vc2 = c1 + c2;                     /* char + char -> char (overflow) */
    
    /* Prevent optimization */
    asm volatile("" : : "r"(vs), "r"(vc), "r"(vuc), "r"(vc2));
}

/* Test 3: Complex memory addressing for MEM_P(x) */
static KEEP_REGISTER void test_complex_memory_addressing(void) {
    /* Multi-dimensional array */
    int arr2d[16][16];
    
    /* Struct with array */
    struct {
        int data[32];
        int offset;
    } s = {0};
    
    /* Pointer with restrict to prevent aliasing assumptions */
    int *restrict ptr = arr2d[0];
    
    register int rval = 0xDEADBEEF;
    register int idx1 = 7, idx2 = 11;
    
    /* Complex addressing patterns */
    
    /* 1. Multi-dimensional array with non-linear index */
    arr2d[idx1 * 2 + 3][idx2 - 4] = rval;
    
    /* 2. Pointer arithmetic with multiple offsets */
    *(ptr + idx1 * 8 + idx2) = rval ^ 0x12345678;
    
    /* 3. Struct member through pointer with index arithmetic */
    int *data_ptr = s.data;
    data_ptr[s.offset + idx1 * 3] = rval + idx2;
    
    /* 4. Nested array access */
    int arr3d[4][4][4];
    arr3d[idx1 & 3][idx2 & 3][(idx1 + idx2) & 3] = rval;
    
    /* Prevent optimization */
    asm volatile("" : : "m"(arr2d[0][0]), "m"(s.data[0]), "m"(arr3d[0][0][0]));
}

/* Test 4: Combined patterns in single assignments */
static KEEP_REGISTER void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct combined {
        unsigned int flags : 8;
        unsigned short values[8];
        int padding;
    } comb = {0};
    
    register unsigned int reg1 = 0x12345678;
    register int reg2 = 0x9ABCDEF0;
    register short reg3 = 0x2468;
    
    /* Combined assignment 1: Bitfield from complex expression */
    comb.flags = ((reg1 & 0xFF) + (reg2 & 0xFF)) & 0x7F;
    
    /* Combined assignment 2: Array element with narrowing cast */
    int complex_idx = (reg1 & 0x7) * 3 + 1;
    comb.values[complex_idx & 0x7] = (short)(reg1 + reg2);
    
    /* Combined assignment 3: Multiple operations in sequence */
    comb.values[0] = (short)reg1;
    comb.flags = (reg2 >> 8) & 0xFF;
    comb.values[1] = reg3;
    
    /* Inline assembly with memory constraint */
    asm volatile("# Force memory operand" : "=m"(comb.values[2]) : "r"(reg3));
    
    /* Prevent optimization */
    asm volatile("" : : "r"(comb.flags), "m"(comb.values[0]));
}

/* Test 5: Direct inline assembly for RTL control */
static KEEP_REGISTER void test_inline_asm_patterns(void) {
    int array[16] = {0};
    int idx = 5;
    int value = 0xCAFEBABE;
    
    /* Inline asm with complex addressing constraint */
    asm volatile(
        "# Complex memory store\n\t"
        : "=m"(array[idx * 2 + 3])  /* Complex address calculation */
        : "r"(value)                /* Value in register */
    );
    
    /* Bitfield-like operation via asm */
    struct {
        volatile unsigned int low : 16;
        volatile unsigned int high : 16;
    } bits = {0};
    
    unsigned int mask = 0xFFFF0000;
    asm volatile(
        "# Bitfield manipulation\n\t"
        : "=m"(bits)    /* Whole struct as memory operand */
        : "r"(mask)
    );
    
    /* Prevent optimization */
    asm volatile("" : : "m"(array[0]), "r"(bits.low));
}

/* Main function with checksum to prevent dead code elimination */
int main(void) {
    unsigned int checksum = 0;
    
    /* Execute all tests */
    test_bitfield_operations();
    checksum += 1;
    
    test_subreg_operations();
    checksum += 2;
    
    test_complex_memory_addressing();
    checksum += 3;
    
    test_combined_patterns();
    checksum += 4;
    
    test_inline_asm_patterns();
    checksum += 5;
    
    /* Use checksum to prevent optimization */
    volatile unsigned int final_checksum = checksum;
    
    /* Print to ensure execution */
    printf("Coverage test checksum: %u\n", final_checksum);
    
    return (int)final_checksum;
}
