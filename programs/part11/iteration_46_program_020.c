/* test_resource_marking.c
 * Designed to generate RTL patterns that trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O2 -fdump-rtl-all -c test_resource_marking.c
 */

#include <stdio.h>
#include <stdint.h>

/* Test 1: Generate ZERO_EXTRACT and STRICT_LOW_PART patterns via bitfields */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct to prevent optimization */
    volatile struct {
        unsigned int field4 : 4;    /* Likely generates ZERO_EXTRACT */
        unsigned int field8 : 8;    /* For sub-byte extraction */
        unsigned int field16 : 16;  /* For word extraction */
    } bf = {0};
    
    /* Source variables in registers */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Complex assignments to bitfields - may generate ZERO_EXTRACT in SET_DEST */
    bf.field4 = (a & 0xF) + (b & 0xF);      /* Extraction and arithmetic */
    bf.field8 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF); /* XOR of extracted bits */
    bf.field16 = (c & 0xFFFF) | ((a & 0xFF) << 8); /* Composition */
    
    /* Use __builtin_popcount on sub-word data */
    unsigned char byte_val = (unsigned char)(a & 0xFF);
    bf.field8 = __builtin_popcount(byte_val); /* May involve bit extraction */
    
    /* Prevent dead code elimination */
    volatile unsigned int dummy = bf.field4 + bf.field8 + bf.field16;
    (void)dummy;
}

/* Test 2: Generate SUBREG patterns via type narrowing */
void test_subreg_operations(void) {
    /* Volatile destination to force store */
    volatile short vs;
    volatile char vc;
    
    /* Register sources to encourage register operations */
    register int reg_int = 0x12345678;
    register long reg_long = 0x9ABCDEF0;
    
    /* Explicit narrowing casts - may generate SUBREG in SET_DEST */
    vs = (short)reg_int;                     /* int -> short */
    vc = (char)(reg_int >> 16);              /* extracted byte */
    
    /* Arithmetic with implicit narrowing */
    unsigned char uc1 = 200;
    unsigned char uc2 = 100;
    vc = uc1 + uc2;                          /* May overflow, generates SUBREG */
    
    /* Complex expression with narrowing */
    vs = (short)((reg_int & 0xFFFF) + (reg_long & 0xFFFF));
    
    /* Prevent dead code elimination */
    volatile int dummy = vs + vc;
    (void)dummy;
}

/* Test 3: Generate MEM_P with complex addressing */
void test_complex_memory_addressing(void) {
    /* Local arrays with restrict to prevent aliasing assumptions */
    int arr1[256] __attribute__((aligned(16)));
    short arr2[512] __attribute__((aligned(8)));
    
    /* Initialize */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 3;
    }
    
    /* Complex index calculations */
    register int idx;
    register int val = 0x87654321;
    
    /* Multi-dimensional style access */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            idx = i * 16 + j * 2 + 7;        /* Non-linear addressing */
            arr1[idx] = val + (i << j);      /* Complex store destination */
        }
    }
    
    /* Pointer arithmetic with multiple offsets */
    int *base = arr1;
    int offset1 = 32;
    int offset2 = 64;
    
    *(base + offset1 + offset2) = val;       /* Compound pointer arithmetic */
    
    /* Struct pointer access */
    struct {
        int header;
        short data[128];
        int footer;
    } s;
    
    int *ptr = &s.header;
    ptr[16] = val;                           /* Array access through pointer */
    
    /* Prevent dead code elimination */
    volatile int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += arr1[i];
    }
    (void)sum;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct with mixed types */
    volatile struct {
        unsigned int flags : 12;
        short values[8];
        unsigned char status : 3;
    } combined = {0};
    
    /* Register variables */
    register int r1 = 0x13579BDF;
    register int r2 = 0x2468ACE0;
    
    /* Combined assignment: bitfield + array with complex index */
    combined.flags = (r1 & 0xFFF) | ((r2 & 0xF) << 8);
    
    /* Array store with narrowing cast and complex index */
    int idx = ((r1 & 0x7) * 3 + 5) & 0x7;    /* Complex index calculation */
    combined.values[idx] = (short)(r1 + r2); /* Narrowing store */
    
    /* Another bitfield assignment */
    combined.status = __builtin_parity(r1) | (__builtin_popcount(r2) & 1);
    
    /* Prevent dead code elimination */
    volatile int check = combined.flags + combined.values[0] + combined.status;
    (void)check;
}

/* Test 5: Inline assembly for direct RTL influence */
void test_inline_asm(void) {
    int array[64] __attribute__((aligned(16)));
    int index = 42;
    int value = 0xDEADBEEF;
    
    /* Complex addressing in asm output */
    asm volatile (
        "# Force complex memory store\n"
        : "=m" (array[index * 2 + 8])  /* Complex addressing mode */
        : "r" (value)
        : "memory"
    );
    
    /* Bitfield-like output (less portable but worth trying) */
    unsigned int packed;
    asm volatile (
        "# Pack bits\n"
        : "=r" (packed)
        : 
        : 
    );
    
    (void)packed;
}

/* Main function executing all tests */
int main(void) {
    unsigned int checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    checksum += 1;
    
    test_subreg_operations();
    checksum += 2;
    
    test_complex_memory_addressing();
    checksum += 3;
    
    test_combined_patterns();
    checksum += 4;
    
    test_inline_asm();
    checksum += 5;
    
    printf("All tests completed. Checksum: %u\n", checksum);
    
    return (int)checksum;
}
