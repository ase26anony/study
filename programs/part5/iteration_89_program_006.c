/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_resource test_resource.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_vol_input = 1;

/* Bit-field structures to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
};

struct mixed_bitfields {
    unsigned short f1 : 4;
    unsigned short f2 : 7;
    unsigned short f3 : 5;
};

/* Packed struct for SUBREG generation */
struct __attribute__((packed)) packed_data {
    char x;
    int y;
    short z;
};

/* Union for type-punning */
union type_pun {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* Global variables for memory operations */
volatile uint32_t g_mem_buffer[64];
struct bitfield_struct g_bf_array[16];

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(void) {
    struct bitfield_struct bf;
    struct mixed_bitfields mbf;
    
    /* Multiple bit-field writes with volatile dependency */
    int idx = g_vol_input & 0xF;
    
    bf.a = idx & 0x7;
    bf.b = (idx * 3) & 0x1F;
    bf.c = (idx * 5) & 0xFF;
    bf.d = (idx * 7) & 0xFFFF;
    
    /* Chain of assignments to same struct */
    g_bf_array[idx] = bf;
    
    /* Mixed bit-field operations */
    mbf.f1 = bf.a;
    mbf.f2 = bf.b;
    mbf.f3 = bf.c & 0x1F;
    
    /* Store to volatile to force MEM generation */
    *(volatile struct mixed_bitfields *)&g_mem_buffer[0] = mbf;
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(void) {
    register uint32_t reg_var asm("r12") = g_vol_input;
    uint32_t temp;
    
    /* Inline asm that uses specific register with clobbers */
    asm volatile (
        "mov %[input], %[temp]\n\t"
        "ror $8, %[temp]\n\t"
        : [temp] "=r" (temp)
        : [input] "r" (reg_var)
        : "cc"
    );
    
    /* Use result in bit-field */
    struct bitfield_struct local_bf;
    local_bf.a = temp & 0x7;
    local_bf.b = (temp >> 3) & 0x1F;
    
    /* Force memory store */
    g_bf_array[temp & 0xF] = local_bf;
}

/* Test 3: SUBREG and MEM generation through type-punning and volatile */
void test_subreg_mem(void) {
    union type_pun pun;
    volatile uint8_t *byte_ptr;
    uint16_t *half_ptr;
    
    /* Initialize */
    pun.word = g_vol_input * 0x12345678;
    
    /* SUBREG generation: access sub-parts through different type pointers */
    byte_ptr = (volatile uint8_t *)&g_mem_buffer[0];
    half_ptr = (uint16_t *)&g_mem_buffer[1];
    
    /* Mixed-size accesses to same memory region */
    for (int i = 0; i < 4; i++) {
        byte_ptr[i] = pun.byte[i];
    }
    
    /* This should generate SUBREG of MEM */
    half_ptr[0] = pun.half[0];
    half_ptr[1] = pun.half[1];
    
    /* Packed struct access - likely SUBREG */
    struct packed_data *packed = (struct packed_data *)&g_mem_buffer[8];
    packed->x = pun.byte[0];
    packed->y = pun.word;
    packed->z = pun.half[1];
}

/* Test 4: Complex addressing modes for MEM_P */
void test_complex_mem(void) {
    volatile uint32_t *ptr = &g_mem_buffer[0];
    int idx = g_vol_input;
    
    /* Memory operations with complex addressing */
    for (int i = 0; i < 8; i++) {
        ptr[idx + i * 2] = ptr[idx + i * 2] ^ 0xA5A5A5A5;
        ptr[idx + i * 2 + 1] = ptr[idx + i * 2 + 1] + i;
    }
    
    /* Pointer arithmetic with different types */
    volatile uint16_t *short_ptr = (volatile uint16_t *)ptr;
    for (int i = 0; i < 16; i++) {
        short_ptr[idx + i] = short_ptr[idx + i] * 3;
    }
}

/* Test 5: Combined operations in loop with data dependency */
void test_combined_loop(void) {
    uint32_t checksum = 0;
    int limit = (g_vol_input & 0x7) + 4;
    
    for (int i = 0; i < limit; i++) {
        /* Bit-field operation */
        struct bitfield_struct bf;
        bf.a = (i * 2) & 0x7;
        bf.b = (i * 3) & 0x1F;
        bf.c = (i * 5) & 0xFF;
        bf.d = (i * 7) & 0xFFFF;
        
        /* Store to memory */
        g_bf_array[i] = bf;
        
        /* Memory operation with complex addressing */
        g_mem_buffer[i * 3] = g_mem_buffer[i * 3] ^ bf.d;
        g_mem_buffer[i * 3 + 1] = g_mem_buffer[i * 3 + 1] + bf.c;
        
        /* Type-punning through union */
        union type_pun pun;
        pun.word = g_mem_buffer[i * 3];
        g_mem_buffer[i * 3 + 2] = pun.half[0] | (pun.half[1] << 16);
        
        checksum += bf.a + bf.b + bf.c + bf.d;
    }
    
    /* Final volatile store */
    *(volatile uint32_t *)&g_mem_buffer[63] = checksum;
}

int main(int argc, char *argv[]) {
    /* Initialize with some data */
    for (int i = 0; i < 64; i++) {
        g_mem_buffer[i] = i * 0x01010101;
    }
    
    for (int i = 0; i < 16; i++) {
        g_bf_array[i].a = i & 0x7;
        g_bf_array[i].b = (i * 2) & 0x1F;
        g_bf_array[i].c = (i * 3) & 0xFF;
        g_bf_array[i].d = (i * 5) & 0xFFFF;
    }
    
    /* Update volatile input from command line if provided */
    if (argc > 1) {
        g_vol_input = atoi(argv[1]);
    }
    
    /* Run all tests */
    test_bitfield_ops();
    test_asm_clobber();
    test_subreg_mem();
    test_complex_mem();
    test_combined_loop();
    
    /* Calculate and print checksum to ensure execution */
    uint32_t final_checksum = 0;
    for (int i = 0; i < 64; i++) {
        final_checksum ^= g_mem_buffer[i];
    }
    
    for (int i = 0; i < 16; i++) {
        final_checksum += g_bf_array[i].a;
        final_checksum += g_bf_array[i].b;
        final_checksum += g_bf_array[i].c;
        final_checksum += g_bf_array[i].d;
    }
    
    printf("Final checksum: 0x%08X\n", final_checksum);
    return 0;
}
