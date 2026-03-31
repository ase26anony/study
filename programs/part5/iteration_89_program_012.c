/* 
 * Test program to trigger uncovered lines in resource.cc (lines 282-290)
 * Specifically targets RTL patterns: ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, MEM
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_volatile_input = 0;

/* Bit-field structures for ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG */
union mixed_types {
    uint32_t full;
    uint16_t half[2];
    uint8_t bytes[4];
};

/* Global with bit-fields */
struct bitfield_struct g_bf = {0};

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_assignments(int seed) {
    struct bitfield_struct local_bf;
    
    /* Multiple bit-field writes - may generate ZERO_EXTRACT */
    local_bf.a = (seed >> 0) & 0x7;
    local_bf.b = (seed >> 3) & 0x1F;
    local_bf.c = (seed >> 8) & 0xFF;
    local_bf.d = (seed >> 16) & 0xFFFF;
    
    /* Copy to global with volatile read to prevent optimization */
    g_bf = local_bf;
    
    /* Complex expression with bit-fields */
    g_bf.c = (local_bf.a + local_bf.b) & 0xFF;
}

/* Test 2: Inline assembly with register variables for reload stress */
void test_asm_register_clobber(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    register uint32_t reg_var2 asm("r13") = 0;
    
    /* Inline asm with clobbers to force reload pass */
    asm volatile (
        "mov %1, %0\n\t"
        "ror $8, %0"
        : "=r" (reg_var2)
        : "r" (reg_var)
        : /* clobbers */
    );
    
    /* Use result in bit-field to combine patterns */
    g_bf.d = (reg_var2 >> 16) & 0xFFFF;
}

/* Test 3: Memory accesses with volatile and type-punning for SUBREG/MEM */
void test_memory_subreg(void) {
    volatile uint32_t mem_buffer[4] = {0};
    union mixed_types *pun;
    
    /* Type-punning through union - may generate SUBREG */
    pun = (union mixed_types *)&mem_buffer[0];
    
    /* Write through different views of same memory */
    pun->full = 0xDEADBEEF;
    pun->half[1] = 0xCAFE;      /* Potential SUBREG of MEM */
    pun->bytes[0] = g_volatile_input & 0xFF;
    
    /* Pointer cast for SUBREG generation */
    uint16_t *short_ptr = (uint16_t *)&mem_buffer[1];
    *short_ptr = 0x1234;        /* SUBREG of MEM */
    
    /* Complex addressing mode */
    uint32_t idx = g_volatile_input & 0x3;
    mem_buffer[idx] = pun->full + 1;  /* MEM with index */
}

/* Test 4: Loop with mixed operations to prevent optimization */
void test_loop_mixed_operations(int iterations) {
    volatile uint32_t buffer[8] = {0};
    struct bitfield_struct temp_bf = {0};
    
    for (int i = 0; i < iterations; i++) {
        /* Bit-field in loop - may use STRICT_LOW_PART */
        temp_bf.a = (i >> 0) & 0x7;
        temp_bf.b = (i >> 3) & 0x1F;
        
        /* Memory write with volatile */
        buffer[i % 8] = temp_bf.c;
        
        /* Type-punning within loop */
        union mixed_types *up = (union mixed_types *)&buffer[(i + 1) % 8];
        up->bytes[0] = temp_bf.a;
        up->half[1] = temp_bf.b;
        
        /* Prevent loop unrolling */
        if (g_volatile_input > 1000) break;
    }
    
    /* Store result to global */
    g_bf = temp_bf;
}

/* Test 5: Complex expression combining multiple patterns */
void test_complex_expression(void) {
    volatile uint32_t vmem[2];
    union mixed_types u;
    
    /* Initialize */
    u.full = 0xABCD1234;
    vmem[0] = u.full;
    
    /* Chain of operations that may generate various RTL patterns */
    uint32_t temp = vmem[0];
    
    /* Bit-field extraction and assignment */
    g_bf.a = (temp >> 0) & 0x7;
    g_bf.b = (temp >> 4) & 0x1F;
    
    /* SUBREG access through pointer */
    uint16_t *sptr = (uint16_t *)&vmem[1];
    *sptr = g_bf.c;  /* SUBREG of MEM */
    
    /* Another memory access */
    vmem[1] = (g_bf.d << 16) | g_bf.c;
}

int main(int argc, char *argv[]) {
    int iterations = 4;
    
    /* Use command line argument for runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]) % 8 + 1;
    }
    
    printf("Testing RTL pattern generation for resource tracking...\n");
    
    /* Run all tests */
    test_bitfield_assignments(0x89ABCDEF);
    test_asm_register_clobber();
    test_memory_subreg();
    test_loop_mixed_operations(iterations);
    test_complex_expression();
    
    /* Print checksum to ensure execution */
    uint32_t checksum = g_bf.a + g_bf.b + g_bf.c + g_bf.d;
    printf("Checksum: %u\n", checksum);
    printf("Global bit-field values: a=%u, b=%u, c=%u, d=%u\n", 
           g_bf.a, g_bf.b, g_bf.c, g_bf.d);
    
    return 0;
}
