/* test_resource_marking.c - Generate RTL patterns for ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P in SET_DEST */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations alive */
static volatile int sink;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int dummy : 8;     /* Padding */
    } bf = {0};
    
    /* Source values with different bit patterns */
    register unsigned int r1 = 0xABCD;
    register unsigned int r2 = 0x1234;
    register unsigned int r3 = 0x5678;
    
    /* Complex assignments to bitfields - may generate ZERO_EXTRACT in SET_DEST */
    bf.field4 = (r1 & 0xF) + (r2 & 0x1);      /* 4-bit field with computation */
    bf.field8 = (r1 >> 4) & 0xFF;             /* 8-bit extract from register */
    bf.field12 = ((r2 & 0xFFF) | (r3 & 0xF00)) ^ 0x555;  /* Complex 12-bit expression */
    
    /* Read back to prevent elimination */
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: Sub-word type operations for SUBREG */
void test_subword_operations(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32 = 0xDEADBEEF;
    
    /* Register sources with different sizes */
    register int32_t r32 = 0x12345678;
    register int64_t r64 = 0x9ABCDEF012345678ULL;
    
    /* Narrowing assignments - may generate SUBREG in SET_DEST */
    v8 = (int8_t)(r32 + 0x100);      /* 32-bit to 8-bit with overflow */
    v16 = (int16_t)((r32 >> 8) & 0xFFFF);  /* Extract middle 16 bits */
    
    /* Complex narrowing with arithmetic */
    v8 = (int8_t)((r32 & 0xFF) + (r64 & 0x7F) - 50);
    v16 = (int16_t)((r32 * 3) / 7);  /* Arithmetic then narrow */
    
    /* Read back */
    sink = v8 + v16 + v32;
}

/* Test 3: Complex memory addressing for MEM_P */
void test_complex_memory_addressing(void) {
    /* Local arrays with restrict to avoid aliasing assumptions */
    int16_t arr16[256] __attribute__((aligned(16)));
    int32_t arr32[128] __attribute__((aligned(16)));
    
    /* Struct with array member */
    struct {
        int16_t data[64];
        int32_t index;
    } s = {0};
    
    register int32_t r1 = 0x1111;
    register int32_t r2 = 0x2222;
    register int32_t r3 = 0x3333;
    
    /* Complex array indexing - non-linear address computation */
    for (int i = 0; i < 32; i++) {
        /* Multi-dimensional style indexing */
        int idx = (i * 7 + 13) & 0xFF;
        arr16[idx] = (int16_t)(r1 + i);  /* Narrow store to complex address */
        
        /* Pointer arithmetic with multiple offsets */
        int32_t *ptr = arr32 + (i * 3 % 128);
        *ptr = r2 - i;  /* Store through computed pointer */
        
        /* Struct member with index computation */
        s.data[(i * 5 + 7) % 64] = (int16_t)(r3 ^ i);
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 256; i++) sum += arr16[i];
    for (int i = 0; i < 128; i++) sum += arr32[i];
    for (int i = 0; i < 64; i++) sum += s.data[i];
    sink = sum + s.index;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfields and arrays */
    volatile struct combined {
        unsigned int header : 8;
        unsigned int flags : 4;
        int16_t payload[32];
        unsigned int footer : 12;
    } comb = {0};
    
    /* Register values */
    register uint32_t rval = 0x89ABCDEF;
    register int32_t ival = 0x76543210;
    
    /* Combined assignment: bitfield + array with complex addressing */
    comb.header = (rval >> 16) & 0xFF;  /* ZERO_EXTRACT candidate */
    
    /* Array store with narrowing and complex index */
    int idx = ((ival & 0xFF) * 3 + 7) % 32;
    comb.payload[idx] = (int16_t)(ival >> 8);  /* SUBREG + MEM_P candidate */
    
    /* Another bitfield with computation */
    comb.flags = ((rval & 0xF) ^ (ival & 0xF)) | 0x1;
    
    /* Complex array store in loop */
    for (int i = 0; i < 16; i++) {
        int j = (i * 11 + 5) % 32;
        comb.payload[j] = (int16_t)((rval + i * 0x100) & 0xFFFF);
    }
    
    comb.footer = (rval >> 4) & 0xFFF;
    
    /* Compute checksum */
    int sum = comb.header + comb.flags + comb.footer;
    for (int i = 0; i < 32; i++) sum += comb.payload[i];
    sink = sum;
}

/* Test 5: Inline assembly for direct RTL influence */
void test_inline_asm(void) {
    int32_t array[64] __attribute__((aligned(16)));
    int16_t short_array[128] __attribute__((aligned(16)));
    
    register int32_t r1 asm("r12") = 0x12345678;
    register int32_t r2 asm("r13") = 0x9ABCDEF0;
    
    /* Inline asm with complex memory destination */
    int idx = (r1 & 0x3F) * 2 + 1;
    asm volatile (
        "# Force complex memory store\n"
        : "=m" (array[idx])  /* Complex addressing in output constraint */
        : "r" (r1)
        : "memory"
    );
    
    /* Another asm with different addressing mode */
    idx = ((r2 >> 4) & 0x7F) + 16;
    asm volatile (
        ""
        : "=m" (short_array[idx])  /* SUBREG + MEM_P potential */
        : "r" (r2)
        : "memory"
    );
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) sum += array[i];
    for (int i = 0; i < 128; i++) sum += short_array[i];
    sink = sum + r1 + r2;
}

/* Test 6: Bit manipulation builtins */
void test_bit_builtins(void) {
    volatile struct {
        unsigned int parity_field : 1;
        unsigned int popcount_field : 4;
        unsigned int extract_field : 6;
    } bits = {0};
    
    register uint32_t r = 0xB7;  /* 10110111 binary */
    
    /* Builtins that may involve bit extraction */
    bits.parity_field = __builtin_parity(r);      /* Parity of all bits */
    bits.popcount_field = __builtin_popcount(r) & 0xF;  /* Population count */
    
    /* Manual extraction that might become ZERO_EXTRACT */
    bits.extract_field = (r >> 2) & 0x3F;  /* Extract bits 2-7 */
    
    sink = bits.parity_field + bits.popcount_field + bits.extract_field;
}

int main(void) {
    int total = 0;
    
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Run all tests */
    test_bitfield_operations();
    total += sink;
    
    test_subword_operations();
    total += sink;
    
    test_complex_memory_addressing();
    total += sink;
    
    test_combined_patterns();
    total += sink;
    
    test_inline_asm();
    total += sink;
    
    test_bit_builtins();
    total += sink;
    
    printf("Total checksum: %d\n", total);
    printf("All tests completed.\n");
    
    return total != 0 ? 0 : 1;  /* Return 0 if any computation happened */
}
