/* test_resource_marking.c */
#include <stdio.h>
#include <stdint.h>

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int dummy : 8;     /* Padding */
    } bitfield = {0};
    
    /* Variables for bit manipulation */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bitfield.field4 = (a & 0xF) ^ (b & 0xF);          /* 4-bit extract and store */
    bitfield.field8 = ((a >> 4) & 0xFF) + ((c >> 8) & 0xFF);  /* 8-bit extract */
    bitfield.field12 = (a & 0xFFF) | (b & 0xFFF);     /* 12-bit extract */
    
    /* Use __builtin_parity on sub-word data */
    unsigned char byte_val = (a & 0xFF);
    bitfield.field4 = __builtin_parity(byte_val);     /* May involve bit extraction */
    
    /* Prevent dead code elimination */
    volatile unsigned int read_back = 
        bitfield.field4 | (bitfield.field8 << 4) | (bitfield.field12 << 12);
    (void)read_back;
}

/* Test 2: SUBREG generation through type narrowing */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs;
    volatile char vc;
    volatile int8_t v8;
    volatile int16_t v16;
    
    /* Register variables to encourage register operations */
    register int reg_int = 0x12345678;
    register unsigned int reg_uint = 0x9ABCDEF0;
    
    /* Explicit narrowing casts - may generate SUBREG in SET_DEST */
    vs = (short)reg_int;                     /* int -> short */
    vc = (char)(reg_int >> 16);              /* int -> char */
    v16 = (int16_t)(reg_uint & 0xFFFF);      /* uint -> int16_t */
    v8 = (int8_t)((reg_int * 3) & 0xFF);     /* arithmetic then narrowing */
    
    /* Implicit narrowing through arithmetic */
    unsigned char uc1 = 200;
    unsigned char uc2 = 100;
    vc = uc1 + uc2;                          /* Overflow truncation */
    
    /* Combined operation with narrowing */
    int temp = reg_int * 2 + 0x7F;
    vs = (short)(temp & 0x7FFF);             /* Multiple operations then narrow */
    
    /* Prevent dead code elimination */
    volatile int sum = vs + vc + v8 + v16;
    (void)sum;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_memory_addressing(void) {
    /* Local arrays with restrict to prevent aliasing assumptions */
    int arr1[256] __attribute__((aligned(16)));
    short arr2[512];
    int *restrict ptr1 = arr1;
    short *restrict ptr2 = arr2;
    
    /* Struct with array member */
    struct {
        int header;
        int data[64];
        short shorts[128];
    } mystruct;
    
    /* Initialize with some values */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 3;
    }
    
    /* Complex index calculations for memory stores */
    for (int i = 0; i < 32; i++) {
        /* Non-linear index calculation */
        int idx1 = (i * 13 + 7) & 0xFF;               /* i*13 + 7 */
        int idx2 = (i * 17 - 3) & 0x1FF;              /* i*17 - 3 */
        
        /* Register-held source values */
        register int src1 = i * 0x1234;
        register int src2 = i * 0x5678;
        
        /* Memory stores with complex addressing */
        ptr1[idx1] = src1;                            /* Base + scaled offset */
        ptr2[idx2] = (short)(src2 & 0xFFFF);          /* With narrowing */
        
        /* Multi-dimensional style access */
        arr1[i * 8 + (i & 7)] = src1 ^ src2;          /* i*8 + (i&7) */
        
        /* Struct member access through pointer */
        mystruct.data[(i * 3) % 64] = src1;
        mystruct.shorts[(i * 5) % 128] = (short)src2;
    }
    
    /* Pointer arithmetic with multiple offsets */
    int *p = arr1 + 128;
    for (int i = 0; i < 16; i++) {
        *(p + i * 2 - 1) = i * 0x1111;                /* p + i*2 - 1 */
    }
    
    /* Prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < 256; i += 17) {
        checksum ^= arr1[i];
    }
    for (int i = 0; i < 512; i += 31) {
        checksum ^= arr2[i];
    }
    (void)checksum;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct combined {
        unsigned int flags : 8;
        unsigned int count : 12;
        short values[32];
        int data[16];
    } comb = {0};
    
    /* Variables for computations */
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    register int r3 = 0x13579BDF;
    
    /* Combined assignment 1: Bitfield with complex expression */
    comb.flags = ((r1 & 0xFF) ^ (r2 & 0xFF)) | ((r3 >> 8) & 0x7F);
    
    /* Combined assignment 2: Array with narrowing and complex index */
    int idx = ((r1 & 0xF) * 7 + 3) & 0x1F;            /* Complex index */
    comb.values[idx] = (short)((r2 + r3) & 0xFFFF);   /* Narrowing store */
    
    /* Combined assignment 3: Another array element with different index calc */
    idx = ((r2 & 0x7) * 11 - 5) & 0x1F;               /* Different complex index */
    comb.values[idx] = (short)r1;                     /* Direct narrowing */
    
    /* Combined assignment 4: Bitfield from builtin */
    comb.count = __builtin_popcount(r1 & 0xFFF);      /* Bit count on 12 bits */
    
    /* Combined assignment 5: Data array with pointer arithmetic */
    int *dptr = comb.data + 8;
    for (int i = 0; i < 4; i++) {
        *(dptr + i * 2) = (r1 << i) | (r2 >> (32 - i));  /* Complex address */
    }
    
    /* Prevent dead code elimination */
    volatile unsigned int total = comb.flags + comb.count;
    for (int i = 0; i < 32; i += 5) {
        total += comb.values[i];
    }
    for (int i = 0; i < 16; i += 3) {
        total ^= comb.data[i];
    }
    (void)total;
}

/* Test 5: Inline assembly for direct RTL influence */
void test_inline_asm(void) {
    int array[64] __attribute__((aligned(16)));
    short sarray[128];
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        array[i] = i * 2;
    }
    
    /* Inline asm with complex memory addressing */
    for (int i = 0; i < 8; i++) {
        int idx = (i * 9 + 5) & 0x3F;                 /* Complex index */
        int value = i * 0x1000 + 0x123;
        
        /* asm with memory output and complex addressing */
        asm volatile (
            "# Complex memory store\n"
            : "=m" (array[idx])                       /* Memory output */
            : "r" (value)                             /* Register input */
            : "memory"
        );
    }
    
    /* Multiple offsets in addressing */
    int base_idx = 16;
    for (int i = 0; i < 4; i++) {
        asm volatile (
            "# Memory store with multiple offsets\n"
            : "=m" (sarray[base_idx + i * 7 + 3])     /* base + i*7 + 3 */
            : "r" ((short)(i * 0x1111))
            : "memory"
        );
    }
    
    /* Prevent dead code elimination */
    volatile int asm_sum = 0;
    for (int i = 0; i < 64; i += 9) {
        asm_sum += array[i];
    }
    (void)asm_sum;
}

int main(void) {
    int final_checksum = 0;
    
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    final_checksum += 1;
    
    test_subreg_operations();
    final_checksum += 2;
    
    test_complex_memory_addressing();
    final_checksum += 3;
    
    test_combined_patterns();
    final_checksum += 4;
    
    test_inline_asm();
    final_checksum += 5;
    
    printf("All tests completed. Final marker: %d\n", final_checksum);
    
    return final_checksum != 15;  /* Return 0 if all tests ran */
}
