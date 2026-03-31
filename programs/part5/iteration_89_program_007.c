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
union mixed_types {
    uint32_t full;
    uint16_t half[2];
    uint8_t bytes[4];
};

/* Global volatile memory buffer for MEM operations */
volatile uint32_t g_mem_buffer[256];

/* Test 1: Bit-field assignments with volatile control */
void test_bitfield_ops(struct bitfield_struct *bf, int idx) {
    /* Data-dependent indexing prevents optimization */
    volatile int i = idx;
    
    /* These assignments may generate ZERO_EXTRACT/STRICT_LOW_PART */
    bf[i].a = (i & 0xF);
    bf[i].b = (i * 3) & 0xFF;
    bf[i].c = (i * 5) & 0xFFF;
    bf[i].d = (i * 7) & 0xFF;
    
    /* Nested bit-field access in loop */
    for (int j = 0; j < 2; j++) {
        bf[j].b = bf[i].a + j;  /* Potential STRICT_LOW_PART */
    }
}

/* Test 2: Inline assembly with register variables and clobbers */
void test_asm_register_ops(void) {
    /* Register variable tied to specific register */
    register uint32_t reg_var asm("r12") = 0x12345678;
    register uint32_t reg_var2 asm("r13") = 0;
    
    /* Inline assembly with clobbers to stress reload pass */
    asm volatile (
        "mov %[src], %[dest]\n\t"
        "ror $8, %[dest]\n\t"
        : [dest] "=r" (reg_var2)
        : [src] "r" (reg_var)
        : "cc"  /* Clobber flags to force resource tracking */
    );
    
    /* Use the result in bit-field context */
    struct bitfield_struct bf_local;
    bf_local.b = reg_var2 & 0xFF;  /* Potential STRICT_LOW_PART */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
}

/* Test 3: Mixed-type accesses via unions and pointers */
void test_mixed_type_access(union mixed_types *u, volatile uint8_t *mem) {
    /* Write full 32-bit value */
    u->full = 0xDEADBEEF;
    
    /* Access via smaller types - may generate SUBREG */
    u->half[0] = 0x1234;  /* SUBREG of MEM */
    u->bytes[1] = 0xAB;   /* Another SUBREG */
    
    /* Volatile memory access with type punning */
    *(volatile uint16_t *)(mem + 2) = 0x5678;  /* MEM with possible SUBREG */
    
    /* Complex addressing mode */
    uint32_t offset = g_vol_input & 0x3;
    *(volatile uint32_t *)(mem + offset * 4) = u->full;  /* MEM_P(x) true */
}

/* Test 4: Memory operations with complex addressing */
void test_memory_ops(int iterations) {
    volatile uint32_t *ptr = g_mem_buffer;
    
    /* Loop with data-dependent index prevents optimization */
    for (int i = 0; i < iterations; i++) {
        /* Complex addressing: base + scaled index */
        ptr[i * 2] = ptr[i] + g_vol_input;  /* MEM operations */
        
        /* Additional offset calculation */
        int idx = (i * 3 + g_vol_input) & 0xFF;
        ptr[idx] = ptr[i] ^ 0xFFFF;  /* Another MEM */
    }
    
    /* Pointer arithmetic with different types */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)ptr;
    byte_ptr[g_vol_input & 0x1F] = 0xCC;  /* MEM of byte type */
}

/* Test 5: Combined operations in data-dependent loop */
void test_combined_ops(struct bitfield_struct *bf_arr, 
                       union mixed_types *unions,
                       int count) {
    volatile int seed = g_vol_input;
    
    for (int i = 0; i < count; i++) {
        /* Bit-field assignment (ZERO_EXTRACT/STRICT_LOW_PART) */
        bf_arr[i].c = (seed + i) & 0xFFF;
        
        /* Mixed-type access (SUBREG) */
        unions[i].half[i & 1] = bf_arr[i].c & 0xFFFF;
        
        /* Memory operation (MEM) */
        g_mem_buffer[i] = unions[i].full;
        
        /* Update seed for data dependence */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for runtime control */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    if (iterations > 100) iterations = 100;  /* Bound for safety */
    
    /* Initialize test data */
    struct bitfield_struct bf_array[20] = {{0}};
    union mixed_types union_array[20];
    volatile uint8_t memory_block[256] = {0};
    
    printf("Starting resource tracking tests...\n");
    printf("Using iteration count: %d\n", iterations);
    
    /* Update volatile input */
    g_vol_input = iterations;
    
    /* Run tests that should trigger the uncovered RTL patterns */
    test_bitfield_ops(bf_array, iterations % 20);
    test_asm_register_ops();
    test_mixed_type_access(&union_array[0], memory_block);
    test_memory_ops(iterations);
    test_combined_ops(bf_array, union_array, iterations % 20);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 20; i++) {
        checksum ^= bf_array[i].a + bf_array[i].b + 
                   bf_array[i].c + bf_array[i].d;
        checksum ^= union_array[i].full;
    }
    
    for (int i = 0; i < 256 && i < iterations * 2; i++) {
        checksum ^= g_mem_buffer[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Tests completed.\n");
    
    return 0;
}
