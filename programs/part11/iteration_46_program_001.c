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
    bf.field4 = (a & 0xF) + (b & 0xF);          /* 4-bit extract and store */
    bf.field8 = ((a >> 4) & 0xFF) ^ (c & 0xFF); /* 8-bit extract with XOR */
    bf.field12 = (a + b) & 0xFFF;               /* 12-bit masked store */
    
    /* Use bitfield values to prevent elimination */
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: SUBREG operations through type narrowing */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32;
    
    /* Register sources of different sizes */
    register int32_t r32 = 0x12345678;
    register int64_t r64 = 0x9ABCDEF012345678ULL;
    
    /* Explicit narrowing casts that may generate SUBREG */
    v8 = (int8_t)(r32 + 0x100);      /* 32-bit to 8-bit with overflow */
    v16 = (int16_t)(r32 * 2);        /* 32-bit to 16-bit */
    v32 = (int32_t)(r64 >> 16);      /* 64-bit to 32-bit */
    
    /* Arithmetic with implicit narrowing */
    register int16_t r16a = 30000;
    register int16_t r16b = 10000;
    v16 = r16a + r16b;               /* Potential overflow truncation */
    
    /* Use results */
    sink = v8 + v16 + v32;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8] = {0};
    int * restrict ptr = &arr[0][0];  /* restrict helps keep addressing */
    
    /* Complex index calculations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Non-linear addressing: i*stride + j*offset */
            int idx = i * 13 + j * 7;  /* Prime numbers for non-simple pattern */
            idx = idx & 63;            /* Keep within bounds */
            
            /* Register source */
            register int val = i * 100 + j;
            
            /* Store with complex addressing */
            arr[idx][j] = val;
            
            /* Pointer arithmetic with multiple offsets */
            *(ptr + idx * 8 + j) = val * 2;
        }
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[32];
        int footer;
    } s = {0};
    
    /* Access through pointer with offset */
    int *data_ptr = s.data;
    for (int i = 0; i < 16; i++) {
        register int x = i * i;
        data_ptr[i * 2] = x;          /* Stride 2 access */
    }
    
    /* Use results */
    sink = arr[0][0] + s.data[0];
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 16;
        short values[16];
        unsigned int status : 4;
    } combined = {0};
    
    /* Register sources */
    register int r1 = 0x1234;
    register int r2 = 0x5678;
    register int r3 = 0x9ABC;
    
    /* Combined assignment: bitfield + narrowed array store */
    combined.flags = (r1 & 0xFFFF);           /* Potential ZERO_EXTRACT */
    
    /* Complex index calculation */
    for (int i = 0; i < 8; i++) {
        int idx = (i * 3 + 7) & 0xF;          /* Non-linear index */
        register int temp = r2 + i * 100;
        combined.values[idx] = (short)temp;   /* SUBREG + complex addressing */
    }
    
    combined.status = (r3 >> 8) & 0xF;        /* Another bitfield */
    
    /* Use results */
    sink = combined.flags + combined.values[0] + combined.status;
}

/* Test 5: Inline assembly for direct RTL influence */
void test_inline_asm(void) {
    int array[32] = {0};
    int index;
    
    /* Complex addressing in asm output */
    for (int i = 0; i < 8; i++) {
        index = (i * 5 + 3) & 0x1F;  /* Non-linear index */
        
        /* asm with memory output constraint and complex addressing */
        asm volatile (
            "# Force memory store with complex address\n"
            : "=m" (array[index])    /* Memory output */
            :                        /* No inputs */
            : "memory"
        );
    }
    
    /* Bitfield-like operation via asm */
    volatile unsigned int packed = 0;
    unsigned int mask = 0xFF00FF00;
    
    asm volatile (
        "# Operation that might involve bit manipulation\n"
        : "+r" (mask)
        :
        : "cc"
    );
    
    packed = mask & 0x00FF00FF;
    
    sink = array[0] + packed;
}

/* Test 6: Builtin bit operations on sub-word data */
void test_builtin_bitops(void) {
    volatile unsigned char bytes[4] = {0x12, 0x34, 0x56, 0x78};
    register unsigned int x = 0x9ABCDEF0;
    
    /* Builtins that operate on bit ranges */
    unsigned int popcnt_result = __builtin_popcount((unsigned short)x);
    unsigned int parity_result = __builtin_parity((unsigned char)x);
    
    /* Store results with potential ZERO_EXTRACT */
    bytes[0] = (unsigned char)popcnt_result;
    bytes[1] = (unsigned char)parity_result;
    
    /* Shift and mask operations that might generate ZERO_EXTRACT */
    unsigned int masked = x & 0x0000FFFF;
    bytes[2] = (masked >> 8) & 0xFF;
    bytes[3] = masked & 0xFF;
    
    sink = bytes[0] + bytes[1] + bytes[2] + bytes[3];
}

int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Run all tests */
    test_bitfield_operations();
    checksum += sink;
    
    test_subreg_operations();
    checksum += sink;
    
    test_complex_addressing();
    checksum += sink;
    
    test_combined_patterns();
    checksum += sink;
    
    test_inline_asm();
    checksum += sink;
    
    test_builtin_bitops();
    checksum += sink;
    
    /* Final output to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
