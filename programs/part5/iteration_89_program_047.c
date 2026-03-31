/* 
 * Test program targeting uncovered lines in GCC's resource.cc
 * Specifically aims to trigger mark_set_resources paths for:
 * - ZERO_EXTRACT / STRICT_LOW_PART (bit-field assignments)
 * - SUBREG of MEM (subword memory operations)
 * - MEM_P (memory writes with complex addressing)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
static volatile int g_volatile_input = 0;

/* ========== BIT-FIELD STRUCTURES ========== */
/* Struct with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG */
union type_pun {
    uint32_t full;
    uint16_t half[2];
    uint8_t bytes[4];
};

/* ========== TEST FUNCTIONS ========== */

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_assignments(struct bitfield_struct *bf) {
    /* Use volatile input to create data-dependent assignments */
    int idx = g_volatile_input & 3;
    
    /* Multiple bit-field writes - may generate STRICT_LOW_PART */
    bf->a = idx;
    bf->b = (idx * 7) & 0x1F;
    bf->c = (idx * 13) & 0xFF;
    bf->d = (idx * 31) & 0xFFFF;
    
    /* Nested bit-field access in loop */
    for (int i = 0; i < 2; i++) {
        /* Complex expression prevents optimization */
        bf->b = (bf->a + i + g_volatile_input) & 0x1F;
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_inline_asm_clobber(void) {
    /* Register variable tied to specific register */
    register uint32_t reg_var asm ("r12") = 0x12345678;
    struct bitfield_struct bf_local;
    
    /* Inline assembly that clobbers registers */
    asm volatile (
        "mov %[input], %[output]\n\t"
        "add $0x1F, %[output]"
        : [output] "=r" (reg_var)
        : [input] "r" (g_volatile_input)
        : "cc", "memory"
    );
    
    /* Use register variable in bit-field assignment */
    bf_local.d = reg_var & 0xFFFF;  /* May generate STRICT_LOW_PART */
    
    /* More inline asm with memory clobber */
    asm volatile ("" : : "r" (reg_var) : "memory");
}

/* Test 3: Subword memory operations to generate SUBREG of MEM */
void test_subword_mem_ops(volatile uint8_t *mem) {
    union type_pun pun;
    
    /* Initialize with volatile to prevent optimization */
    pun.full = g_volatile_input;
    
    /* Write subword through pointer - may generate SUBREG */
    *(volatile uint16_t *)(mem + 1) = pun.half[0];
    
    /* Type-punning write of byte to word location */
    uint32_t *word_ptr = (uint32_t *)mem;
    *word_ptr = pun.bytes[0];  /* Implicit conversion may use SUBREG */
    
    /* Volatile access with different sizes */
    volatile uint16_t *short_ptr = (volatile uint16_t *)mem;
    for (int i = 0; i < 2; i++) {
        short_ptr[i] = pun.bytes[i] + i;
    }
}

/* Test 4: Complex memory addressing for MEM_P path */
void test_complex_mem_addressing(volatile int *arr, int size) {
    /* Data-dependent index prevents optimization */
    int idx = g_volatile_input % size;
    
    /* Complex addressing mode */
    arr[idx * 2 + 1] = idx * 3;
    
    /* Pointer arithmetic with volatile */
    volatile int *ptr = arr + idx;
    for (int i = 0; i < 3; i++) {
        ptr[i] = ptr[i] + g_volatile_input + i;
    }
}

/* Test 5: Mixed operations in loop with data-dependent flow */
void test_mixed_operations(void) {
    struct bitfield_struct bf_array[4];
    volatile uint8_t buffer[32];
    union type_pun pun;
    
    /* Initialize with volatile input */
    pun.full = g_volatile_input;
    
    for (int i = 0; i < 4; i++) {
        /* Data-dependent condition */
        if ((g_volatile_input >> i) & 1) {
            /* Bit-field assignment */
            bf_array[i].c = pun.bytes[i] & 0x7F;
            
            /* Subword memory write */
            *(volatile uint16_t *)&buffer[i * 4] = pun.half[i % 2];
        } else {
            /* Different bit-field assignment */
            bf_array[i].b = (pun.bytes[i] + i) & 0x1F;
            
            /* Byte memory write */
            buffer[i * 4 + 2] = pun.bytes[i];
        }
        
        /* Complex memory addressing */
        volatile int *int_ptr = (volatile int *)buffer;
        int_ptr[i] += bf_array[i].a;
    }
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    /* Initialize with command-line argument for runtime variability */
    if (argc > 1) {
        g_volatile_input = atoi(argv[1]);
    } else {
        g_volatile_input = 42;  /* Default test value */
    }
    
    /* Test data structures */
    struct bitfield_struct bf = {0};
    volatile uint8_t memory_buffer[64] = {0};
    volatile int int_array[16] = {0};
    
    printf("Starting resource tracking tests...\n");
    printf("Volatile input: %d\n", g_volatile_input);
    
    /* Run all tests */
    test_bitfield_assignments(&bf);
    test_inline_asm_clobber();
    test_subword_mem_ops(memory_buffer);
    test_complex_mem_addressing(int_array, 16);
    test_mixed_operations();
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    checksum += bf.a + bf.b + bf.c + bf.d;
    
    for (int i = 0; i < 64; i++) {
        checksum += memory_buffer[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += int_array[i];
    }
    
    printf("Checksum: %u\n", checksum);
    printf("Tests completed.\n");
    
    return 0;
}
