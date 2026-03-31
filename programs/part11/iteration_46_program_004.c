/* test_resource_marking.c - Generate RTL patterns for ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations alive */
static volatile int sink;

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int dummy : 8;     /* Padding */
    } bf = {0};
    
    /* Source variables with different values */
    register uint32_t a = 0xABCD1234;
    register uint32_t b = 0x56789ABC;
    register uint32_t c = 0xF0F0F0F0;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.field4 = (a & 0xF) + (b & 0x7);          /* 4-bit field with computation */
    bf.field8 = ((a >> 8) & 0xFF) ^ ((b >> 4) & 0xFF); /* 8-bit field with XOR */
    bf.field12 = ((a + b) & 0xFFF) | ((c >> 4) & 0xFFF); /* 12-bit field with OR */
    
    /* Read back to prevent elimination */
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: Sub-word type operations to generate SUBREG */
void test_subword_operations(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32;
    
    /* Register sources of different sizes */
    register int32_t r32 = 0x12345678;
    register int64_t r64 = 0x9ABCDEF012345678ULL;
    
    /* Narrowing assignments that may generate SUBREG in SET_DEST */
    v8 = (int8_t)(r32 + 0x100);      /* 32-bit to 8-bit with overflow */
    v16 = (int16_t)(r64 >> 16);      /* 64-bit to 16-bit with shift */
    v32 = (int32_t)(r64 & 0xFFFFFFFF); /* 64-bit to 32-bit with mask */
    
    /* Arithmetic with implicit narrowing */
    register int16_t r16a = 30000;
    register int16_t r16b = 10000;
    v16 = r16a + r16b;  /* May overflow and generate SUBREG */
    
    /* Read back */
    sink = v8 + v16 + v32;
}

/* Test 3: Complex memory addressing to generate MEM_P with non-trivial address */
void test_complex_memory_addressing(void) {
    /* Local arrays with different dimensions */
    int32_t arr1d[256];
    int32_t arr2d[16][16];
    int32_t arr3d[8][8][8];
    
    /* Struct with array member */
    struct {
        int32_t header;
        int32_t data[64];
        int32_t footer;
    } mystruct;
    
    /* Register source values */
    register int32_t rval1 = 0x11111111;
    register int32_t rval2 = 0x22222222;
    register int32_t rval3 = 0x33333333;
    register int32_t rval4 = 0x44444444;
    
    /* Complex 1D array access with non-linear index */
    for (int i = 0; i < 16; i++) {
        /* Non-linear index calculation */
        int idx = (i * 13 + 7) & 0xFF;  /* Prime multiplier for scattering */
        arr1d[idx] = rval1 + i;
    }
    
    /* 2D array with row-major calculation */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Multi-dimensional index calculation */
            arr2d[i][j] = rval2 + (i << 4) + j;
        }
    }
    
    /* 3D array with complex addressing */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                arr3d[i][j][k] = rval3 + (i * 64 + j * 8 + k);
            }
        }
    }
    
    /* Struct member access through pointer with offset */
    int32_t *ptr = &mystruct.data[0];
    for (int i = 0; i < 16; i++) {
        /* Pointer arithmetic with multiple offsets */
        *(ptr + (i * 3) + 1) = rval4 + i;  /* Skip pattern */
    }
    
    /* Read back samples to prevent elimination */
    sink = arr1d[13] + arr2d[3][3] + arr3d[1][1][1] + mystruct.data[4];
}

/* Test 4: Combined patterns in single assignments */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int config : 6;
        unsigned int status : 10;
        int16_t samples[32];
        int32_t accumulator;
    } device;
    
    /* Source registers */
    register uint32_t config_val = 0x3F;  /* Max 6-bit value */
    register int32_t temp32 = 0x87654321;
    register int16_t temp16;
    
    /* Combined: bitfield assignment (ZERO_EXTRACT) */
    device.config = config_val & 0x3F;
    
    /* Combined: sub-word array store with complex index */
    for (int i = 0; i < 8; i++) {
        /* Complex index calculation */
        int idx = (i * 5 + 3) & 0x1F;
        
        /* Narrow 32-bit to 16-bit (SUBREG) + array store (MEM_P) */
        device.samples[idx] = (int16_t)(temp32 >> (i * 2));
    }
    
    /* Another bitfield with computation */
    device.status = (config_val >> 6) & 0x3FF;
    
    /* Read back */
    sink = device.config + device.status + device.samples[3] + device.accumulator;
}

/* Test 5: Inline assembly for direct RTL influence */
void test_inline_asm(void) {
    int32_t array[64];
    int32_t value = 0xDEADBEEF;
    
    /* Complex addressing in asm output constraint */
    for (int i = 0; i < 8; i++) {
        int idx = (i * 9 + 5) & 0x3F;  /* Non-linear index */
        
        /* Inline asm with memory output and complex addressing */
        asm volatile (
            "# Force memory store with complex address\n"
            : "=m" (array[idx])  /* Memory output with computed index */
            : "r" (value)        /* Value in register */
            : "memory"
        );
        
        /* Modify value for next iteration */
        value = value * 13 + 1;
    }
    
    /* Bitfield manipulation via asm */
    volatile struct {
        unsigned int bits : 8;
    } bf = {0};
    
    uint32_t src = 0xAA;
    
    /* Indirect bitfield store */
    asm volatile (
        "# Hint at bitfield operation\n"
        : "=m" (bf)      /* Whole struct as memory output */
        : "r" (src)      /* Source in register */
        : "memory"
    );
    
    sink = array[5] + bf.bits;
}

/* Test 6: Bit manipulation builtins on sub-word data */
void test_bit_builtins(void) {
    volatile uint16_t result1, result2;
    register uint32_t data = 0x13579BDF;
    
    /* Builtins that may operate on extracted bits */
    result1 = __builtin_popcount((uint8_t)(data & 0xFF));  /* Extract byte first */
    result2 = __builtin_parity((uint16_t)(data >> 8));     /* Extract halfword */
    
    /* Bitfield extract simulation */
    volatile struct {
        unsigned int extracted : 12;
    } extractor;
    
    /* Extract middle 12 bits */
    extractor.extracted = (data >> 10) & 0xFFF;
    
    sink = result1 + result2 + extractor.extracted;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Run all tests */
    test_bitfield_operations();
    checksum += sink;
    
    test_subword_operations();
    checksum += sink;
    
    test_complex_memory_addressing();
    checksum += sink;
    
    test_combined_patterns();
    checksum += sink;
    
    test_inline_asm();
    checksum += sink;
    
    test_bit_builtins();
    checksum += sink;
    
    /* Final output to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
