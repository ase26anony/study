#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Test structures with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG patterns */
union type_pun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
    uint8_t bytes[4];
};

/* Volatile memory buffer for MEM RTL generation */
volatile uint32_t mem_buffer[256];

/* Function 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, volatile int idx) {
    /* Data-dependent indexing prevents optimization */
    int i = idx & 0xFF;
    
    /* Multiple bit-field assignments */
    bf[i].a = (i & 0x0F);
    bf[i].b = (i * 3) & 0xFF;
    bf[i].c = (i * 5) & 0xFFF;
    bf[i].d = (i * 7) & 0xFF;
    
    /* Nested bit-field access in loop */
    for (int j = 0; j < 4; j++) {
        bf[(i + j) & 0xFF].b = bf[i].a + j;
    }
}

/* Function 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobbers(volatile int *input) {
    int val = *input;
    int result;
    
    /* Inline assembly tying C variable to specific register */
    asm volatile (
        "mov %[src], %%r12\n\t"
        "add $0x7F, %%r12\n\t"
        "mov %%r12, %[dst]\n\t"
        : [dst] "=r" (result)
        : [src] "r" (val)
        : "r12", "cc"
    );
    
    /* Use result in bit-field context */
    struct bitfield_struct bf_local;
    bf_local.a = result & 0x0F;
    bf_local.b = (result >> 4) & 0xFF;
    
    mem_buffer[0] = bf_local.b;
}

/* Function 3: Type-punning and sub-register access for SUBREG RTL */
void test_subreg_access(union type_pun *pun, volatile int idx) {
    /* Write full 32-bit value */
    pun[idx].full = 0xDEADBEEF;
    
    /* Access sub-registers (should generate SUBREG RTL) */
    pun[idx].parts.low = 0xCAFE;
    pun[idx].parts.high = 0xBABE;
    
    /* Byte access through pointer cast (more SUBREG patterns) */
    uint8_t *byte_ptr = (uint8_t *)&pun[idx].full;
    for (int i = 0; i < 4; i++) {
        byte_ptr[i] = (idx + i) & 0xFF;
    }
}

/* Function 4: Complex memory addressing for MEM RTL */
void test_mem_addressing(volatile uint32_t *mem, volatile int *offsets) {
    /* Complex addressing mode */
    for (int i = 0; i < 8; i++) {
        int idx = offsets[i] & 0xFF;
        
        /* Memory write with index (MEM RTL) */
        mem[idx * 2] = idx * 0x1001;
        
        /* Memory write with scaled index */
        mem[idx + 64] = mem[idx * 2] + 0x100;
    }
}

/* Function 5: Mixed operations to combine patterns */
void test_mixed_ops(struct bitfield_struct *bf, union type_pun *pun, 
                    volatile int idx) {
    /* Bit-field to memory */
    bf[idx].c = pun[idx].parts.low & 0xFFF;
    
    /* Memory to bit-field with intermediate register variable */
    register uint32_t temp asm("r10") = mem_buffer[idx];
    bf[idx].b = temp & 0xFF;
    
    /* Inline assembly with memory operand */
    asm volatile (
        "movl %[mem], %%eax\n\t"
        "rorl $8, %%eax\n\t"
        "movl %%eax, %[mem]\n\t"
        : [mem] "+m" (mem_buffer[idx])
        :
        : "eax", "cc"
    );
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for runtime variability */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize test data */
    struct bitfield_struct bf_array[256];
    union type_pun pun_array[256];
    volatile int offsets[8] = {1, 3, 7, 15, 31, 63, 127, 255};
    
    /* Initialize memory */
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = i * 0x101;
        bf_array[i].full = 0;
        pun_array[i].full = i;
    }
    
    /* Run test sequences */
    test_bitfield_ops(bf_array, seed);
    test_asm_clobbers(&seed);
    test_subreg_access(pun_array, seed & 0xFF);
    test_mem_addressing(mem_buffer, offsets);
    test_mixed_ops(bf_array, pun_array, seed & 0xFF);
    
    /* Calculate checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= bf_array[i].full;
        checksum ^= pun_array[i].full;
        checksum ^= mem_buffer[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    return 0;
}
