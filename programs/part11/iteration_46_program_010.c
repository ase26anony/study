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
    } bitfields = {0};
    
    /* Variables to use in expressions */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bitfields.field4 = (a & 0xF) + (b & 0xF);          /* 4-bit field with masking */
    bitfields.field8 = (a >> 4) & 0xFF;                /* 8-bit extract from larger value */
    bitfields.field12 = ((a & 0xFFF) ^ (b & 0xFFF)) | (c & 0xFF); /* Complex bitwise ops */
    
    /* Use __builtin_popcount on sub-word data */
    unsigned char byte_val = 0x37;
    bitfields.field4 = __builtin_popcount(byte_val) & 0xF; /* May involve bit extraction */
    
    /* Read back to prevent elimination */
    volatile unsigned int readback = bitfields.field4 + bitfields.field8 + bitfields.field12;
    (void)readback;
}

/* Test 2: SUBREG generation through type narrowing */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs;
    volatile signed char vc;
    volatile unsigned char vuc;
    
    /* Register variables to encourage register operations */
    register int reg_int = 0x12345678;
    register unsigned int reg_uint = 0x9ABCDEF0;
    
    /* Explicit narrowing casts - may generate SUBREG in SET_DEST */
    vs = (short)reg_int;                     /* int -> short */
    vc = (signed char)(reg_int + 100);       /* int -> char with arithmetic */
    vuc = (unsigned char)(reg_uint >> 8);    /* Shift then narrow */
    
    /* Implicit narrowing through arithmetic */
    char c1 = 100, c2 = 50;
    volatile char vc_result = c1 + c2;       /* char + char -> char (may overflow) */
    
    /* Mixed-type operations */
    short s1 = 1000;
    int i1 = 50000;
    vs = s1 + (short)i1;                     /* Mixed with explicit narrowing */
    
    /* Read back */
    volatile int sum = vs + vc + vuc + vc_result;
    (void)sum;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_memory_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int array2d[16][16];
    int * restrict ptr = &array2d[0][0];  /* restrict helps keep address computation */
    
    /* Complex index calculations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Non-linear index: i*stride + j*offset + constant */
            int idx = i * 16 + j * 2 + 3;
            int value = (i << 4) | j;      /* Value in register */
            array2d[i][j] = value;         /* Store with 2D addressing */
            ptr[idx] = value * 2;          /* Store with computed pointer offset */
        }
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[32];
        int footer;
    } mystruct;
    
    /* Access through pointer with offset */
    int *data_ptr = mystruct.data;
    for (int i = 0; i < 16; i++) {
        /* Complex address: base + scaled index + constant */
        data_ptr[i * 2 + 1] = i * i;       /* Store to odd indices only */
    }
    
    /* Pointer chain */
    int ***triple_ptr = NULL;  /* Not used, just for example */
    (void)triple_ptr;
    
    /* Compute checksum to prevent elimination */
    int checksum = 0;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += array2d[i][j];
        }
    }
    checksum += mystruct.header + mystruct.footer;
    for (int i = 0; i < 32; i++) checksum += mystruct.data[i];
    
    volatile int result = checksum;
    (void)result;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        unsigned int status : 4;
        short data[32];
        int counter;
    } combined = {0};
    
    /* Register source for narrowing */
    register int reg_src = 0x89ABCDEF;
    
    /* Combined assignment: bitfield + narrowed store to array */
    combined.flags = (reg_src >> 16) & 0xFF;          /* ZERO_EXTRACT potential */
    
    /* Complex array index with narrowed store */
    for (int i = 0; i < 16; i++) {
        int complex_idx = (i * 7 + 3) & 31;          /* Non-linear index */
        combined.data[complex_idx] = (short)(reg_src + i * 100); /* SUBREG potential */
    }
    
    /* Update counter with memory address computation */
    combined.counter = 0;
    for (int i = 0; i < 32; i++) {
        combined.counter += combined.data[i];        /* MEM_P with array access */
    }
    
    /* Inline assembly to directly influence RTL */
    int dummy_array[16] = {0};
    int idx = 5;
    
    /* Memory output constraint with complex addressing */
    asm volatile (
        "# Force memory operand"
        : "=m" (dummy_array[idx * 2 + 1])  /* Complex address */
        :
        : "memory"
    );
    
    /* Bitfield output via memory constraint */
    asm volatile (
        "# Bitfield reference"
        : "=m" (combined.flags)
        :
        : "memory"
    );
    
    /* Read back everything */
    volatile int total = combined.flags + combined.status + combined.counter;
    for (int i = 0; i < 32; i++) total += combined.data[i];
    (void)total;
}

/* Test 5: Additional patterns for specific architectures */
void test_architecture_specific(void) {
    /* Use parity builtin on sub-word data */
    unsigned short us = 0xBEEF;
    volatile unsigned char parity_result = __builtin_parity(us) & 1;
    
    /* Byte-wise operations with masking */
    volatile uint32_t word = 0;
    uint8_t bytes[4] = {0x11, 0x22, 0x33, 0x44};
    
    /* Build word from bytes - may involve ZERO_EXTRACT during optimization */
    for (int i = 0; i < 4; i++) {
        word |= (bytes[i] << (i * 8));
    }
    
    /* Store byte to word-aligned location with shift */
    volatile uint32_t *word_ptr = &word;
    uint8_t new_byte = 0x55;
    *word_ptr = (*word_ptr & 0xFF00FFFF) | (new_byte << 16);
    
    (void)parity_result;
}

int main(void) {
    int final_checksum = 0;
    
    /* Execute all tests */
    test_bitfield_operations();
    final_checksum += 1;
    
    test_subreg_operations();
    final_checksum += 2;
    
    test_complex_memory_addressing();
    final_checksum += 3;
    
    test_combined_patterns();
    final_checksum += 4;
    
    test_architecture_specific();
    final_checksum += 5;
    
    /* Print result to ensure execution */
    printf("Final checksum: %d\n", final_checksum);
    
    return final_checksum > 0 ? 0 : 1;
}
