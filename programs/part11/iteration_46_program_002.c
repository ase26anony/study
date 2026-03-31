/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
static int checksum = 0;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bf = {0};
    
    /* Source variables with different values */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Multiple assignments to trigger different patterns */
    bf.field4 = (a & 0xF);                     /* Simple extraction */
    bf.field8 = (b & 0xFF) + (c & 0xF);        /* Expression with masking */
    bf.field12 = __builtin_popcount(a) & 0xFFF; /* Builtin with masking */
    
    /* Read back and accumulate to checksum */
    checksum += bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: Sub-word type operations for SUBREG */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32;
    
    /* Register sources (hint to compiler) */
    register int32_t r32 asm("r12") = 0x12345678;
    register int64_t r64 asm("r13") = 0x9ABCDEF012345678ULL;
    
    /* Various narrowing assignments that may create SUBREG */
    v8 = (int8_t)r32;                    /* Direct truncation */
    v16 = (int16_t)(r32 + 0x100);        /* Arithmetic then truncation */
    v32 = (int32_t)r64;                  /* 64->32 truncation */
    
    /* More complex narrowing with operations */
    int16_t temp16 = (int16_t)((r32 >> 8) & 0xFFFF);
    v16 = temp16 + 1;                    /* Operation on narrowed value */
    
    /* Accumulate to checksum */
    checksum += v8 + v16 + v32;
}

/* Test 3: Complex memory addressing for MEM_P */
void test_complex_addressing(void) {
    /* Local arrays with different dimensions */
    int array1d[256];
    int array2d[16][16];
    int array3d[8][8][8];
    
    /* Struct with array member */
    struct {
        int data[64];
        int offset;
    } s = { .offset = 7 };
    
    /* Complex index calculations */
    for (int i = 0; i < 8; i++) {
        /* 1D with non-linear index */
        int idx1 = (i * 13 + 7) & 0xFF;
        array1d[idx1] = i * 100;
        
        /* 2D with separate row/col calculation */
        int row = (i * 3) % 16;
        int col = (i * 7) % 16;
        array2d[row][col] = i * 200;
        
        /* 3D with nested calculations */
        int x = i % 8;
        int y = (i * 2) % 8;
        int z = (i * 3) % 8;
        array3d[x][y][z] = i * 300;
        
        /* Struct member with computed offset */
        s.data[(i * s.offset) % 64] = i * 400;
    }
    
    /* Pointer arithmetic with multiple offsets */
    int *ptr = array1d;
    for (int i = 0; i < 16; i++) {
        int offset = (i << 2) + (i & 3);
        *(ptr + offset + s.offset) = i * 500;
    }
    
    /* Accumulate some values to checksum */
    checksum += array1d[13] + array2d[3][7] + array3d[1][2][3] + s.data[14];
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct combined {
        unsigned int flags : 8;
        unsigned int count : 16;
        int16_t values[32];
    } comb = {0};
    
    /* Register source */
    register int32_t src asm("r14") = 0x87654321;
    
    /* Combined assignment: bitfield + array with complex index */
    comb.flags = (src >> 16) & 0xFF;          /* ZERO_EXTRACT candidate */
    
    int index = (src & 0x1F) + 3;             /* Complex index calculation */
    comb.values[index] = (int16_t)src;        /* SUBREG candidate */
    
    /* Additional complex store */
    comb.count = __builtin_parity(src) | ((src >> 8) & 0x7FFF);
    
    /* Accumulate to checksum */
    checksum += comb.flags + comb.count + comb.values[index];
}

/* Test 5: Inline assembly for direct RTL influence */
void test_inline_asm(void) {
    int array[64] = {0};
    int index = 0;
    
    /* Complex addressing in asm output */
    asm volatile (
        "/* Force complex memory operand */"
        : "=m" (array[(index * 7 + 3) & 0x3F])
        : 
        : "memory"
    );
    
    /* Bitfield output (less portable but worth trying) */
    volatile unsigned int bf_target = 0;
    asm volatile (
        "/* Manipulate specific bits */"
        : "=r" (bf_target)
        : "0" (bf_target)
        : 
    );
    
    checksum += array[10] + bf_target;
}

int main(void) {
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    test_subreg_operations();
    test_complex_addressing();
    test_combined_patterns();
    test_inline_asm();
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
