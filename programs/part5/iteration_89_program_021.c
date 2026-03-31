/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_program test.c */
/* Also try: gcc -O3 -frename-registers -fno-merge-bitfields -o test_program test.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
volatile uint32_t mem_buffer[256];

/* Function 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, volatile int seed) {
    for (int i = 0; i < 100; i++) {
        /* Data-dependent assignments prevent constant propagation */
        bf->a = (seed + i) & 0xF;
        bf->b = (seed * i) & 0xFF;
        bf->c = (seed - i) & 0xFFF;
        bf->d = (seed ^ i) & 0xFF;
        
        /* Force reload by using inline assembly with clobbers */
        asm volatile("" : "+r" (bf->a) : : "r12", "r13");
    }
}

/* Function 2: Inline assembly with register variables for reload stress */
void test_register_bitfield(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    struct bitfield_struct local_bf;
    
    /* Mix register variable with bit-field access */
    local_bf.a = reg_var & 0xF;
    local_bf.b = (reg_var >> 4) & 0xFF;
    
    /* Inline assembly that clobbers the register */
    asm volatile(
        "mov %[reg], %[temp]\n\t"
        "ror %[temp]\n\t"
        : [temp] "=r" (reg_var)
        : [reg] "r" (reg_var)
        : "cc"
    );
    
    local_bf.c = reg_var & 0xFFF;
    local_bf.d = (reg_var >> 16) & 0xFF;
}

/* Function 3: Type-punning and volatile memory for SUBREG and MEM */
void test_mem_subreg(volatile int index) {
    union mixed_types *ptr;
    volatile uint32_t *mem_ptr;
    
    /* Ensure index is within bounds */
    int idx = index & 0xFF;
    
    /* SUBREG generation via type-punning */
    ptr = (union mixed_types *)&mem_buffer[idx];
    ptr->half[0] = idx * 2;
    ptr->byte[3] = idx;
    
    /* MEM generation with complex addressing */
    mem_ptr = &mem_buffer[idx + 1];
    *mem_ptr = *mem_ptr ^ 0xDEADBEEF;
    
    /* SUBREG of MEM through pointer cast */
    *(volatile uint16_t *)&mem_buffer[idx + 2] = idx * 3;
}

/* Function 4: Mixed-size accesses in loop for SUBREG generation */
void test_mixed_accesses(volatile int count) {
    uint32_t base = 0x1000;
    
    for (int i = 0; i < (count & 0x3F); i++) {
        /* Different size accesses to same memory */
        *(volatile uint8_t *)(&mem_buffer[i]) = i & 0xFF;
        *(volatile uint16_t *)(&mem_buffer[i]) += i;
        *(volatile uint32_t *)(&mem_buffer[i]) ^= base;
        
        /* Data-dependent index prevents optimization */
        base += i;
    }
}

/* Main function with runtime-dependent control flow */
int main(int argc, char *argv[]) {
    struct bitfield_struct bf_instance = {0};
    volatile int seed = 0;
    
    /* Use command-line argument for runtime variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 42; /* Default seed */
    }
    
    /* Initialize memory buffer */
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = i;
    }
    
    /* Execute test functions with data-dependent control */
    test_bitfield_ops(&bf_instance, seed);
    test_register_bitfield();
    
    for (int i = 0; i < 10; i++) {
        test_mem_subreg(seed + i);
    }
    
    test_mixed_accesses(seed);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= mem_buffer[i];
    }
    
    checksum ^= bf_instance.a;
    checksum ^= bf_instance.b << 4;
    checksum ^= bf_instance.c << 12;
    checksum ^= bf_instance.d << 24;
    
    printf("Checksum: 0x%08X\n", checksum);
    return 0;
}
