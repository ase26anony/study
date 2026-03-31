/* test_resource_marking.c */
#include <stdio.h>
#include <stdint.h>

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int f1 : 4;
        unsigned int f2 : 8;
        unsigned int f3 : 12;
        unsigned int f4 : 8;
    } bf = {0};
    
    /* Variables for bit manipulation */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.f1 = (a & 0xF) + (b & 0x7);          /* 4-bit field */
    bf.f2 = (a >> 4) & 0xFF;                /* 8-bit field */
    bf.f3 = ((b << 4) | (c & 0xF)) & 0xFFF; /* 12-bit field */
    bf.f4 = __builtin_popcount(a) & 0xFF;   /* Builtin on sub-word result */
    
    /* Read back to prevent elimination */
    volatile unsigned int readback = bf.f1 + bf.f2 + bf.f3 + bf.f4;
    (void)readback;
}

/* Test 2: SUBREG generation through type narrowing */
void test_subreg_ops(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32;
    
    /* Register sources to encourage register operations */
    register int32_t r32_1 = 0x12345678;
    register int32_t r32_2 = 0x9ABCDEF0;
    register int64_t r64 = 0x1122334455667788ULL;
    
    /* Explicit narrowing casts that may generate SUBREG in SET_DEST */
    v8 = (int8_t)(r32_1 + r32_2);           /* 32-bit to 8-bit */
    v16 = (int16_t)(r32_1 * 2);             /* 32-bit to 16-bit */
    v32 = (int32_t)r64;                     /* 64-bit to 32-bit */
    
    /* Implicit narrowing through arithmetic */
    int8_t c1 = 100, c2 = 50;
    volatile int8_t result = c1 + c2;       /* May overflow, needs truncation */
    
    /* Read back */
    (void)(v8 + v16 + v32 + result);
}

/* Test 3: Complex memory addressing for MEM_P(x) path */
void test_complex_mem_ops(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][16];
    int *restrict ptr1 = &arr[0][0];
    int *restrict ptr2 = &arr[32][8];
    
    /* Complex address calculations */
    for (int i = 0; i < 8; i++) {
        /* Non-linear index calculation */
        int idx = i * 7 + 3;
        
        /* Store with complex addressing - may generate MEM with complex address */
        arr[idx % 64][(idx / 2) % 16] = i * 100;
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr1 + i * 3 + 5) = i * 200;
        
        /* Struct-like access through pointer */
        ptr2[i * 2] = i * 300;
    }
    
    /* Compute checksum to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 16; j++) {
            sum += arr[i][j];
        }
    }
    (void)sum;
}

/* Test 4: Combined patterns */
void test_combined_ops(void) {
    /* Struct with mixed members */
    struct combined {
        volatile unsigned int flags : 16;
        volatile short data[32];
        volatile int counter;
    } cmb;
    
    /* Initialize */
    for (int i = 0; i < 32; i++) {
        cmb.data[i] = 0;
    }
    cmb.counter = 0;
    
    /* Combined assignment: bitfield + sub-word array with complex index */
    register int temp = 0x87654321;
    
    /* Bitfield assignment (potential ZERO_EXTRACT) */
    cmb.flags = (temp & 0xFFFF) ^ 0x1234;
    
    /* Sub-word store with narrowing (potential SUBREG) */
    int idx = (temp & 0x1F);  /* 0-31 */
    cmb.data[idx] = (short)(temp >> 16);
    
    /* Complex memory store */
    cmb.data[(idx * 3 + 7) % 32] = (short)cmb.counter;
    
    /* Inline assembly to directly influence RTL generation */
    int array[64];
    int complex_idx = (temp & 0x3F) * 2 + 1;
    
    /* Memory output constraint with complex addressing */
    asm volatile (
        "# Force memory operand with complex address\n"
        : "=m" (array[complex_idx])
        : 
        : "memory"
    );
    
    /* Read back everything */
    volatile int checksum = cmb.flags + cmb.data[0] + cmb.data[idx] + array[complex_idx];
    (void)checksum;
}

/* Test 5: Additional patterns for STRICT_LOW_PART */
void test_strict_low_part(void) {
    /* Operations that might generate STRICT_LOW_PART for partial word updates */
    volatile uint32_t word = 0;
    
    /* Multiple byte operations on the same word */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)&word;
    
    byte_ptr[0] = 0xAA;  /* May generate byte store to word */
    byte_ptr[1] = 0xBB;
    byte_ptr[2] = 0xCC;
    byte_ptr[3] = 0xDD;
    
    /* Bitfield operations on overlapping fields */
    struct overlapping {
        volatile uint32_t low : 10;
        volatile uint32_t mid : 12;
        volatile uint32_t high : 10;
    } ov;
    
    ov.low = 0x1FF;   /* 9 bits set */
    ov.mid = 0xAAA;   /* Might overlap in RTL representation */
    
    /* Read back */
    (void)(word + ov.low + ov.mid);
}

int main(void) {
    int total_checksum = 0;
    
    /* Execute all tests */
    test_bitfield_ops();
    total_checksum += 1;
    
    test_subreg_ops();
    total_checksum += 2;
    
    test_complex_mem_ops();
    total_checksum += 3;
    
    test_combined_ops();
    total_checksum += 4;
    
    test_strict_low_part();
    total_checksum += 5;
    
    /* Print result to prevent optimization */
    printf("Test checksum: %d\n", total_checksum);
    
    return total_checksum > 0 ? 0 : 1;
}
