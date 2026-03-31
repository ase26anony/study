/* test_resource_cc.c
 * 
 * This program is designed to trigger the uncovered lines 282-290 in resource.cc
 * by generating specific RTL patterns during GCC compilation.
 * 
 * Target RTL patterns:
 * - ZERO_EXTRACT / STRICT_LOW_PART (via bit-field operations)
 * - SUBREG of MEM (via subword memory accesses)
 * - MEM with complex addressing (via volatile pointers)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force runtime values to prevent constant propagation */
static volatile int g_volatile_seed = 42;

/* ==================== BIT-FIELD STRUCTURES ==================== */

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

/* ==================== TEST FUNCTIONS ==================== */

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent assignments prevent optimization */
        bf->a = (g_volatile_seed + i) & 0xF;
        bf->b = (g_volatile_seed * i) & 0xFF;
        bf->c = (g_volatile_seed + i * 3) & 0xFFF;
        bf->d = (g_volatile_seed - i) & 0xFF;
        
        /* Force memory barrier */
        asm volatile("" ::: "memory");
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(void) {
    register uint32_t reg_var asm("r12") = g_volatile_seed;
    struct bitfield_struct local_bf;
    
    /* Inline asm that clobbers hard register */
    asm volatile (
        "mov %[val], %[reg]\n\t"
        "add $1, %[reg]"
        : [reg] "+r" (reg_var)
        : [val] "r" (g_volatile_seed)
        : "cc"
    );
    
    /* Use the register variable with bit-field */
    local_bf.a = reg_var & 0xF;
    local_bf.b = (reg_var >> 4) & 0xFF;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r" (reg_var));
}

/* Test 3: Subword memory accesses to generate SUBREG of MEM */
void test_subreg_mem(volatile uint8_t *mem, int size) {
    union mixed_access *u = (union mixed_access *)mem;
    
    for (int i = 0; i < size - 4; i += 4) {
        /* Access different-sized elements in union */
        u->byte[0] = g_volatile_seed + i;
        u->half[1] = g_volatile_seed * i;
        u->word = u->word ^ 0x12345678;
        
        /* Cast to different pointer types for SUBREG generation */
        *(volatile uint16_t *)(mem + i + 1) = (uint16_t)(g_volatile_seed + i * 2);
        *(volatile uint8_t *)(mem + i + 3) = (uint8_t)(g_volatile_seed - i);
    }
}

/* Test 4: Complex memory addressing for MEM_P(x) path */
void test_complex_mem_addressing(volatile int *arr, int *indices, int count) {
    for (int i = 0; i < count; i++) {
        /* Complex addressing with multiple components */
        int idx = indices[i] + g_volatile_seed;
        arr[idx * 2 + 1] = arr[idx * 2] * 3 + i;
        
        /* Additional memory operation with offset */
        volatile short *short_ptr = (volatile short *)&arr[idx];
        *short_ptr = (short)(g_volatile_seed + i);
    }
}

/* Test 5: Mixed operations in loop with data-dependent control flow */
void test_mixed_operations(struct bitfield_struct *bf_array, 
                          volatile uint8_t *buffer, 
                          int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent condition */
        if ((g_volatile_seed + i) % 3 == 0) {
            /* Bit-field operation */
            bf_array[i % 4].c = (bf_array[i % 4].c + i) & 0xFFF;
        } else if ((g_volatile_seed + i) % 3 == 1) {
            /* Subword memory access */
            *(volatile uint16_t *)(buffer + i * 2) = 
                (uint16_t)(bf_array[i % 4].b * 2);
        } else {
            /* Complex memory addressing */
            volatile int *ptr = (volatile int *)(buffer + i);
            *ptr = *ptr ^ g_volatile_seed;
        }
        
        /* Register variable with inline asm */
        register int tmp asm("r11") = i;
        asm volatile("" : "+r" (tmp));
    }
}

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char *argv[]) {
    int iterations = 100;
    
    /* Use command-line argument for runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Allocate and initialize test data */
    struct bitfield_struct bf_array[4] = {0};
    volatile uint8_t buffer[256];
    int indices[50];
    
    /* Initialize with runtime-dependent values */
    for (int i = 0; i < 256; i++) {
        buffer[i] = (uint8_t)(g_volatile_seed + i);
    }
    for (int i = 0; i < 50; i++) {
        indices[i] = (g_volatile_seed * i) % 40;
    }
    
    printf("Starting tests with iterations = %d\n", iterations);
    
    /* Execute test functions */
    test_bitfield_ops(&bf_array[0], iterations);
    test_asm_clobber();
    test_subreg_mem(buffer, 256);
    test_complex_mem_addressing((volatile int *)buffer, indices, 50);
    test_mixed_operations(bf_array, buffer, iterations);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += buffer[i];
    }
    for (int i = 0; i < 4; i++) {
        checksum += bf_array[i].a + bf_array[i].b + 
                   bf_array[i].c + bf_array[i].d;
    }
    
    printf("Checksum: %u\n", (unsigned int)checksum);
    printf("Tests completed successfully.\n");
    
    return 0;
}
