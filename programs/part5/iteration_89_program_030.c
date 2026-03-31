/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o coverage_test coverage_test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} __attribute__((packed));

/* Volatile memory buffer for MEM operations */
volatile uint32_t mem_buffer[256];

/* Union for type-punning to generate SUBREG */
union type_pun {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* Test 1: Bit-field assignments for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(struct bitfield_struct *bf, volatile int input) {
    /* Loop with data-dependent index to prevent optimization */
    for (int i = 0; i < 4; i++) {
        /* Complex bit-field assignments */
        bf[i].a = (input >> (i * 4)) & 0xF;
        bf[i].b = (input >> (i * 8)) & 0xFF;
        bf[i].c = (input >> (i * 12)) & 0xFFF;
        bf[i].d = (input >> (i * 8)) & 0xFF;
        
        /* Nested bit-field access */
        bf[i].a = bf[i].b & 0x7;
    }
}

/* Test 2: Inline assembly with clobbers for reload stress */
void test_asm_clobbers(volatile int *ptr) {
    int temp;
    register int reg_var asm("r12") = *ptr;
    
    /* Inline assembly that forces register allocation */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (temp)
        : "r" (reg_var)
        : "%eax", "memory"
    );
    
    /* Use the result in bit-field context */
    struct bitfield_struct local_bf;
    local_bf.b = temp & 0xFF;
    local_bf.c = temp & 0xFFF;
    
    *ptr = local_bf.c;
}

/* Test 3: Mixed-type accesses for SUBREG generation */
void test_mixed_type_accesses(union type_pun *data, volatile int idx) {
    /* Access different sized elements */
    data[idx].half[0] = 0x1234;
    data[idx].byte[2] = 0xAB;
    
    /* Pointer casting for SUBREG of MEM */
    uint32_t *word_ptr = &data[idx].word;
    uint16_t *half_ptr = (uint16_t *)word_ptr;
    
    /* Complex addressing with volatile */
    volatile uint16_t *vol_half = (volatile uint16_t *)word_ptr;
    *vol_half = idx * 2;
    
    /* SUBREG through pointer arithmetic */
    ((uint8_t *)word_ptr)[1] = idx & 0xFF;
}

/* Test 4: Memory operations with complex addressing */
void test_memory_operations(volatile uint32_t *mem, int size, volatile int offset) {
    /* Loop with volatile memory access */
    for (int i = 0; i < size; i++) {
        /* MEM with complex addressing */
        mem[(i + offset) % 256] = i * 0x01010101;
        
        /* Access via different pointer types */
        volatile uint16_t *short_ptr = (volatile uint16_t *)&mem[i];
        *short_ptr = i & 0xFFFF;
        
        /* Another level of indirection */
        volatile uint8_t *byte_ptr = (volatile uint8_t *)&mem[i];
        byte_ptr[2] = (i >> 8) & 0xFF;
    }
}

/* Test 5: Combined operations in data-dependent loop */
void test_combined_operations(struct bitfield_struct *bf_array, 
                              union type_pun *pun_array,
                              volatile uint32_t *mem,
                              volatile int control) {
    for (int i = 0; i < 8; i++) {
        /* Data-dependent branching */
        if (control & (1 << i)) {
            /* Bit-field operation */
            bf_array[i].a = (mem[i] >> 4) & 0xF;
            bf_array[i].b = bf_array[i].a * 2;
            
            /* Type-punning operation */
            pun_array[i].half[0] = bf_array[i].b;
            pun_array[i].byte[3] = i;
            
            /* Memory operation with complex address */
            mem[(i + control) % 256] = pun_array[i].word;
        } else {
            /* Different path with SUBREG */
            uint32_t temp = mem[i];
            ((uint16_t *)&temp)[1] = i * 0x100;
            mem[i] = temp;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Initialize test data */
    struct bitfield_struct bf_array[16] = {0};
    union type_pun pun_array[16] = {0};
    
    /* Use command-line argument for runtime variability */
    volatile int input = (argc > 1) ? atoi(argv[1]) : 12345;
    
    /* Clear memory buffer */
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = 0;
    }
    
    /* Run all tests */
    test_bitfield_operations(bf_array, input);
    test_asm_clobbers(&input);
    test_mixed_type_accesses(pun_array, input % 16);
    test_memory_operations(mem_buffer, 64, input % 128);
    test_combined_operations(bf_array, pun_array, mem_buffer, input);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum ^= bf_array[i].a + bf_array[i].b + bf_array[i].c + bf_array[i].d;
        checksum ^= pun_array[i].word;
    }
    
    for (int i = 0; i < 64; i++) {
        checksum ^= mem_buffer[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    return 0;
}
