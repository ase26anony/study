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
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
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
    bf.field4 = (a & 0xF) + (b & 0x7);      /* 4-bit field with computation */
    bf.field8 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF); /* 8-bit field */
    bf.field12 = (c & 0xFFF) | ((a & 0xF00) >> 4);     /* 12-bit field */
    
    /* Use __builtin_popcount on sub-word data */
    unsigned char byte_val = (a & 0xFF);
    int popcnt = __builtin_popcount(byte_val);
    bf.field4 = popcnt & 0xF;  /* Store popcount result in bitfield */
    
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: SUBREG patterns through type narrowing */
void test_subreg_patterns(void) {
    /* Volatile sub-word destinations */
    volatile short vs;
    volatile signed char vc;
    
    /* Register sources of different sizes */
    register int ri = 0x12345678;
    register long rl = 0x9ABCDEF0;
    
    /* Explicit narrowing casts that may generate SUBREG */
    vs = (short)ri;                     /* int -> short */
    vc = (signed char)(ri >> 16);       /* int -> char */
    
    /* Arithmetic with implicit narrowing */
    unsigned char uc1 = 200;
    unsigned char uc2 = 100;
    vc = uc1 + uc2;  /* Result truncated to char */
    
    /* Complex expression with narrowing */
    vs = (short)((ri * 3) / 7);         /* Computation then narrowing */
    
    /* Multiple narrowing operations */
    int temp = ri ^ rl;
    vc = (signed char)temp;
    vs = (short)(temp >> 8);
    
    sink = vs + vc;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Local arrays with restrict to prevent aliasing assumptions */
    int arr1[256] __attribute__((aligned(16)));
    short arr2[512];
    int *restrict ptr1 = arr1;
    short *restrict ptr2 = arr2;
    
    /* Initialize with some values */
    for (int i = 0; i < 256; i++) {
        ptr1[i] = i * 3;
    }
    
    register int rval = 0x87654321;
    
    /* Complex array indexing */
    for (int i = 0; i < 64; i++) {
        /* Non-linear index computation */
        int idx = (i * 7 + 13) & 0xFF;
        
        /* Store with complex addressing */
        ptr1[idx * 2 + 1] = rval + i;      /* Multi-dimensional style */
        ptr2[idx + 128] = (short)(rval >> (i & 0xF)); /* With narrowing */
    }
    
    /* Pointer arithmetic with multiple offsets */
    int *p = ptr1 + 128;
    for (int i = 0; i < 32; i++) {
        *(p + i*3 - 7) = rval ^ i;  /* Complex pointer arithmetic */
    }
    
    /* Struct-like access through pointer */
    struct {
        int header;
        short data[64];
    } block;
    
    for (int i = 0; i < 16; i++) {
        block.data[i * 4] = (short)(rval + i * 17);
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += ptr1[i];
    }
    sink = sum;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct with mixed members */
    volatile struct {
        unsigned int flags : 16;
        short values[8];
        unsigned char status : 3;
    } combined = {0};
    
    register int src1 = 0x13579BDF;
    register int src2 = 0x2468ACE0;
    
    /* Combined assignment: bitfield + array with complex index */
    combined.flags = (src1 & 0xFFFF) | ((src2 >> 8) & 0xFF);
    
    /* Array store with narrowing and complex index */
    for (int i = 0; i < 8; i++) {
        int idx = (i * 5 + 3) & 0x7;  /* Non-linear index */
        combined.values[idx] = (short)((src1 + i * src2) & 0xFFFF);
    }
    
    /* Bitfield from computation */
    combined.status = __builtin_parity(src1) & 0x7;
    
    /* Inline assembly to directly influence RTL */
    int temp_array[4] = {0};
    int complex_idx = (src1 & 0x3);
    
    /* Inline asm with memory output and complex addressing */
    asm volatile (
        "# Force complex memory store"
        : "=m" (temp_array[complex_idx * 2 + 1])
        : 
        : "memory"
    );
    
    sink = combined.flags + combined.values[0] + combined.status + temp_array[1];
}

/* Test 5: Additional patterns for coverage */
void test_additional_patterns(void) {
    /* Test STRICT_LOW_PART patterns through byte operations */
    volatile uint32_t word;
    volatile uint8_t *byte_ptr = (volatile uint8_t*)&word;
    
    register uint32_t r = 0x89ABCDEF;
    
    /* Byte stores that might generate STRICT_LOW_PART */
    byte_ptr[0] = r & 0xFF;
    byte_ptr[1] = (r >> 8) & 0xFF;
    byte_ptr[2] = (r >> 16) & 0xFF;
    byte_ptr[3] = (r >> 24) & 0xFF;
    
    /* Bitfield extract and store */
    struct {
        volatile uint32_t low16 : 16;
        volatile uint32_t high16 : 16;
    } split;
    
    split.low16 = r & 0xFFFF;
    split.high16 = r >> 16;
    
    /* Mixed-size operations */
    uint16_t halfwords[4];
    for (int i = 0; i < 4; i++) {
        halfwords[i] = (uint16_t)(r >> (i * 4));
    }
    
    sink = word + split.low16 + split.high16 + halfwords[0];
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    total_checksum += sink;
    
    test_subreg_patterns();
    total_checksum += sink;
    
    test_complex_addressing();
    total_checksum += sink;
    
    test_combined_patterns();
    total_checksum += sink;
    
    test_additional_patterns();
    total_checksum += sink;
    
    /* Prevent optimization of entire program */
    asm volatile ("" : : "r"(total_checksum));
    
    printf("Total checksum: %d\n", total_checksum);
    printf("All tests completed.\n");
    
    return total_checksum != 0 ? 0 : 1;
}
