/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o coverage_test coverage_test.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_volatile_input = 0;

/* ========== BIT-FIELD STRUCTURES ========== */
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
    uint8_t byte[4];
};

/* ========== TEST FUNCTIONS ========== */

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf) {
    /* Data-dependent assignments prevent optimization */
    int idx = g_volatile_input & 3;
    
    /* Multiple bit-field writes - may generate ZERO_EXTRACT */
    bf->a = idx;
    bf->b = idx * 2;
    bf->c = idx * 3;
    bf->d = idx * 100;
    
    /* Nested conditional with bit-field access */
    if (bf->a > 1) {
        bf->b = bf->c ^ bf->a;
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    struct bitfield_struct local_bf;
    
    /* Inline asm that clobbers hard register */
    asm volatile (
        "mov %[input], %[output]\n\t"
        : [output] "=r" (reg_var)
        : [input] "r" (g_volatile_input)
        : "r12"  /* Explicit clobber */
    );
    
    /* Use the register variable in bit-field operation */
    local_bf.c = (reg_var >> 4) & 0xFF;
    
    /* Another asm to force reload */
    asm volatile (
        "add $1, %0\n\t"
        : "+r" (reg_var)
        :
        : "cc"
    );
    
    local_bf.d = reg_var & 0xFFFF;
}

/* Test 3: Memory accesses with volatile and type-punning for SUBREG/MEM */
void test_mem_subreg(volatile uint32_t *mem) {
    union mixed_access *u = (union mixed_access *)mem;
    
    /* Write through different-sized views - may generate SUBREG */
    u->half[0] = g_volatile_input & 0xFFFF;
    u->byte[2] = (g_volatile_input >> 16) & 0xFF;
    
    /* Volatile memory write with complex addressing */
    *(volatile uint16_t *)((uint8_t *)mem + 1) = 0xABCD;
    
    /* Pointer cast for sub-word access */
    uint8_t *byte_ptr = (uint8_t *)mem;
    byte_ptr[g_volatile_input & 3] = 0xFF;
}

/* Test 4: Loop with mixed memory operations */
void test_loop_mixed_ops(volatile uint32_t *buffer, int size) {
    struct bitfield_struct bf_array[4];
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent index prevents loop unrolling */
        int idx = (i + g_volatile_input) & 3;
        
        /* Bit-field operation in loop */
        bf_array[idx].a = i & 0x7;
        bf_array[idx].b = (i >> 3) & 0x1F;
        
        /* Memory access with volatile */
        buffer[i] = bf_array[idx].c;
        
        /* Type-punning within loop */
        union mixed_access *u = (union mixed_access *)&buffer[i];
        u->half[1] = bf_array[idx].d;
    }
}

/* Test 5: Complex addressing mode for MEM_P */
void test_complex_addressing(uint32_t *base, int offset) {
    /* Volatile pointer with indexed addressing */
    volatile uint32_t *vol_ptr = base + offset;
    
    /* Multiple memory writes with different offsets */
    vol_ptr[0] = 0xDEADBEEF;
    vol_ptr[g_volatile_input & 1] = 0xCAFEBABE;
    
    /* Pointer arithmetic in memory access */
    *(volatile uint32_t *)((char *)vol_ptr + 8) = 0x12345678;
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    /* Initialize test data */
    struct bitfield_struct bf = {0};
    volatile uint32_t mem_buffer[16] = {0};
    uint32_t regular_buffer[16] = {0};
    
    /* Use command-line argument for variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    g_volatile_input = seed;
    
    printf("Starting coverage test with seed=%d\n", seed);
    
    /* Execute all test patterns */
    test_bitfield_ops(&bf);
    test_asm_clobber();
    test_mem_subreg(mem_buffer);
    test_loop_mixed_ops(mem_buffer, 8);
    test_complex_addressing(regular_buffer, seed & 7);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum ^= mem_buffer[i];
        checksum ^= regular_buffer[i];
    }
    checksum ^= bf.a ^ bf.b ^ bf.c ^ bf.d;
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed successfully\n");
    
    return 0;
}
