/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_program test.c */
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
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* Global volatile memory buffer */
volatile uint32_t g_mem_buffer[256];

/* Test 1: Bit-field assignments with register variable */
__attribute__((noinline))
void test_bitfield_register(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    struct bitfield_struct bf;
    
    /* Force register variable usage with bit-field assignment */
    bf.a = (reg_var >> 0) & 0xF;   /* May generate STRICT_LOW_PART */
    bf.b = (reg_var >> 4) & 0xFF;  /* May generate ZERO_EXTRACT */
    bf.c = (reg_var >> 12) & 0xFFF;
    
    /* Data-dependent assignment to prevent optimization */
    if (g_vol_input > 0) {
        bf.d = reg_var & 0xFF;
    } else {
        bf.d = (reg_var >> 16) & 0xFF;
    }
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+r" (reg_var) : "r" (bf.a));
}

/* Test 2: Memory operations with SUBREG and MEM */
__attribute__((noinline))
void test_mem_subreg(void) {
    union mixed_access *ptr = (union mixed_access *)&g_mem_buffer[0];
    volatile int idx = g_vol_input & 0xF;
    
    /* Generate SUBREG of MEM through type-punning */
    ptr[idx].half[1] = ptr[idx].byte[0] + ptr[idx].byte[3];
    
    /* Complex addressing mode */
    uint32_t *mem_ptr = (uint32_t *)&g_mem_buffer[idx * 2];
    *mem_ptr = (*mem_ptr << 3) | (*mem_ptr >> 29);
    
    /* Volatile memory write with different size access */
    volatile uint16_t *vol_ptr = (volatile uint16_t *)mem_ptr;
    vol_ptr[1] = vol_ptr[0] ^ 0x55AA;
}

/* Test 3: Inline assembly with clobbers to stress reload */
__attribute__((noinline))
void test_asm_clobber(void) {
    uint32_t var1 = 0xDEADBEEF;
    uint32_t var2 = 0xCAFEBABE;
    struct bitfield_struct bf;
    
    /* Inline asm that ties C variables to hard registers */
    asm volatile (
        "mov %[v1], %%r12\n\t"
        "mov %[v2], %%r13\n\t"
        "and $0xF, %%r12\n\t"
        "mov %%r12, %[bf_a]\n\t"
        : [bf_a] "=rm" (bf.a)
        : [v1] "r" (var1), [v2] "r" (var2)
        : "r12", "r13", "cc"
    );
    
    /* Use the modified bit-field */
    bf.b = bf.a * 2;
    bf.c = bf.b | (var1 & 0xFFF);
}

/* Test 4: Loop with mixed memory accesses */
__attribute__((noinline))
void test_loop_mixed_access(void) {
    volatile int limit = (g_vol_input & 0x3F) + 10;
    union mixed_access local_buf[16];
    
    for (int i = 0; i < limit && i < 16; i++) {
        /* Generate various RTL patterns in loop */
        local_buf[i].word = i * 0x11111111;
        
        /* SUBREG access */
        local_buf[i].half[0] = local_buf[i].byte[1] + local_buf[i].byte[2];
        
        /* Bit-field in loop */
        struct bitfield_struct *bf_ptr = (struct bitfield_struct *)&local_buf[i];
        bf_ptr->a = (local_buf[i].byte[0] >> 2) & 0xF;
        bf_ptr->b = bf_ptr->a * 3;
        
        /* Volatile memory access */
        g_mem_buffer[i] = local_buf[i].word;
    }
}

/* Test 5: Complex addressing with pointer arithmetic */
__attribute__((noinline))
void test_complex_addressing(void) {
    volatile uint8_t *base_ptr = (volatile uint8_t *)g_mem_buffer;
    int offset = g_vol_input & 0xFF;
    
    /* MEM with complex address */
    volatile uint32_t *mem_loc = (volatile uint32_t *)(base_ptr + offset * 4);
    *mem_loc = (*mem_loc & 0x00FFFFFF) | ((offset << 24) & 0xFF000000);
    
    /* SUBREG of MEM with offset */
    volatile uint16_t *subreg_ptr = (volatile uint16_t *)((uint8_t *)mem_loc + 1);
    *subreg_ptr = (*subreg_ptr + offset) & 0x7FFF;
    
    /* Pointer cast to different type size */
    struct bitfield_struct *bf_mem = (struct bitfield_struct *)mem_loc;
    bf_mem->c = (offset * 7) & 0xFFF;
}

int main(int argc, char *argv[]) {
    /* Initialize with runtime value */
    if (argc > 1) {
        g_vol_input = atoi(argv[1]);
    } else {
        g_vol_input = 42;
    }
    
    /* Initialize memory buffer */
    for (int i = 0; i < 256; i++) {
        g_mem_buffer[i] = i * 0x01010101;
    }
    
    printf("Starting tests with input: %d\n", g_vol_input);
    
    /* Execute all test patterns */
    test_bitfield_register();
    printf("Test 1 completed\n");
    
    test_mem_subreg();
    printf("Test 2 completed\n");
    
    test_asm_clobber();
    printf("Test 3 completed\n");
    
    test_loop_mixed_access();
    printf("Test 4 completed\n");
    
    test_complex_addressing();
    printf("Test 5 completed\n");
    
    /* Compute checksum to verify execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum ^= g_mem_buffer[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Final checksum: 0x%08X\n", checksum);
    return 0;
}
