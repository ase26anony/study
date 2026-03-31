/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_program test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
static volatile int g_volatile_input = 0;

/* Struct with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG */
union mixed_types {
    uint32_t full;
    uint16_t half[2];
    uint8_t bytes[4];
};

/* Global with volatile to force MEM RTL patterns */
volatile uint32_t g_mem_buffer[64];

/* Test 1: Bit-field assignments in loop - should generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent indexing prevents optimization */
        int idx = g_volatile_input & 0xF;
        
        /* Multiple bit-field assignments */
        bf[idx].a = (i & 0xF);
        bf[idx].b = (i >> 4) & 0xFF;
        bf[idx].c = (i >> 8) & 0xFFF;
        bf[idx].d = (i >> 20) & 0xFF;
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobbers(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    struct bitfield_struct bf_local;
    
    /* Inline asm that clobbers hard registers */
    asm volatile (
        "mov %[input], %[temp] \n\t"
        "and $0xF, %[temp] \n\t"
        : [temp] "=r" (bf_local.a)
        : [input] "r" (reg_var)
        : "cc"
    );
    
    /* Follow with bit-field assignment to potentially create STRICT_LOW_PART */
    bf_local.b = reg_var & 0xFF;
    bf_local.c = (reg_var >> 8) & 0xFFF;
    
    /* Use result to prevent dead code elimination */
    g_mem_buffer[0] = bf_local.b;
}

/* Test 3: Mixed-type accesses via pointers to generate SUBREG of MEM */
void test_mixed_type_access(volatile uint8_t *buffer, int size) {
    union mixed_types *ptr = (union mixed_types *)buffer;
    
    for (int i = 0; i < size/4; i++) {
        /* Write full 32-bit */
        ptr[i].full = i * 0x01010101;
        
        /* Access via smaller types - should generate SUBREG */
        ptr[i].half[0] = g_volatile_input & 0xFFFF;
        ptr[i].bytes[2] = (g_volatile_input >> 16) & 0xFF;
        
        /* Complex addressing mode */
        g_mem_buffer[i] = ptr[i].full + buffer[i];
    }
}

/* Test 4: Volatile memory writes with complex addressing */
void test_volatile_mem_ops(int iterations) {
    volatile uint16_t *short_ptr = (volatile uint16_t *)g_mem_buffer;
    
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent index */
        int idx = (i * g_volatile_input) & 0x3F;
        
        /* Multiple volatile writes - each generates MEM RTL */
        g_mem_buffer[idx] = i;
        short_ptr[idx * 2] = i & 0xFFFF;
        
        /* Pointer arithmetic with volatile */
        volatile uint32_t *ptr = &g_mem_buffer[idx] + (i & 3);
        *ptr = *ptr + 1;
    }
}

/* Test 5: Combined operations in loop with data-dependent flow */
void test_combined_ops(struct bitfield_struct *bf_array, 
                       volatile uint8_t *mem, 
                       int count) {
    union mixed_types converter;
    
    for (int i = 0; i < count; i++) {
        /* Data-dependent condition */
        if (g_volatile_input & (1 << (i & 7))) {
            /* Bit-field operation */
            bf_array[i].a = mem[i] & 0xF;
            bf_array[i].b = (mem[i] >> 4) & 0xF;
            
            /* Type-punning write to generate SUBREG */
            converter.full = bf_array[i].c;
            converter.bytes[1] = bf_array[i].a;
            mem[i * 4] = converter.bytes[0];
        } else {
            /* Volatile memory write */
            g_mem_buffer[i & 0x3F] = i * 0x1001;
            
            /* Cast to different type for SUBREG */
            *(volatile uint16_t *)&mem[i * 2] = i & 0xFFFF;
        }
    }
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    
    /* Use command line argument for runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Allocate aligned memory for bit-field structs */
    struct bitfield_struct *bf_array = 
        (struct bitfield_struct *)aligned_alloc(16, 64 * sizeof(struct bitfield_struct));
    
    /* Allocate memory buffer for mixed-type access */
    volatile uint8_t *mem_buffer = 
        (volatile uint8_t *)malloc(256 * sizeof(uint8_t));
    
    if (!bf_array || !mem_buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 64; i++) {
        bf_array[i].a = i & 0xF;
        bf_array[i].b = (i >> 4) & 0xFF;
        bf_array[i].c = i * 3;
        bf_array[i].d = i * 5;
        g_mem_buffer[i] = i * 0x10001;
    }
    
    printf("Starting resource tracking tests...\n");
    
    /* Run all tests to trigger different RTL patterns */
    test_bitfield_ops(bf_array, iterations);
    test_asm_clobbers();
    test_mixed_type_access(mem_buffer, 256);
    test_volatile_mem_ops(iterations);
    test_combined_ops(bf_array, mem_buffer, 64);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += bf_array[i].a + bf_array[i].b + bf_array[i].c + bf_array[i].d;
        checksum += g_mem_buffer[i];
        if (i < 256) checksum += mem_buffer[i];
    }
    
    printf("Checksum: %u\n", checksum);
    printf("Tests completed.\n");
    
    /* Cleanup */
    free(bf_array);
    free((void *)mem_buffer);
    
    return 0;
}
