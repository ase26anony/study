#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Bit-field structures for ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} __attribute__((packed));

/* Union for type-punning and SUBREG generation */
union mixed_access {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
    struct bitfield_struct bits;
};

/* Volatile memory buffer for MEM operations */
volatile uint32_t mem_buffer[256];

/* Test 1: Bit-field assignments for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, volatile int seed) {
    /* Data-dependent assignments prevent constant propagation */
    if (seed & 1) {
        bf->a = (seed >> 1) & 0xF;  /* ZERO_EXTRACT likely */
    }
    if (seed & 2) {
        bf->b = (seed >> 4) & 0xFF; /* Another bit-field write */
    }
    /* Complex expression to force RTL generation */
    bf->c = ((seed * 3) + 7) & 0xFFF;
    bf->d = (bf->a + bf->b + bf->c) & 0xFF;
}

/* Test 2: Inline assembly with clobbers for reload stress */
void test_asm_clobber(volatile int *input, volatile int *output) {
    int temp;
    
    /* Inline asm that ties C variable to hard register */
    asm volatile (
        "mov %[in], %%r12\n\t"      /* Use specific register */
        "add $1, %%r12\n\t"
        "mov %%r12, %[out]\n\t"
        : [out] "=r" (temp)
        : [in] "r" (*input)
        : "r12", "cc"               /* Clobber list forces reload */
    );
    
    /* Use the result in bit-field to combine patterns */
    struct bitfield_struct local_bf;
    local_bf.b = temp & 0xFF;       /* STRICT_LOW_PART possible */
    *output = local_bf.b;
}

/* Test 3: Mixed-type accesses for SUBREG generation */
void test_mixed_access(union mixed_access *ma, volatile int index) {
    /* SUBREG from memory access with different sizes */
    ma->half[0] = (index & 0xFFFF);          /* 16-bit write to 32-bit location */
    
    /* Byte access within word - likely SUBREG of MEM */
    ma->byte[2] = (index >> 8) & 0xFF;
    
    /* Pointer casting for SUBREG */
    uint16_t *ptr = (uint16_t *)&ma->word;
    ptr[1] = ma->byte[0] + ma->byte[1];      /* Another sub-word access */
}

/* Test 4: Complex memory addressing for MEM_P */
void test_mem_addressing(volatile uint32_t *base, volatile int idx) {
    /* Complex addressing mode */
    volatile uint32_t *addr = &base[(idx * 3 + 7) & 0xFF];
    
    /* Memory write with computation */
    *addr = (*addr & 0xFFFF0000) | (idx & 0xFFFF);
    
    /* Another memory access with different size via cast */
    *(volatile uint16_t *)addr = (idx >> 4) & 0xFFFF;  /* SUBREG of MEM */
}

/* Test 5: Loop with data-dependent memory and bit-field ops */
void test_loop_complex(volatile int iterations) {
    union mixed_access ma;
    struct bitfield_struct bf_array[4];
    
    for (volatile int i = 0; i < iterations && i < 4; i++) {
        /* Mix all patterns in loop */
        test_bitfield_ops(&bf_array[i], i + iterations);
        
        /* Memory access with volatile to prevent optimization */
        mem_buffer[i] = bf_array[i].c;
        
        /* Mixed type access */
        ma.word = mem_buffer[i];
        test_mixed_access(&ma, i);
        
        /* Update memory with new value */
        mem_buffer[i + 128] = ma.word;
    }
}

/* Main driver with volatile input to prevent compile-time optimization */
int main(int argc, char *argv[]) {
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int result = 0;
    
    /* Initialize */
    union mixed_access ma = {0};
    struct bitfield_struct bf = {0};
    
    /* Run tests that generate specific RTL patterns */
    test_bitfield_ops(&bf, seed);
    
    int asm_input = seed * 2;
    int asm_output;
    test_asm_clobber(&asm_input, &asm_output);
    
    test_mixed_access(&ma, seed);
    
    /* Initialize memory buffer */
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = i;
    }
    
    test_mem_addressing(mem_buffer, seed);
    test_loop_complex(seed & 3);  /* Small iteration count */
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= mem_buffer[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    checksum ^= bf.a ^ bf.b ^ bf.c ^ bf.d;
    checksum ^= ma.word;
    checksum ^= asm_output;
    
    printf("Result checksum: 0x%08X\n", (unsigned int)checksum);
    return 0;
}
