/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o coverage_test coverage_test.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
static volatile int g_input = 0;

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

/* Volatile memory buffer for MEM RTL patterns */
static volatile uint32_t g_mem_buffer[256];

/* Test 1: Bit-field assignments with volatile control flow */
void test_bitfield_assignments(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent assignments to prevent optimization */
        if (g_input & (1 << (i & 3))) {
            bf->a = (i & 0xF);          /* 4-bit field */
            bf->b = (i * 3) & 0xFF;     /* 8-bit field */
        } else {
            bf->c = (i * 5) & 0xFFF;    /* 12-bit field */
            bf->d = (i * 7) & 0xFF;     /* 8-bit field */
        }
        
        /* Force memory barrier */
        asm volatile("" ::: "memory");
    }
}

/* Test 2: Inline assembly with register clobbers */
void test_asm_clobbers(void) {
    register uint32_t reg_var asm("r12") = g_input;
    struct bitfield_struct local_bf;
    
    /* Inline asm that clobbers hard registers */
    asm volatile (
        "mov %[input], %[reg]\n\t"
        "and $0xF, %[reg]\n\t"
        : [reg] "+r" (reg_var)
        : [input] "r" (g_input)
        : "cc", "memory"
    );
    
    /* Use the register variable in bit-field assignment */
    local_bf.a = reg_var & 0xF;
    local_bf.b = (reg_var >> 4) & 0xFF;
    
    /* Another asm to force reload */
    asm volatile (
        "add $1, %0\n\t"
        : "+r" (reg_var)
        :: "cc"
    );
    
    local_bf.c = reg_var & 0xFFF;
    
    /* Prevent dead code elimination */
    g_mem_buffer[0] = local_bf.a + local_bf.b + local_bf.c;
}

/* Test 3: Mixed-type accesses via unions and pointers */
void test_mixed_type_accesses(int index) {
    union mixed_types *pun = (union mixed_types *)&g_mem_buffer[index];
    
    /* Write whole word (MEM RTL) */
    pun->word = g_input * 0x12345678;
    
    /* Write half-word (SUBREG of MEM) */
    pun->half[1] = (g_input * 0xABCD) & 0xFFFF;
    
    /* Write bytes (SUBREG of MEM) */
    for (int i = 0; i < 4; i++) {
        pun->byte[i] = (g_input + i) & 0xFF;
    }
    
    /* Pointer casting for SUBREG generation */
    volatile uint16_t *short_ptr = (volatile uint16_t *)&g_mem_buffer[index + 1];
    *short_ptr = g_input & 0xFFFF;
    
    volatile uint8_t *char_ptr = (volatile uint8_t *)&g_mem_buffer[index + 2];
    *char_ptr = (g_input >> 8) & 0xFF;
}

/* Test 4: Complex addressing modes with volatile */
void test_complex_addressing(int base_idx) {
    volatile uint32_t *ptr;
    
    /* Data-dependent pointer arithmetic */
    if (g_input & 1) {
        ptr = &g_mem_buffer[base_idx + (g_input & 0x3F)];
    } else {
        ptr = &g_mem_buffer[base_idx - (g_input & 0x1F)];
    }
    
    /* Memory write with complex addressing (MEM RTL) */
    *ptr = g_input * 0xDEADBEEF;
    
    /* Pointer to bit-field struct in memory */
    struct bitfield_struct *bf_ptr = (struct bitfield_struct *)ptr;
    bf_ptr->a = (g_input >> 4) & 0xF;
    bf_ptr->b = (g_input >> 8) & 0xFF;
}

/* Test 5: Loop with volatile memory updates */
void test_volatile_loop(int iterations) {
    volatile uint32_t accumulator = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Volatile read-modify-write */
        accumulator = g_mem_buffer[i];
        
        /* Data-dependent update */
        if (accumulator & 1) {
            g_mem_buffer[i] = accumulator * 3;
        } else {
            g_mem_buffer[i] = accumulator / 2;
        }
        
        /* Mixed-type access within loop */
        volatile uint16_t *short_view = (volatile uint16_t *)&g_mem_buffer[i];
        short_view[0] = short_view[0] ^ (i & 0xFFFF);
    }
}

int main(int argc, char *argv[]) {
    struct bitfield_struct bf_instance = {0};
    int iterations = 100;
    
    /* Use command-line argument for runtime variability */
    if (argc > 1) {
        g_input = atoi(argv[1]);
    } else {
        g_input = 42;  /* Default seed */
    }
    
    printf("Starting coverage test with input: %d\n", g_input);
    
    /* Initialize memory buffer */
    for (int i = 0; i < 256; i++) {
        g_mem_buffer[i] = i * 0x10001;
    }
    
    /* Run all tests */
    test_bitfield_assignments(&bf_instance, iterations);
    test_asm_clobbers();
    test_mixed_type_accesses(g_input & 0x7F);
    test_complex_addressing(64);
    test_volatile_loop(iterations / 2);
    
    /* Compute checksum to verify execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= g_mem_buffer[i];
    }
    checksum ^= bf_instance.a + bf_instance.b + bf_instance.c + bf_instance.d;
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
