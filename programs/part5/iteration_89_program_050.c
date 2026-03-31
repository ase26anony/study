/* 
 * Test program to trigger uncovered lines in resource.cc (lines 282-290)
 * Specifically targeting SET_DEST patterns: ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
static volatile int g_input = 0;

/* ========== BIT-FIELD STRUCTURES ========== */
/* For ZERO_EXTRACT and STRICT_LOW_PART generation */

struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

struct nested_bitfields {
    struct {
        unsigned int low : 4;
        unsigned int high : 4;
    } byte1;
    struct {
        unsigned int low : 6;
        unsigned int high : 10;
    } word1;
} __attribute__((packed));

/* ========== TEST FUNCTIONS ========== */

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* These assignments often generate ZERO_EXTRACT or STRICT_LOW_PART in RTL */
        bf->a = (g_input + i) & 0x7;          /* 3-bit field */
        bf->b = (g_input * i) & 0x1F;         /* 5-bit field */
        bf->c = (g_input ^ i) & 0xFF;         /* 8-bit field */
        bf->d = (g_input + i * 2) & 0xFFFF;   /* 16-bit field */
        
        /* Mix with volatile to prevent optimization */
        asm volatile("" : "+m" (*bf));
    }
}

/* Test 2: Register variables with bit-fields for STRICT_LOW_PART */
void test_register_bitfield(void) {
    /* Register variable tied to specific register */
    register uint32_t reg_var asm("r12") = g_input;
    
    struct nested_bitfields nb;
    
    /* Access bit-fields through register variable */
    nb.byte1.low = reg_var & 0xF;
    nb.byte1.high = (reg_var >> 4) & 0xF;
    
    /* Force reload with inline asm clobber */
    asm volatile(
        "mov %[reg], %[temp]\n\t"
        : [temp] "=r" (reg_var)
        : [reg] "r" (reg_var)
        : "cc"
    );
    
    nb.word1.low = reg_var & 0x3F;
    nb.word1.high = (reg_var >> 6) & 0x3FF;
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+m" (nb));
}

/* Test 3: SUBREG generation through mixed-type accesses */
void test_subreg_mem_access(volatile uint32_t *mem, int size) {
    /* Access 32-bit memory as smaller types to generate SUBREG */
    for (int i = 0; i < size; i++) {
        /* Cast to different pointer types - may generate SUBREG in RTL */
        volatile uint16_t *ptr16 = (volatile uint16_t *)&mem[i];
        volatile uint8_t *ptr8 = (volatile uint8_t *)&mem[i];
        
        /* Mixed-size writes */
        *ptr16 = (g_input + i) & 0xFFFF;
        ptr8[2] = (g_input * i) & 0xFF;  /* Third byte */
        
        /* Union for type-punning (another SUBREG source) */
        union {
            uint32_t word;
            struct {
                uint8_t b0;
                uint8_t b1;
                uint8_t b2;
                uint8_t b3;
            } bytes;
        } converter;
        
        converter.word = mem[i];
        converter.bytes.b1 = i & 0xFF;
        mem[i] = converter.word;
    }
}

/* Test 4: Complex MEM addressing modes */
void test_complex_mem_addressing(volatile int *base, int offset, int count) {
    /* Complex addressing that survives to RTL generation */
    for (int i = 0; i < count; i++) {
        /* Address calculation with multiple components */
        volatile int *addr = base + (offset * i) / sizeof(int);
        
        /* Memory write with addressing mode that may generate MEM RTL */
        *addr = g_input + i;
        
        /* Additional indirection */
        volatile int **ptr_ptr = &addr;
        **ptr_ptr += 1;
    }
}

/* Test 5: Inline assembly with memory clobbers */
void test_asm_memory_clobber(void) {
    uint32_t data[4] = {0};
    uint32_t temp;
    
    /* Inline asm that forces memory reload */
    asm volatile(
        "movl %[in], %[out]\n\t"
        "movl %[out], (%[ptr])\n\t"
        : [out] "=r" (temp), [ptr] "+r" (data)
        : [in] "r" (g_input)
        : "memory"
    );
    
    /* Additional asm with specific register constraints */
    register uint32_t r1 asm("eax") = data[0];
    register uint32_t r2 asm("ebx") = data[1];
    
    asm volatile(
        "addl %%ebx, %%eax\n\t"
        : "+r" (r1)
        : "r" (r2)
        : "cc"
    );
    
    data[0] = r1;
}

/* ========== MAIN TEST DRIVER ========== */
int main(int argc, char *argv[]) {
    int iterations = 100;
    
    /* Use command line argument for runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    printf("Testing RTL pattern generation for resource tracking\n");
    printf("Iterations: %d\n", iterations);
    
    /* Initialize test structures */
    struct bitfield_struct bf = {0};
    volatile uint32_t memory_buffer[256] = {0};
    volatile int base_array[128] = {0};
    
    /* Update volatile input */
    g_input = iterations;
    
    /* Run test suite */
    test_bitfield_ops(&bf, iterations);
    test_register_bitfield();
    test_subreg_mem_access(memory_buffer, 64);
    test_complex_mem_addressing(base_array, 2, iterations % 32);
    test_asm_memory_clobber();
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    checksum ^= bf.a ^ bf.b ^ bf.c ^ bf.d;
    
    for (int i = 0; i < 64; i++) {
        checksum ^= memory_buffer[i];
    }
    
    for (int i = 0; i < 32; i++) {
        checksum ^= base_array[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed successfully\n");
    
    return 0;
}
