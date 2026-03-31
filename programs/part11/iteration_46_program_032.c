/* test_resource_marking.c - Generate RTL patterns for uncovered lines in resource.cc */

#include <stdio.h>
#include <stdint.h>

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int padding : 8;   /* Padding */
    } bitfields = {0};
    
    /* Variables to use in expressions */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with complex expressions */
    bitfields.field4 = (a & 0xF) + (b & 0xF);           /* Should generate ZERO_EXTRACT */
    bitfields.field8 = (a >> 4) & 0xFF;                 /* Another bitfield store */
    bitfields.field12 = ((b + c) & 0xFFF) | (a & 0x7);  /* Complex expression */
    
    /* Use __builtin_popcount on sub-word data */
    unsigned char byte_val = 0x55;
    unsigned int popcnt = __builtin_popcount(byte_val);  /* May involve bit extraction */
    bitfields.field4 = popcnt & 0xF;                     /* Store result back to bitfield */
    
    /* Prevent optimization */
    volatile unsigned int dummy = bitfields.field4 + bitfields.field8 + bitfields.field12;
    (void)dummy;
}

/* Test 2: SUBREG generation through type narrowing */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    
    /* Register variables to encourage register operations */
    register int reg_int1 = 0x12345678;
    register int reg_int2 = 0x9ABCDEF0;
    register char reg_char1 = 0x42;
    register char reg_char2 = 0x84;
    
    /* Explicit narrowing casts - should generate SUBREG in SET_DEST */
    vs1 = (short)reg_int1;                     /* int -> short narrowing */
    vs2 = (short)(reg_int1 + reg_int2);        /* Arithmetic then narrowing */
    
    /* Char operations with implicit truncation */
    vc1 = reg_char1 + reg_char2;               /* char + char -> char (truncation) */
    vc2 = (char)(reg_int1 & 0xFF);             /* Mask and narrow */
    
    /* More complex narrowing with arithmetic */
    for (int i = 0; i < 4; i++) {
        vs1 = (short)(vs1 + (short)reg_int1);  /* Repeated narrowing stores */
        vc1 = (char)(vc1 * 2);                 /* Char arithmetic with truncation */
    }
    
    /* Prevent optimization */
    volatile int dummy = vs1 + vs2 + vc1 + vc2;
    (void)dummy;
}

/* Test 3: Complex memory addressing for MEM_P(x) path */
void test_complex_memory_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr2d[16][16];
    int arr1d[256];
    
    /* Struct with array member */
    struct {
        int header;
        int data[32];
        int footer;
    } mystruct;
    
    /* Variables for index calculations */
    int i, j, k;
    int sum = 0;
    
    /* Complex array indexing */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            /* Non-linear index calculation */
            int idx = i * 13 + j * 7 + 3;      /* Complex addressing */
            arr2d[i][j] = idx;                 /* Store with computed index */
            
            /* Pointer arithmetic with multiple offsets */
            int *ptr = arr1d + i * 16 + j;
            *ptr = idx * 2;                    /* Store through pointer arithmetic */
        }
    }
    
    /* Struct member access through pointer */
    struct {
        int header;
        int data[32];
        int footer;
    } *struct_ptr = &mystruct;
    
    for (k = 0; k < 16; k++) {
        /* Complex index for struct array */
        int complex_idx = (k * 3 + 7) & 0x1F;
        struct_ptr->data[complex_idx] = k * 100;  /* Store to struct member array */
    }
    
    /* Restrict pointer for alias analysis */
    int *restrict rptr = arr1d;
    for (i = 0; i < 64; i++) {
        rptr[i * 2 + 1] = i * i;               /* Stride access with restrict */
    }
    
    /* Compute checksum to prevent elimination */
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            sum += arr2d[i][j];
        }
    }
    
    volatile int dummy = sum;
    (void)dummy;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfields and arrays */
    volatile struct {
        unsigned int flags : 8;
        unsigned int status : 4;
        short data[16];
        volatile int counter;
    } combined = {0};
    
    /* Register variables */
    register int reg_val = 0x89ABCDEF;
    register short reg_short = 0x1234;
    
    /* Combined assignment: bitfield + narrowed store */
    combined.flags = (reg_val >> 16) & 0xFF;    /* Bitfield extract */
    
    /* Complex index calculation */
    for (int i = 0; i < 8; i++) {
        int idx = (i * 5 + 3) & 0xF;           /* Non-linear index */
        
        /* Narrow int to short with complex addressing */
        combined.data[idx] = (short)(reg_val + i * 1000);  /* SUBREG in dest */
        
        /* Update bitfield based on array value */
        combined.status = (combined.data[idx] >> 4) & 0xF; /* Another bitfield */
    }
    
    /* Inline assembly to influence RTL generation */
    int temp_array[8] = {0};
    int idx = 3;
    
    /* Inline asm with memory output and complex addressing */
    asm volatile (
        "# Force complex memory operand"
        : "=m" (temp_array[idx * 2 + 1])  /* Complex address calculation */
        :
        : "memory"
    );
    
    /* Another asm with register narrowing hint */
    asm volatile (
        "# Hint at register narrowing"
        : "=r" (reg_short)
        : "0" (reg_short)
    );
    
    /* Store the narrowed value */
    combined.data[0] = reg_short;  /* Should involve SUBREG */
    
    /* Prevent optimization */
    volatile int dummy = combined.flags + combined.status + combined.data[0] + combined.counter;
    (void)dummy;
}

/* Test 5: Additional patterns for ZERO_EXTRACT */
void test_zero_extract_patterns(void) {
    /* Various bitfield configurations */
    struct {
        volatile unsigned int a : 3;
        volatile unsigned int b : 5;
        volatile unsigned int c : 10;
        volatile unsigned int d : 14;
    } bits = {0};
    
    /* Operations that might generate ZERO_EXTRACT */
    unsigned int x = 0x13579BDF;
    
    /* Multiple bitfield assignments */
    bits.a = (x >> 0) & 0x7;      /* Extract 3 bits */
    bits.b = (x >> 3) & 0x1F;     /* Extract 5 bits */
    bits.c = (x >> 8) & 0x3FF;    /* Extract 10 bits */
    bits.d = (x >> 18) & 0x3FFF;  /* Extract 14 bits */
    
    /* Bitfield assignment from builtin */
    bits.a = __builtin_parity(x) & 1;  /* Parity of full int to single bit */
    
    /* Chain of bitfield operations */
    bits.b = bits.a + bits.c;          /* Mix bitfields */
    
    /* Prevent optimization */
    volatile unsigned int dummy = bits.a + bits.b + bits.c + bits.d;
    (void)dummy;
}

int main(void) {
    int checksum = 0;
    
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
    
    test_zero_extract_patterns();
    checksum += 5;
    
    /* Final checksum to ensure all code executed */
    printf("Final checksum: %d\n", checksum);
    
    return checksum == 15 ? 0 : 1;
}
