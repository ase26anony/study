/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource marking:
 * 1. ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * 2. SUBREG in SET_DEST  
 * 3. MEM_P with complex addressing
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
    bf.field4 = (a & 0xF) + (b & 0xF);          /* Extract and add low nibbles */
    bf.field8 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF); /* XOR of middle bytes */
    bf.field12 = __builtin_popcount(a) + __builtin_popcount(b); /* Bit count sum */
    
    /* Store to prevent elimination */
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: Sub-word type operations for SUBREG */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32;
    
    /* Register sources of different sizes */
    register int32_t r32 = 0x12345678;
    register int64_t r64 = 0x9ABCDEF012345678ULL;
    
    /* Narrowing assignments that may generate SUBREG */
    v8 = (int8_t)(r32 + 0x100);      /* 32-bit to 8-bit with overflow */
    v16 = (int16_t)((r32 >> 8) + 0x2000); /* Shifted 32-bit to 16-bit */
    
    /* Arithmetic with implicit narrowing */
    register int16_t r16a = 0x7FFF;
    register int16_t r16b = 0x0001;
    v16 = r16a + r16b;  /* Potential overflow, stored to volatile short */
    
    /* Store results */
    sink = v8 + v16 + v32;
}

/* Test 3: Complex memory addressing for MEM_P recursion */
void test_complex_addressing(void) {
    /* Local arrays with restrict to prevent aliasing assumptions */
    int16_t arr16[256] __attribute__((aligned(16)));
    int32_t arr32[128] __attribute__((aligned(16)));
    
    /* Initialize */
    for (int i = 0; i < 256; i++) arr16[i] = i;
    for (int i = 0; i < 128; i++) arr32[i] = i * 2;
    
    /* Complex index calculations */
    register int idx;
    register int32_t src32 = 0x87654321;
    register int16_t src16 = 0x1234;
    
    /* Multi-dimensional style addressing */
    idx = 7 * 13 + 11;  /* Non-linear calculation */
    arr16[idx] = src16;  /* Store with computed index */
    
    /* Pointer arithmetic with multiple offsets */
    int32_t *ptr32 = arr32 + 32;
    ptr32[4 * 7 + 3] = src32;  /* Base + scaled offset */
    
    /* Struct-like access pattern */
    struct {
        int16_t data[64];
        int32_t extra[32];
    } s __attribute__((aligned(16)));
    
    s.data[8 * 3 + 2] = src16;
    s.extra[4 * 5 + 1] = src32;
    
    /* Compute checksum to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 256; i += 16) sum += arr16[i];
    for (int i = 0; i < 128; i += 8) sum += arr32[i];
    sink = sum + s.data[0] + s.extra[0];
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct with mixed members */
    volatile struct {
        unsigned int flags : 16;
        int16_t values[8];
        int32_t data;
    } combined __attribute__((aligned(16)));
    
    /* Source computations */
    register uint32_t r1 = 0xDEADBEEF;
    register uint32_t r2 = 0xCAFEBABE;
    register int32_t calc = (r1 & 0xFFFF) + (r2 >> 16);
    
    /* Combined assignment: bitfield + array with complex index */
    combined.flags = (r1 & 0xFF) | ((r2 & 0xFF) << 8);  /* ZERO_EXTRACT potential */
    
    /* Array store with narrowing */
    int index = ((r1 & 0x7) * 3 + 5) & 0x7;  /* Complex but bounded index */
    combined.values[index] = (int16_t)calc;  /* SUBREG potential */
    
    /* Complex memory store */
    combined.data = calc * 2 + (r1 >> 24);
    
    sink = combined.flags + combined.values[0] + combined.data;
}

/* Test 5: Inline assembly for direct RTL influence */
void test_asm_patterns(void) {
    int16_t array[64] __attribute__((aligned(16)));
    int32_t big_array[32] __attribute__((aligned(16)));
    
    /* Initialize */
    for (int i = 0; i < 64; i++) array[i] = i * 3;
    for (int i = 0; i < 32; i++) big_array[i] = i * 5;
    
    /* Complex addressing in asm output */
    register int idx1 = 7 * 3 + 4;
    register int idx2 = 11 * 2 + 5;
    
    /* Memory output with complex addressing - may generate MEM with complex address */
    asm volatile (
        "# Force memory store with complex address\n"
        : "=m" (array[idx1]), "=m" (big_array[idx2])
        : 
        : "memory"
    );
    
    /* Bitfield-like operation via asm */
    volatile uint32_t bitfield = 0;
    asm volatile (
        "# Manipulate specific bits\n"
        : "+r" (bitfield)
        : 
        : "cc"
    );
    
    sink = array[0] + big_array[0] + bitfield;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    checksum += sink;
    
    test_subreg_operations();
    checksum += sink;
    
    test_complex_addressing();
    checksum += sink;
    
    test_combined_patterns();
    checksum += sink;
    
    test_asm_patterns();
    checksum += sink;
    
    /* Final output to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
