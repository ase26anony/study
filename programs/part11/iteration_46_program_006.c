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
        unsigned int dummy : 8;     /* Padding */
    } bf = {0};
    
    /* Variables to create complex expressions */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with expressions */
    bf.field4 = (a & 0xF) + (b & 0x7);      /* Should generate ZERO_EXTRACT */
    bf.field8 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF);
    bf.field12 = (a + b + c) & 0xFFF;
    
    /* Use __builtin_parity on sub-word data */
    unsigned char byte_val = (a ^ b) & 0xFF;
    int parity = __builtin_parity(byte_val);
    bf.field4 = parity & 0xF;
    
    /* Prevent optimization */
    volatile unsigned int dummy_sum = bf.field4 + bf.field8 + bf.field12;
    (void)dummy_sum;
}

/* Test 2: SUBREG generation through type narrowing */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2, vs3;
    volatile char vc1, vc2;
    
    /* Register sources of different sizes */
    register int ri1 = 0x12345678;
    register int ri2 = 0x9ABCDEF0;
    register short rs1 = 0x1234;
    register char rc1 = 0x56;
    
    /* Explicit narrowing casts - should generate SUBREG in SET_DEST */
    vs1 = (short)ri1;                     /* int -> short */
    vs2 = (short)(ri1 + ri2);             /* expression with narrowing */
    vc1 = (char)rs1;                      /* short -> char */
    vc2 = (char)(rc1 + 10);               /* char arithmetic with truncation */
    
    /* Implicit narrowing through arithmetic */
    char c1 = 100, c2 = 200;
    char c3 = c1 + c2;                    /* Overflow truncation */
    vs3 = rs1 + c3;                       /* mixed types */
    
    /* Complex expression with narrowing */
    int temp = ri1 * 3 + ri2 / 5;
    vs1 = (short)(temp & 0xFFFF);
    
    /* Prevent optimization */
    volatile int dummy_sum = vs1 + vs2 + vs3 + vc1 + vc2 + c3;
    (void)dummy_sum;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_memory_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr2d[16][16];
    int arr1d[256];
    
    /* Struct with array member */
    struct {
        int data[64];
        int offset;
    } s = {0};
    s.offset = 8;
    
    /* Pointer for arithmetic */
    int *restrict ptr = arr1d;
    
    /* Complex address calculations */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            /* Multi-dimensional with non-linear index */
            int idx = i * 16 + j;
            int idx2 = (i * 3 + j * 7) & 0xF;
            
            /* Store with complex addressing */
            arr2d[i][j] = idx;                     /* Base + index */
            arr1d[idx * 2] = idx2;                 /* Scaled index */
            
            /* Pointer arithmetic with multiple offsets */
            *(ptr + idx + s.offset) = idx + idx2;
            
            /* Struct member with computed index */
            s.data[(i * 4 + j) & 0x3F] = idx;
        }
    }
    
    /* Additional complex addressing patterns */
    int *base = arr1d;
    int offset1 = 32;
    int offset2 = 16;
    
    /* Multiple offsets in one expression */
    base[offset1 + offset2] = 0xDEADBEEF;
    *(base + offset1 * 2 - offset2) = 0xCAFEBABE;
    
    /* Prevent optimization - compute checksum */
    volatile int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += arr1d[i];
    }
    (void)sum;
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
    
    /* Variables for complex expressions */
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    register short rs = 0x1234;
    
    /* Combined assignment 1: Bitfield + complex addressing */
    int idx = (r1 & 0x1F);  /* 0-31 index */
    combined.data[idx * 2] = (short)(r1 + r2);  /* SUBREG + complex address */
    combined.flags = (r1 ^ r2) & 0xFF;          /* ZERO_EXTRACT */
    
    /* Combined assignment 2: Multiple patterns in sequence */
    combined.status = __builtin_popcount(r1) & 0xF;
    
    /* Pointer to struct member with offset */
    short *data_ptr = combined.data;
    data_ptr[idx + 4] = (short)r2;  /* SUBREG + pointer arithmetic */
    
    /* Complex index calculation */
    int complex_idx = ((r1 & 0xFF) * 3 + (r2 & 0xFF) * 7) & 0x1F;
    combined.data[complex_idx] = (short)(r1 * r2);
    
    /* Prevent optimization */
    volatile int sum = combined.flags + combined.status + combined.data[0] + 
                      combined.data[31] + combined.counter;
    (void)sum;
}

/* Test 5: Inline assembly for direct RTL influence */
void test_inline_assembly(void) {
    int array[64] = {0};
    volatile short vs = 0;
    volatile struct {
        unsigned int field : 8;
    } asm_bf = {0};
    
    /* Complex memory addressing via asm */
    int idx = 42;
    asm volatile (
        "# Force complex memory store\n"
        : "=m" (array[idx * 2 + 8])  /* Complex addressing */
        :
        : "memory"
    );
    
    /* Memory store with constraint */
    int value = 0x1234;
    asm volatile (
        ""
        : "=m" (vs)      /* Should create MEM in SET_DEST */
        : "r" (value)    /* Register input */
        : "memory"
    );
    
    /* Prevent optimization */
    volatile int dummy = array[0] + vs + asm_bf.field;
    (void)dummy;
}

/* Main function executing all tests */
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
    
    test_inline_assembly();
    checksum += 5;
    
    /* Final checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum != 15 ? 1 : 0;
}
