/* test_resource_marking.c */
#include <stdio.h>
#include <stdint.h>

/* Force specific RTL patterns for resource.cc coverage testing */

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int dummy : 8;     /* Padding */
    } bit_struct = {0};
    
    /* Variables for bit manipulation */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bit_struct.field4 = (a & 0xF) ^ (b & 0xF);          /* 4-bit extract and store */
    bit_struct.field8 = ((a >> 4) & 0xFF) + ((c >> 8) & 0xFF); /* 8-bit extract */
    bit_struct.field12 = (a & 0xFFF) | (b & 0xFFF);     /* 12-bit extract */
    
    /* Use __builtin_popcount on sub-word data */
    unsigned char byte_val = (unsigned char)(a & 0xFF);
    int popcnt = __builtin_popcount(byte_val);
    bit_struct.field8 = popcnt & 0xFF;  /* Store result in bitfield */
    
    /* Prevent optimization */
    volatile unsigned int dummy_read = bit_struct.field4 + bit_struct.field8 + bit_struct.field12;
    (void)dummy_read;
}

/* Test 2: SUBREG operations in assignment destinations */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    
    /* Register sources of different sizes */
    register int rint1 = 0x12345678;
    register int rint2 = 0x9ABCDEF0;
    register short rshort = 0x4321;
    register char rchar = 0x65;
    
    /* Explicit narrowing casts that may generate SUBREG in SET_DEST */
    vs1 = (short)rint1;                     /* int -> short with SUBREG */
    vs2 = (short)(rint1 + rint2);           /* Arithmetic then narrowing */
    vc1 = (char)(rshort * 2);               /* short -> char with overflow */
    vc2 = (char)(rchar + 128);              /* char -> char with wrap-around */
    
    /* Implicit narrowing through arithmetic */
    unsigned char uc1 = 200, uc2 = 100;
    unsigned char sum = uc1 + uc2;          /* May generate SUBREG for truncation */
    vc1 = sum;
    
    /* Prevent optimization */
    volatile int dummy_sum = vs1 + vs2 + vc1 + vc2;
    (void)dummy_sum;
}

/* Test 3: Complex memory addressing for MEM_P(x) path */
void test_complex_memory_addressing(void) {
    /* Local arrays with restrict to prevent aliasing assumptions */
    int arr1[256] __attribute__((aligned(16)));
    short arr2[512] __attribute__((aligned(8)));
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 3;
    }
    
    /* Complex addressing computations */
    register int idx;
    register int val = 0xDEADBEEF;
    
    /* Multi-dimensional style addressing */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            idx = i * 16 + j * 2 + 7;           /* Non-linear index computation */
            arr1[idx] = val + (i << 4) + j;     /* Store with complex address */
        }
    }
    
    /* Pointer arithmetic with multiple offsets */
    int *base_ptr = arr1;
    int offset1 = 32;
    int offset2 = 64;
    
    *(base_ptr + offset1 + offset2) = val;      /* Multiple offset addition */
    *(base_ptr + (offset1 << 1) - offset2) = val >> 1;
    
    /* Struct member access through pointer */
    struct {
        int header;
        short data[128];
        int footer;
    } mystruct;
    
    struct s_ptr {
        int header;
        short data[128];
        int footer;
    } *ptr = (struct s_ptr*)&mystruct;
    
    for (int i = 0; i < 32; i++) {
        ptr->data[i * 3] = (short)(val + i);    /* Array access through pointer */
    }
    
    /* Prevent optimization */
    volatile int checksum = arr1[0] + arr1[255] + ptr->data[0];
    (void)checksum;
}

/* Test 4: Combined patterns in single assignments */
void test_combined_patterns(void) {
    /* Struct with mixed members */
    volatile struct combined {
        unsigned int flags : 16;     /* Bitfield */
        short array[32];             /* Sub-word array */
        int counter;                 /* Full word */
    } comb = {0};
    
    /* Variables in registers */
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    register short r3 = 0x4321;
    
    /* Combined assignment 1: Bitfield from complex expression */
    comb.flags = ((r1 & 0xFFFF) ^ (r2 & 0xFFFF)) | 0x8000;
    
    /* Combined assignment 2: Sub-word array with computed index */
    int complex_idx = ((r1 & 0xFF) * 3 + 7) % 32;
    comb.array[complex_idx] = (short)(r1 + r2);  /* Narrowing store to array */
    
    /* Combined assignment 3: Multiple operations */
    comb.array[(r2 & 0x1F)] = (short)r3;
    comb.flags = (comb.flags & 0xFF00) | (r3 & 0xFF);
    
    /* Inline assembly to directly influence RTL generation */
    /* Memory output with complex addressing */
    int mem_array[64];
    asm volatile (
        "# Force complex memory operand\n"
        : "=m" (mem_array[((r1 & 0x3F) * 2 + 16)])  /* Complex index */
        :
        : "memory"
    );
    
    /* Another asm with sub-word memory output */
    short short_array[128];
    asm volatile (
        "# Force SUBREG memory store\n"
        : "=m" (short_array[(r2 & 0x7F)])
        :
        : "memory"
    );
    
    /* Prevent optimization */
    volatile int result = comb.flags + comb.array[0] + comb.counter + 
                         mem_array[0] + short_array[0];
    (void)result;
}

/* Main function executing all tests */
int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Execute all test patterns */
    test_bitfield_operations();
    checksum += 1;
    
    test_subreg_operations();
    checksum += 2;
    
    test_complex_memory_addressing();
    checksum += 3;
    
    test_combined_patterns();
    checksum += 4;
    
    /* Final checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
