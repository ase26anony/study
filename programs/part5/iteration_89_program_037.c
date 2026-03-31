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

/* Volatile memory buffer for MEM RTL generation */
static volatile uint32_t g_mem_buffer[64];

/* Test 1: Bit-field assignments with volatile control flow */
void test_bitfield_assignments(struct bitfield_struct *bf) {
    int i;
    /* Data-dependent loop prevents optimization */
    for (i = 0; i < g_input % 8 + 1; i++) {
        /* These assignments may generate ZERO_EXTRACT or STRICT_LOW_PART */
        bf->a = (i * 3) & 0xF;
        bf->b = (i * 5) & 0xFF;
        bf->c = (i * 7) & 0xFFF;
        bf->d = (i * 11) & 0xFF;
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    struct bitfield_struct bf_local;
    
    /* Inline asm that clobbers hard register */
    asm volatile (
        "mov %[reg], %[val]\n\t"
        : [reg] "+r" (reg_var)
        : [val] "r" (g_input)
        : "cc"
    );
    
    /* Bit-field assignment after clobber - may generate STRICT_LOW_PART */
    bf_local.a = reg_var & 0xF;
    bf_local.b = (reg_var >> 4) & 0xFF;
    
    /* Use result to prevent dead code elimination */
    g_mem_buffer[0] = bf_local.b;
}

/* Test 3: Mixed-type accesses via union for SUBREG generation */
void test_mixed_type_access(union mixed_types *u) {
    volatile uint8_t *byte_ptr = (volatile uint8_t *)&u->word;
    
    /* Access word as smaller types - may generate SUBREG RTL */
    u->half[0] = g_input & 0xFFFF;
    u->byte[2] = (g_input >> 16) & 0xFF;
    
    /* Memory access with type punning */
    *(volatile uint16_t *)(&u->byte[1]) = 0xABCD;
}

/* Test 4: Complex memory addressing modes for MEM RTL */
void test_complex_mem_addressing(void) {
    int i;
    volatile uint32_t *ptr;
    
    /* Loop with data-dependent index */
    for (i = g_input % 16; i < 32; i += (g_input % 3) + 1) {
        /* Complex addressing: base + scaled index */
        ptr = &g_mem_buffer[i * 2];
        
        /* Memory write - will generate MEM RTL */
        *ptr = i * 0x1001;
        
        /* Additional MEM with offset */
        g_mem_buffer[i + 16] = *ptr + 0x100;
    }
}

/* Test 5: Pointer casting for SUBREG of MEM */
void test_pointer_casting(void) {
    volatile uint32_t *word_ptr = &g_mem_buffer[32];
    volatile uint16_t *half_ptr;
    volatile uint8_t *byte_ptr;
    
    /* Write to different parts of the same memory location */
    *word_ptr = 0xDEADBEEF;
    
    /* Cast to smaller type - may generate SUBREG of MEM */
    half_ptr = (volatile uint16_t *)word_ptr;
    half_ptr[1] = 0xCAFE;  /* Modify upper half */
    
    /* Byte access */
    byte_ptr = (volatile uint8_t *)word_ptr;
    byte_ptr[3] = 0x42;
}

int main(int argc, char *argv[]) {
    struct bitfield_struct bf = {0};
    union mixed_types u = {0};
    int i, sum = 0;
    
    /* Initialize with command-line argument for runtime variability */
    if (argc > 1) {
        g_input = atoi(argv[1]);
    } else {
        g_input = 42;  /* Default test value */
    }
    
    /* Initialize memory buffer */
    for (i = 0; i < 64; i++) {
        g_mem_buffer[i] = i;
    }
    
    /* Run all tests */
    test_bitfield_assignments(&bf);
    test_asm_clobber();
    test_mixed_type_access(&u);
    test_complex_mem_addressing();
    test_pointer_casting();
    
    /* Compute checksum to ensure all operations executed */
    for (i = 0; i < 64; i++) {
        sum += g_mem_buffer[i];
    }
    
    sum += bf.a + bf.b + bf.c + bf.d;
    sum += u.word;
    
    printf("Checksum: %d\n", sum);
    printf("Bitfield: a=%u, b=%u, c=%u, d=%u\n", bf.a, bf.b, bf.c, bf.d);
    printf("Union word: 0x%08X\n", u.word);
    
    return 0;
}
