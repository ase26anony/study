/* test_resource_marking.c - Target coverage for resource.cc lines 282-290 */

#include <stdio.h>
#include <stdint.h>

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
static void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int padding : 8;
    } bit_struct = {0};
    
    /* Variables to force register usage */
    register unsigned int r1 = 0xABCD;
    register unsigned int r2 = 0x1234;
    register unsigned int r3 = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bit_struct.field4 = (r1 & 0xF) + (r2 & 0x3);  /* Should use ZERO_EXTRACT */
    bit_struct.field8 = (r1 >> 4) & 0xFF;         /* Another bitfield store */
    bit_struct.field12 = ((r2 ^ r3) & 0xFFF);     /* Complex expression */
    
    /* Use __builtin_popcount on sub-word data */
    unsigned char byte_data = r1 & 0xFF;
    int popcnt = __builtin_popcount(byte_data);  /* May involve bit extraction */
    (void)popcnt;  /* Prevent unused warning */
    
    /* Read back to prevent elimination */
    volatile unsigned int readback = 
        bit_struct.field4 | (bit_struct.field8 << 4) | (bit_struct.field12 << 12);
    (void)readback;
}

/* Test 2: SUBREG generation through type narrowing */
static void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    
    /* Register sources of different sizes */
    register int ri = 0x12345678;
    register short rs = 0xABCD;
    register char rc = 0x42;
    
    /* Explicit narrowing casts - may generate SUBREG in SET_DEST */
    vs1 = (short)ri;                    /* int -> short */
    vs2 = (short)(ri + rs);             /* Arithmetic then narrowing */
    vc1 = (char)(ri & 0xFF);            /* int -> char with masking */
    vc2 = (char)((rs >> 8) + rc);       /* Complex narrowing */
    
    /* Implicit narrowing through arithmetic */
    char c1 = 100, c2 = 50;
    char sum = c1 + c2;                 /* May overflow and truncate */
    vc1 = sum;
    
    /* Read back */
    volatile int sum_read = vs1 + vs2 + vc1 + vc2;
    (void)sum_read;
}

/* Test 3: Complex memory addressing for MEM_P(x) path */
static void test_complex_memory_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int array[64][8] = {0};
    int * restrict ptr = &array[0][0];  /* restrict helps keep address computation */
    
    /* Register values */
    register int rval1 = 0xDEADBEEF;
    register int rval2 = 0xCAFEBABE;
    
    /* Complex address calculations */
    for (int i = 0; i < 8; i++) {
        /* Non-linear index calculation */
        int idx = i * 7 + 3;                    /* i*stride + offset */
        array[idx % 64][i] = rval1 + i;         /* 2D array access */
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr + idx + (i * 2)) = rval2 - i;     /* Base + idx + (i*2) */
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[16];
        int footer;
    } mystruct = {0};
    
    int *data_ptr = mystruct.data;
    for (int i = 0; i < 8; i++) {
        /* Access through pointer with offset */
        data_ptr[i * 2 + 1] = rval1 ^ (i << 4);  /* ptr->array[index] pattern */
    }
    
    /* Compute checksum to prevent elimination */
    volatile int checksum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            checksum ^= array[i][j];
        }
    }
    checksum ^= mystruct.header + mystruct.footer;
    (void)checksum;
}

/* Test 4: Combined patterns */
static void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct combined {
        unsigned int flags : 8;
        unsigned int count : 12;
        short data[32];
        volatile int checksum;
    } comb = {0};
    
    /* Register variables */
    register int rint = 0x89ABCDEF;
    register short rshort = 0x1234;
    
    /* Combined assignment 1: Bitfield + complex source */
    comb.flags = (rint >> 16) & 0xFF;           /* ZERO_EXTRACT potential */
    
    /* Combined assignment 2: Array with complex index + narrowing */
    int complex_idx = ((rint & 0xF) * 3 + 7) % 32;
    comb.data[complex_idx] = (short)rint;       /* SUBREG potential */
    
    /* Combined assignment 3: Multiple operations */
    comb.count = (rint & 0xFFF) ^ (rshort & 0xFFF);
    comb.data[(complex_idx + 5) % 32] = (short)(rint >> 8);
    
    /* Inline assembly to directly influence RTL */
    int temp_array[16] = {0};
    int idx = (rint & 0x7) * 2 + 1;
    
    /* asm with memory output constraint - complex addressing */
    asm volatile (
        "# Force memory store with complex address"
        : "=m" (temp_array[idx])  /* Complex memory destination */
        : 
        : "memory"
    );
    
    /* Compute final checksum */
    comb.checksum = comb.flags + comb.count;
    for (int i = 0; i < 32; i++) {
        comb.checksum += comb.data[i];
    }
    comb.checksum += temp_array[idx];
}

/* Test 5: Additional patterns for STRICT_LOW_PART */
static void test_strict_low_part_patterns(void) {
    /* Operations that might generate STRICT_LOW_PART */
    volatile unsigned short low_part;
    register unsigned int reg32 = 0x87654321;
    
    /* Assignment that keeps only low part */
    low_part = reg32 & 0xFFFF;
    
    /* Bitfield struct with volatile */
    volatile struct {
        unsigned int low : 16;
        unsigned int high : 16;
    } split = {0};
    
    /* Split 32-bit value - may use STRICT_LOW_PART */
    split.low = reg32 & 0xFFFF;
    split.high = reg32 >> 16;
    
    /* Read back */
    volatile unsigned int reconstructed = split.low | (split.high << 16);
    (void)reconstructed;
}

int main(void) {
    int final_checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    final_checksum += 1;
    
    test_subreg_operations();
    final_checksum += 2;
    
    test_complex_memory_addressing();
    final_checksum += 3;
    
    test_combined_patterns();
    final_checksum += 4;
    
    test_strict_low_part_patterns();
    final_checksum += 5;
    
    /* Print result to ensure execution */
    printf("Final checksum: %d\n", final_checksum);
    
    return final_checksum != 15;  /* Return 0 if all tests ran */
}
