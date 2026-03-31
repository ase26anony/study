/* test_resource_cc.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc (lines 282-290) by generating RTL patterns that involve
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM expressions.
 * 
 * Compilation recommendations:
 *   gcc -O2 -fdump-rtl-all -fdump-tree-all -o test test_resource_cc.c
 *   gcc -O3 -fno-strict-aliasing -frename-registers -o test test_resource_cc.c
 *   gcc -O1 -fschedule-insns -fno-merge-bitfields -o test test_resource_cc.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* ========== Bit-field structures for ZERO_EXTRACT/STRICT_LOW_PART ========== */

/* Packed struct with multiple bit-fields to force ZERO_EXTRACT */
struct bitfield_packet {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

/* Struct with volatile bit-field to prevent optimization */
struct volatile_bitfield {
    volatile unsigned int flag : 1;
    volatile unsigned int value : 7;
    volatile unsigned int mode : 4;
};

/* ========== Unions for type-punning and SUBREG generation ========== */

union mixed_access {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
    struct {
        uint8_t b0 : 2;
        uint8_t b1 : 2;
        uint8_t b2 : 2;
        uint8_t b3 : 2;
    } bits;
};

/* ========== Volatile memory buffers for MEM operations ========== */

volatile uint32_t mem_buffer[256];
volatile uint16_t short_buffer[512];
volatile uint8_t byte_buffer[1024];

/* ========== Test functions ========== */

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(int iterations) {
    struct bitfield_packet bf = {0};
    struct volatile_bitfield vbf = {0};
    
    for (int i = 0; i < iterations; i++) {
        /* These assignments may generate ZERO_EXTRACT or STRICT_LOW_PART */
        bf.a = (i & 0x7);                    /* 3-bit field */
        bf.b = ((i >> 3) & 0x1F);            /* 5-bit field */
        bf.c = ((i >> 8) & 0xFF);            /* 8-bit field */
        bf.d = ((i >> 16) & 0xFFFF);         /* 16-bit field */
        
        /* Volatile bit-field access - prevents optimization */
        vbf.flag = (i & 1);
        vbf.value = (i & 0x7F);
        vbf.mode = ((i >> 7) & 0xF);
        
        /* Use values to prevent dead code elimination */
        mem_buffer[i % 256] = bf.d + vbf.value;
    }
}

/* Test 2: Mixed-type accesses via unions to generate SUBREG */
void test_mixed_type_access(int iterations) {
    union mixed_access ma;
    volatile int idx = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Initialize with volatile to prevent constant propagation */
        idx = (i * 13) % 256;
        
        /* Whole word access */
        ma.word = mem_buffer[idx];
        
        /* Sub-word accesses that may generate SUBREG */
        ma.half[0] = short_buffer[idx * 2];
        ma.half[1] = short_buffer[idx * 2 + 1];
        
        /* Byte access - potential SUBREG of MEM */
        ma.byte[0] = byte_buffer[idx * 4];
        ma.byte[1] = byte_buffer[idx * 4 + 1];
        ma.byte[2] = byte_buffer[idx * 4 + 2];
        ma.byte[3] = byte_buffer[idx * 4 + 3];
        
        /* Bit-field access within union */
        ma.bits.b0 = (i & 0x3);
        ma.bits.b1 = ((i >> 2) & 0x3);
        
        /* Write back to memory */
        mem_buffer[idx] = ma.word;
    }
}

/* Test 3: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(int iterations) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    struct bitfield_packet bf_local = {0};
    
    for (int i = 0; i < iterations; i++) {
        /* Move between C variable and hard register */
        asm volatile (
            "mov %[reg], %[val]\n\t"
            : [reg] "+r" (reg_var)
            : [val] "r" (i)
            : /* No clobbers - using specific register constraint */
        );
        
        /* Use the register variable in bit-field assignment */
        bf_local.c = (reg_var & 0xFF);
        
        /* Inline asm with memory clobber to force MEM RTL */
        asm volatile (
            "mov %[mem], %[reg]\n\t"
            : [mem] "=m" (mem_buffer[i % 256])
            : [reg] "r" (reg_var)
            : "memory"
        );
    }
}

/* Test 4: Complex addressing modes for MEM_P(x) */
void test_complex_addressing(int iterations) {
    volatile uint32_t * volatile ptr_array[4];
    volatile uint16_t *base_ptr;
    
    /* Setup pointers with different bases */
    ptr_array[0] = &mem_buffer[0];
    ptr_array[1] = &mem_buffer[64];
    ptr_array[2] = &mem_buffer[128];
    ptr_array[3] = &mem_buffer[192];
    
    base_ptr = (volatile uint16_t *)&mem_buffer[0];
    
    for (int i = 0; i < iterations; i++) {
        int idx = i & 3;
        int offset = (i * 7) % 64;
        
        /* Complex addressing: pointer array + offset */
        uint32_t val = *ptr_array[idx] + offset;
        
        /* Write with complex addressing - should generate MEM RTL */
        *ptr_array[(idx + 1) & 3] = val;
        
        /* SUBREG of MEM: accessing 16-bit portion of 32-bit memory */
        uint16_t half_val = base_ptr[offset * 2];
        base_ptr[offset * 2 + 1] = half_val + 1;
    }
}

/* Test 5: Pointer casting for SUBREG generation */
void test_pointer_casting(int iterations) {
    volatile uint32_t *word_ptr = &mem_buffer[0];
    volatile uint8_t *byte_ptr = (volatile uint8_t *)word_ptr;
    volatile uint16_t *short_ptr = (volatile uint16_t *)word_ptr;
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % 128;
        
        /* Write 32-bit value */
        word_ptr[idx] = i * 0x01010101;
        
        /* Access as smaller types - may generate SUBREG */
        byte_ptr[idx * 4 + 1] = (i & 0xFF);
        short_ptr[idx * 2] = (i & 0xFFFF);
        
        /* Cast back and forth */
        *(volatile uint16_t *)((volatile uint8_t *)&word_ptr[idx] + 1) = 
            (uint16_t)(i * 3);
    }
}

/* ========== Main function ========== */

int main(int argc, char *argv[]) {
    int iterations = 100;
    
    /* Use command-line argument for runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
        if (iterations > 1000) iterations = 1000;
    }
    
    printf("Testing resource.cc uncovered lines (282-290)\n");
    printf("Iterations: %d\n\n", iterations);
    
    /* Initialize buffers with pattern */
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = i;
        short_buffer[i] = i & 0xFFFF;
        byte_buffer[i] = i & 0xFF;
    }
    
    /* Run all tests */
    test_bitfield_ops(iterations);
    printf("Test 1 (bit-field) completed\n");
    
    test_mixed_type_access(iterations);
    printf("Test 2 (mixed-type) completed\n");
    
    test_asm_clobber(iterations);
    printf("Test 3 (asm clobber) completed\n");
    
    test_complex_addressing(iterations);
    printf("Test 4 (complex addressing) completed\n");
    
    test_pointer_casting(iterations);
    printf("Test 5 (pointer casting) completed\n");
    
    /* Compute checksum to verify execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += mem_buffer[i];
        checksum += short_buffer[i];
        checksum += byte_buffer[i];
    }
    
    printf("\nFinal checksum: 0x%08X\n", checksum);
    printf("(If checksum varies between runs, it's due to volatile/asm)\n");
    
    return 0;
}
