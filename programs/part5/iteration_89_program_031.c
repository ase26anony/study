/* 
 * Test program targeting uncovered lines in resource.cc (lines 282-290)
 * Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_rtl test_rtl.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_input = 0;

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

/* Global variables for memory operations */
volatile uint32_t g_mem_buffer[64];
struct bitfield_struct g_bf_array[16];

/* Function 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(int idx) {
    struct bitfield_struct *p = &g_bf_array[idx];
    
    /* Multiple bit-field assignments - may generate ZERO_EXTRACT */
    p->a = (g_input & 0xF);
    p->b = (g_input >> 4) & 0xFF;
    p->c = (g_input >> 8) & 0xFFF;
    p->d = (g_input >> 20) & 0xFF;
    
    /* Chain assignments to create complex RTL patterns */
    if (idx > 0) {
        g_bf_array[idx-1].b = p->a;
        g_bf_array[(idx+1) & 15].c = p->b;
    }
}

/* Function 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(void) {
    register uint32_t reg_var asm("r12") = g_input;
    uint32_t temp;
    
    /* Inline assembly that clobbers registers */
    asm volatile (
        "mov %1, %0\n\t"
        "ror $8, %0\n\t"
        : "=r" (temp)
        : "r" (reg_var)
        : "cc"
    );
    
    /* Use the result in bit-field to create STRICT_LOW_PART */
    struct bitfield_struct local_bf;
    local_bf.b = temp & 0xFF;  /* Potential STRICT_LOW_PART */
    local_bf.c = (temp >> 8) & 0xFFF;
    
    g_bf_array[0].a = local_bf.b;
}

/* Function 3: Memory accesses with type-punning for SUBREG generation */
void test_mem_subreg(int offset) {
    union mixed_types *mem_ptr = (union mixed_types*)&g_mem_buffer[offset];
    
    /* Write through different-sized views - may generate SUBREG of MEM */
    mem_ptr->word = 0xDEADBEEF;
    
    /* Access sub-parts - potential SUBREG generation */
    mem_ptr->half[1] = g_input & 0xFFFF;
    mem_ptr->byte[0] = (g_input >> 16) & 0xFF;
    
    /* Pointer arithmetic with different types */
    uint8_t *byte_ptr = (uint8_t*)&g_mem_buffer[offset];
    uint16_t *half_ptr = (uint16_t*)(byte_ptr + 2);
    *half_ptr = *byte_ptr | 0x100;  /* Could generate SUBREG */
}

/* Function 4: Complex addressing modes for MEM_P(x) path */
void test_complex_addressing(int idx) {
    volatile uint32_t *ptr;
    int i;
    
    /* Loop with volatile and complex addressing */
    for (i = 0; i < 4; i++) {
        ptr = &g_mem_buffer[idx + i * 3 + (g_input & 3)];
        *ptr = *ptr + i + 1;  /* MEM with addressing side effects */
        
        /* Additional memory op with offset */
        *(ptr + 1) = *(ptr - 1) ^ 0xABCD;
    }
}

/* Function 5: Mixed operations in data-dependent loop */
void test_mixed_operations(int iterations) {
    int i;
    union mixed_types local_union;
    
    for (i = 0; i < iterations; i++) {
        /* Alternate between different operations based on input */
        if ((g_input + i) & 1) {
            /* Bit-field operation */
            g_bf_array[i & 15].a = (i * 3) & 0xF;
            g_bf_array[i & 15].b = (g_input + i) & 0xFF;
        } else {
            /* Memory operation with type punning */
            uint32_t *word_ptr = &g_mem_buffer[i & 31];
            uint16_t *half_ptr = (uint16_t*)word_ptr;
            *half_ptr = (*half_ptr + i) & 0xFFFF;  /* SUBREG potential */
        }
        
        /* Union-based type punning */
        local_union.word = g_input + i;
        g_mem_buffer[(i + 1) & 31] = local_union.half[0] | (local_union.byte[3] << 16);
    }
}

/* Main function with runtime-dependent control flow */
int main(int argc, char *argv[]) {
    int i, iterations;
    
    /* Use command-line argument for runtime variability */
    iterations = (argc > 1) ? atoi(argv[1]) : 8;
    if (iterations < 1) iterations = 8;
    if (iterations > 32) iterations = 32;
    
    /* Initialize with non-zero pattern */
    for (i = 0; i < 64; i++) {
        g_mem_buffer[i] = i * 0x1234567;
    }
    
    for (i = 0; i < 16; i++) {
        g_bf_array[i].a = i;
        g_bf_array[i].b = i * 2;
        g_bf_array[i].c = i * 100;
        g_bf_array[i].d = i * 3;
    }
    
    /* Execute test functions in data-dependent order */
    for (i = 0; i < iterations; i++) {
        g_input = i * 17 + argc;  /* Volatile modified each iteration */
        
        test_bitfield_ops(i & 15);
        
        if (i & 1) {
            test_asm_clobber();
        }
        
        test_mem_subreg((i * 7) & 31);
        
        if ((i + g_input) & 2) {
            test_complex_addressing(i & 28);
        }
    }
    
    test_mixed_operations(iterations);
    
    /* Compute and print checksum to ensure execution */
    uint32_t checksum = 0;
    for (i = 0; i < 64; i++) {
        checksum ^= g_mem_buffer[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    for (i = 0; i < 16; i++) {
        checksum += g_bf_array[i].a;
        checksum += g_bf_array[i].b << 8;
        checksum += g_bf_array[i].c << 16;
        checksum += g_bf_array[i].d;
    }
    
    printf("Final checksum: 0x%08X\n", checksum);
    printf("Test completed with %d iterations\n", iterations);
    
    return 0;
}
