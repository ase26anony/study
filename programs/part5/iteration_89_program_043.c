/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_resource test_resource.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_vol_input = 0;

/* Struct with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG */
union mixed_access {
    uint32_t full;
    uint16_t half[2];
    uint8_t bytes[4];
};

/* Global volatile memory for MEM RTL generation */
volatile uint32_t g_mem_buffer[256];

/* Test 1: Bit-field assignments with register variable */
void test_bitfield_reg(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    struct bitfield_struct bf;
    
    /* Force bit-field operations on register-tied variable */
    bf.a = (reg_var >> 0) & 0xF;
    bf.b = (reg_var >> 4) & 0xFF;
    bf.c = (reg_var >> 12) & 0xFFF;
    bf.d = (reg_var >> 24) & 0xFF;
    
    /* Use result to prevent dead code elimination */
    g_mem_buffer[0] = (bf.a << 24) | (bf.b << 16) | (bf.c << 4) | bf.d;
}

/* Test 2: Inline assembly with clobbers to stress reload */
void test_asm_clobber(void) {
    uint32_t val1 = 0xDEADBEEF;
    uint32_t val2 = 0xCAFEBABE;
    struct bitfield_struct bf;
    
    /* Inline asm that clobbers registers, forcing reload to manage resources */
    asm volatile (
        "mov %[in1], %%r12\n\t"
        "mov %[in2], %%r13\n\t"
        "and $0xF, %%r12\n\t"
        "mov %%r12, %[out]\n\t"
        : [out] "=r" (bf.a)
        : [in1] "r" (val1), [in2] "r" (val2)
        : "r12", "r13", "cc"
    );
    
    /* Additional bit-field ops after asm */
    bf.b = (val1 >> 8) & 0xFF;
    bf.c = (val2 >> 4) & 0xFFF;
    
    g_mem_buffer[1] = (bf.a << 24) | (bf.b << 16) | bf.c;
}

/* Test 3: SUBREG generation via type-punning and volatile memory */
void test_subreg_mem(void) {
    union mixed_access *ptr = (union mixed_access *)&g_mem_buffer[16];
    volatile uint16_t *vol_ptr = (volatile uint16_t *)&g_mem_buffer[32];
    
    /* Write full word (MEM) */
    ptr->full = 0x87654321;
    
    /* Write half-word (SUBREG of MEM) */
    ptr->half[1] = 0xABCD;
    
    /* Volatile sub-word access (likely SUBREG) */
    vol_ptr[g_vol_input & 1] = 0x1234;
    
    /* Cast between different pointer types to force SUBREG */
    *(volatile uint8_t *)(&g_mem_buffer[48]) = 0x42;
}

/* Test 4: Complex addressing modes and loops for MEM_P */
void test_complex_mem(void) {
    volatile uint32_t *base = &g_mem_buffer[64];
    int i = g_vol_input;
    
    /* Loop with data-dependent index prevents optimization */
    for (int j = 0; j < 8; j++) {
        /* Complex addressing: base + (i * j) / 2 */
        int idx = (i * j) >> 1;
        base[idx] = base[idx] ^ (0xFF << (j * 4));
        
        /* Nested bit-field in loop */
        struct bitfield_struct temp;
        temp.a = (base[idx] >> 0) & 0xF;
        temp.b = (base[idx] >> 4) & 0xFF;
        base[idx + 1] = (temp.a << 8) | temp.b;
    }
}

/* Test 5: Mixed operations to hit multiple paths */
void test_mixed_ops(void) {
    struct bitfield_struct bf_arr[4];
    volatile uint32_t *mem = &g_mem_buffer[128];
    
    /* Initialize with volatile input */
    uint32_t seed = g_vol_input;
    
    for (int i = 0; i < 4; i++) {
        /* Bit-field assignment (ZERO_EXTRACT/STRICT_LOW_PART) */
        bf_arr[i].a = (seed >> (i * 2)) & 0xF;
        bf_arr[i].b = (seed >> (i * 4)) & 0xFF;
        
        /* Memory write (MEM) */
        mem[i] = bf_arr[i].a | (bf_arr[i].b << 8);
        
        /* SUBREG memory access */
        *(volatile uint16_t *)((uint8_t *)mem + i * 8 + 4) = bf_arr[i].b;
    }
}

/* Main driver with checksum verification */
int main(int argc, char **argv) {
    /* Use command-line argument for variability */
    if (argc > 1) {
        g_vol_input = atoi(argv[1]);
    } else {
        g_vol_input = 0x1234;
    }
    
    /* Initialize memory */
    for (int i = 0; i < 256; i++) {
        g_mem_buffer[i] = i * 0x01010101;
    }
    
    /* Run all tests */
    test_bitfield_reg();
    test_asm_clobber();
    test_subreg_mem();
    test_complex_mem();
    test_mixed_ops();
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= g_mem_buffer[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    return 0;
}
