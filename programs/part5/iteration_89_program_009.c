#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Test structures with bit-fields for ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG patterns */
union mixed_access {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* Volatile memory buffer for MEM operations */
volatile uint32_t mem_buffer[256];

/* Function 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, volatile int seed) {
    for (int i = 0; i < 100; i++) {
        /* Data-dependent assignments prevent constant propagation */
        bf->a = (seed + i) & 0xF;
        bf->b = (seed * i) & 0xFF;
        bf->c = (seed ^ i) & 0xFFF;
        bf->d = (seed - i) & 0xFF;
        
        /* Force register allocation conflict */
        register uint32_t tmp asm("r12") = bf->c;
        asm volatile ("" : "+r" (tmp));
        bf->b = tmp & 0xFF;
    }
}

/* Function 2: Mixed-type accesses to generate SUBREG patterns */
void test_subreg_ops(union mixed_access *ma, volatile int index) {
    /* Access different parts of the same memory with different types */
    ma->word = 0x12345678;
    
    /* SUBREG generation through type-punning */
    ma->half[0] = (index & 0xFFFF);
    ma->byte[2] = (index >> 8) & 0xFF;
    
    /* Complex addressing mode */
    volatile uint16_t *ptr = (volatile uint16_t *)&ma->word;
    ptr[1] = ma->byte[3] | 0x100;
}

/* Function 3: Memory operations for MEM_P patterns */
void test_mem_ops(volatile uint32_t *buffer, volatile int size) {
    volatile uint32_t *dest = buffer;
    volatile uint32_t *src = buffer + 128;
    
    for (int i = 0; i < size; i++) {
        /* Complex memory addressing with index */
        dest[i] = src[(i * 13) & 0x7F] ^ 0xAA55AA55;
        
        /* Inline assembly with memory clobber to force reload */
        asm volatile ("# Memory barrier" : : : "memory");
        
        /* SUBREG of MEM through byte access */
        volatile uint8_t *byte_ptr = (volatile uint8_t *)&dest[i];
        byte_ptr[2] = i & 0xFF;
    }
}

/* Function 4: Inline assembly with hard register clobbers */
void test_asm_clobbers(volatile int *input, volatile int *output) {
    register int a asm("r10") = *input;
    register int b asm("r11") = *input + 1;
    
    /* Inline assembly that ties C variables to hard registers */
    asm volatile (
        "add %1, %0\n\t"
        "ror %0, #8\n\t"
        : "+r" (a), "+r" (b)
        : 
        : "cc"
    );
    
    /* Bit-field operation on register variable */
    struct bitfield_struct bf_local;
    bf_local.a = a & 0xF;
    bf_local.b = b & 0xFF;
    
    /* This may generate STRICT_LOW_PART for register allocation */
    *output = bf_local.b | (bf_local.a << 8);
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for runtime variability */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize test data */
    struct bitfield_struct bf_data = {0};
    union mixed_access ma_data;
    volatile uint32_t checksum = 0;
    
    /* Run test functions in data-dependent order */
    if (seed & 1) {
        test_bitfield_ops(&bf_data, seed);
        checksum += bf_data.a + bf_data.b + bf_data.c + bf_data.d;
    }
    
    if (seed & 2) {
        test_subreg_ops(&ma_data, seed);
        for (int i = 0; i < 4; i++) {
            checksum += ma_data.byte[i];
        }
    }
    
    if (seed & 4) {
        /* Initialize memory buffer */
        for (int i = 0; i < 256; i++) {
            mem_buffer[i] = i * 0x01010101;
        }
        
        test_mem_ops(mem_buffer, 64);
        for (int i = 0; i < 64; i++) {
            checksum ^= mem_buffer[i];
        }
    }
    
    if (seed & 8) {
        volatile int in = seed * 3;
        volatile int out;
        test_asm_clobbers(&in, &out);
        checksum += out;
    }
    
    /* Print result to ensure execution */
    printf("Checksum: 0x%08X\n", checksum);
    
    return 0;
}
