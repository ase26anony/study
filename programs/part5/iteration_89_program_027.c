/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_resource test_resource.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force runtime dependency to prevent constant propagation */
volatile int g_volatile_input = 0;

/* ==================== BIT-FIELD STRUCTURES ==================== */
/* Struct with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG */
union mixed_access {
    uint32_t full;
    uint16_t half[2];
    uint8_t bytes[4];
};

/* ==================== TEST FUNCTIONS ==================== */

/* Test 1: Bit-field assignments in loop - should generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent assignments prevent optimization */
        bf->a = (i + g_volatile_input) & 0x7;
        bf->b = (i * 3) & 0x1F;
        bf->c = (i + g_volatile_input * 2) & 0xFF;
        bf->d = (i * 5) & 0xFFFF;
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    register uint32_t result asm("r11") = 0;
    
    /* Inline asm that clobbers registers, forcing reload to manage resources */
    asm volatile (
        "movl %1, %%eax\n\t"
        "rorl $8, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (reg_var)
        : "%eax", "cc"
    );
    
    /* Use result to prevent dead code elimination */
    g_volatile_input = result & 1;
}

/* Test 3: Memory operations with SUBREG and MEM patterns */
void test_mem_subreg(volatile uint32_t *mem, int size) {
    union mixed_access *ptr = (union mixed_access *)mem;
    
    for (int i = 0; i < size; i++) {
        /* Access different parts of the same memory location */
        ptr[i].half[0] = (i + g_volatile_input) & 0xFFFF;      /* SUBREG of MEM */
        ptr[i].bytes[2] = (i * 7) & 0xFF;                     /* Another SUBREG */
        ptr[i].full ^= 0x00FF00FF;                           /* Full MEM access */
    }
}

/* Test 4: Complex addressing modes with volatile */
void test_complex_addressing(volatile uint32_t *base, int offset) {
    /* Complex addressing that may generate MEM with non-trivial address */
    volatile uint32_t *ptr = base + offset + g_volatile_input;
    
    for (int i = 0; i < 10; i++) {
        *ptr = *ptr + i;
        ptr += (i & 3) + 1;  /* Variable stride */
    }
}

/* Test 5: Mixed size accesses through pointers */
void test_mixed_size_access(void *buffer) {
    volatile uint8_t *p8 = (volatile uint8_t *)buffer;
    volatile uint16_t *p16 = (volatile uint16_t *)buffer;
    volatile uint32_t *p32 = (volatile uint32_t *)buffer;
    
    /* Access same memory with different sized operations */
    *p32 = 0xDEADBEEF;
    *p16 = 0x1234;          /* SUBREG store */
    p8[2] = 0xAB;           /* Another SUBREG */
    
    /* Create data dependency */
    if (*p8 & 0x1) {
        p16[1] = g_volatile_input;
    }
}

/* ==================== MAIN DRIVER ==================== */
int main(int argc, char *argv[]) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize test data */
    struct bitfield_struct bf = {0};
    uint32_t *mem_buffer = (uint32_t *)malloc(256 * sizeof(uint32_t));
    volatile uint32_t *vol_buffer = (volatile uint32_t *)malloc(128 * sizeof(uint32_t));
    
    if (!mem_buffer || !vol_buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = i * 0x01010101;
    }
    for (int i = 0; i < 128; i++) {
        vol_buffer[i] = i;
    }
    
    printf("Starting resource tracking tests...\n");
    
    /* Run all tests to trigger different RTL patterns */
    test_bitfield_ops(&bf, iterations);
    test_asm_clobber();
    test_mem_subreg(vol_buffer, 64);
    test_complex_addressing(vol_buffer, 16);
    test_mixed_size_access(mem_buffer);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    checksum ^= bf.a + bf.b + bf.c + bf.d;
    for (int i = 0; i < 64; i++) {
        checksum ^= mem_buffer[i];
        checksum ^= vol_buffer[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    free(mem_buffer);
    free((void *)vol_buffer);
    
    return 0;
}
