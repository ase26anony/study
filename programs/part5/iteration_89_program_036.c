/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_program test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_vol_input = 0;

/* Struct with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG */
union mixed_types {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* Global with volatile to force MEM RTL patterns */
volatile uint32_t g_mem_buffer[256];

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Complex assignments that may generate STRICT_LOW_PART */
        bf->a = (i & 0xF);
        bf->b = (i >> 4) & 0xFF;
        bf->c = (i * 3) & 0xFFF;
        bf->d = (bf->a + bf->b) & 0xFF;
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    uint32_t tmp;
    
    /* Inline asm that clobbers hard register */
    asm volatile (
        "mov %[tmp], %[reg]\n\t"
        "ror %[tmp], #8\n\t"
        : [tmp] "=r" (tmp)
        : [reg] "r" (reg_var)
        : "cc"
    );
    
    /* Use result to prevent optimization */
    g_mem_buffer[0] = tmp;
}

/* Test 3: Mixed-type accesses via union to generate SUBREG */
void test_mixed_type_access(union mixed_types *u, int idx) {
    /* Write word, then access sub-parts (may generate SUBREG) */
    u->word = 0xDEADBEEF;
    
    /* These accesses may generate SUBREG of MEM */
    u->half[0] = (uint16_t)(g_vol_input & 0xFFFF);
    u->byte[2] = (uint8_t)(idx & 0xFF);
    
    /* Volatile read to force MEM RTL */
    volatile uint16_t *vol_ptr = (volatile uint16_t *)&u->word;
    *vol_ptr = *vol_ptr + 1;
}

/* Test 4: Complex memory addressing modes for MEM_P(x) */
void test_complex_mem_addressing(int offset) {
    volatile uint32_t *ptr = &g_mem_buffer[128];
    
    /* Complex addressing with variable offset */
    for (int i = 0; i < 32; i++) {
        ptr[i + offset] = ptr[i] ^ 0xAAAAAAAA;
        ptr[i + offset + 1] = ptr[i + offset] + g_vol_input;
    }
}

/* Test 5: Pointer casting for SUBREG generation */
void test_pointer_casts(void) {
    uint32_t array[16];
    volatile uint8_t *byte_ptr = (volatile uint8_t *)array;
    
    /* Write 32-bit, then access as bytes (may generate SUBREG) */
    for (int i = 0; i < 4; i++) {
        array[i] = i * 0x11111111;
        byte_ptr[i * 4 + 1] = g_vol_input & 0xFF;
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations <= 0) iterations = 100;
    
    struct bitfield_struct bf = {0};
    union mixed_types u;
    
    printf("Starting RTL pattern tests...\n");
    
    /* Run all tests to generate various RTL patterns */
    test_bitfield_ops(&bf, iterations);
    test_asm_clobber();
    test_mixed_type_access(&u, iterations);
    test_complex_mem_addressing(g_vol_input & 0xF);
    test_pointer_casts();
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    checksum ^= bf.a + bf.b + bf.c + bf.d;
    checksum ^= u.word;
    checksum ^= g_mem_buffer[0];
    checksum ^= g_mem_buffer[128];
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
