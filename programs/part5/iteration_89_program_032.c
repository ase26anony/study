/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_program test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_volatile_input = 0;

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
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

/* Global variables to force memory operations */
volatile uint32_t g_mem_buffer[64];
struct bitfield_struct g_bitfield_array[16];

/* Test 1: Bit-field assignments with volatile control flow */
void test_bitfield_operations(int index) {
    struct bitfield_struct *bf = &g_bitfield_array[index];
    
    /* These assignments may generate ZERO_EXTRACT or STRICT_LOW_PART */
    bf->a = (g_volatile_input & 0xF);
    bf->b = (g_volatile_input >> 4) & 0xFF;
    bf->c = (g_volatile_input >> 12) & 0xFFF;
    bf->d = (g_volatile_input >> 24) & 0xFF;
    
    /* Complex expression to prevent simplification */
    if (index & 1) {
        bf->a = bf->b ^ bf->c;
    } else {
        bf->b = bf->a | bf->d;
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_register_clobber(void) {
    register uint32_t reg_var asm("r12") = g_volatile_input;
    struct bitfield_struct local_bf;
    
    /* Inline assembly that clobbers hard registers */
    asm volatile (
        "mov %[input], %[temp]\n\t"
        "and $0xF, %[temp]\n\t"
        : [temp] "=r" (local_bf.a)
        : [input] "r" (reg_var)
        : "cc"
    );
    
    /* Additional bit-field assignment after assembly */
    local_bf.b = (reg_var >> 4) & 0xFF;
    
    /* Use the result to prevent dead code elimination */
    g_mem_buffer[0] = local_bf.a + local_bf.b;
}

/* Test 3: Memory accesses with type-punning for SUBREG generation */
void test_memory_subreg(int offset) {
    union mixed_types *mem = (union mixed_types *)&g_mem_buffer[offset];
    
    /* Write whole word (MEM) */
    mem->word = g_volatile_input;
    
    /* Write half-word (may generate SUBREG of MEM) */
    mem->half[1] = (g_volatile_input >> 16) & 0xFFFF;
    
    /* Write byte (another SUBREG of MEM) */
    mem->byte[0] = (g_volatile_input >> 8) & 0xFF;
    
    /* Complex addressing mode */
    volatile uint16_t *ptr = (volatile uint16_t *)&g_mem_buffer[offset + 2];
    *ptr = mem->half[0] ^ mem->byte[3];
}

/* Test 4: Loop with mixed operations to generate various RTL patterns */
void test_loop_mixed_operations(int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Alternate between different operations */
        if (i & 1) {
            /* Bit-field operation */
            struct bitfield_struct *bf = &g_bitfield_array[i % 16];
            bf->c = (bf->a + bf->b) & 0xFFF;
        } else {
            /* Memory operation with type punning */
            union mixed_types *mem = (union mixed_types *)&g_mem_buffer[i % 32];
            mem->half[i % 2] = g_volatile_input + i;
        }
        
        /* Volatile memory write (pure MEM) */
        g_mem_buffer[63] = i;
    }
}

/* Test 5: Complex expression combining multiple patterns */
void test_complex_expression(void) {
    volatile uint32_t *mem_ptr = &g_mem_buffer[32];
    struct bitfield_struct *bf_ptr = &g_bitfield_array[8];
    
    /* Chain of operations that may generate SUBREG -> MEM */
    uint32_t temp = *mem_ptr;
    temp = (temp >> 4) | (temp << 28);  /* Rotate */
    
    /* Store rotated value into bit-fields */
    bf_ptr->a = temp & 0xF;
    bf_ptr->b = (temp >> 4) & 0xFF;
    
    /* Store back to memory through cast (potential SUBREG) */
    *(volatile uint16_t *)mem_ptr = bf_ptr->a + bf_ptr->b;
}

int main(int argc, char *argv[]) {
    int iterations = 10;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 64; i++) {
        g_mem_buffer[i] = i * 0x01010101;
    }
    
    for (int i = 0; i < 16; i++) {
        g_bitfield_array[i].a = i;
        g_bitfield_array[i].b = i * 2;
        g_bitfield_array[i].c = i * 3;
        g_bitfield_array[i].d = i * 4;
    }
    
    /* Run all tests */
    test_bitfield_operations(g_volatile_input % 16);
    test_register_clobber();
    test_memory_subreg(g_volatile_input % 32);
    test_loop_mixed_operations(iterations);
    test_complex_expression();
    
    /* Compute checksum to verify execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum ^= g_mem_buffer[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum ^= *(uint32_t *)&g_bitfield_array[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    return 0;
}
