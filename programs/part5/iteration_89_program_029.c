/* Compile with: gcc -O2 -fdump-rtl-all -fdump-tree-all -o test_program test.c */
/* Also try: gcc -O3 -fno-strict-aliasing -frename-registers -o test_program test.c */
/* And: gcc -O1 -fschedule-insns -fno-merge-bitfields -o test_program test.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART RTL */
struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

/* Volatile memory buffer for MEM RTL generation */
volatile uint32_t mem_buffer[256];

/* Union for type-punning to generate SUBREG RTL */
union mixed_access {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, volatile int trigger) {
    for (int i = 0; i < 100; i++) {
        /* Data-dependent assignments prevent constant propagation */
        if (trigger & (1 << (i & 7))) {
            bf->a = (i & 0x7);           /* 3-bit field */
            bf->b = (i & 0x1F);          /* 5-bit field */
        } else {
            bf->c = (i & 0xFF);          /* 8-bit field */
            bf->d = (i & 0xFFFF);        /* 16-bit field */
        }
        
        /* Mix with memory access to create register pressure */
        mem_buffer[i % 256] = i;
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(volatile int *input) {
    register int reg_var asm ("r12") = *input;
    struct bitfield_struct local_bf;
    
    /* Inline assembly that clobbers hard registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (reg_var)
        : "r" (reg_var)
        : "%eax", "%ebx", "cc"
    );
    
    /* Use the register variable with bit-field */
    local_bf.a = reg_var & 0x7;
    local_bf.b = (reg_var >> 3) & 0x1F;
    
    /* Force memory reference */
    mem_buffer[0] = local_bf.d;
}

/* Test 3: Type-punning and volatile memory for SUBREG and MEM RTL */
void test_mixed_access(volatile int count) {
    union mixed_access ma;
    volatile uint8_t *byte_ptr;
    
    for (int i = 0; i < count; i++) {
        /* Access through different-sized types to generate SUBREG */
        ma.word = i * 0x01010101;
        
        /* SUBREG of MEM pattern: access subword of memory location */
        byte_ptr = (volatile uint8_t *)&mem_buffer[i % 256];
        *byte_ptr = ma.byte[i & 3];  /* char store to int memory */
        
        /* Another SUBREG pattern: short within long */
        ((volatile uint16_t *)&mem_buffer[(i + 1) % 256])[0] = ma.half[0];
        
        /* Complex addressing mode for MEM RTL */
        mem_buffer[(i * 17) % 256] = ma.word + ((i & 1) ? 1000 : 2000);
    }
}

/* Test 4: Register variable with bit-field in loop (STRICT_LOW_PART potential) */
void test_register_bitfield(volatile int iterations) {
    register uint32_t reg_data asm ("ebx");
    struct bitfield_struct bf_array[10];
    
    reg_data = iterations;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        /* Complex expression to prevent optimization */
        uint32_t val = reg_data + i * 3;
        
        /* Multiple bit-field assignments */
        bf_array[i].a = (val >> 0) & 0x7;
        bf_array[i].b = (val >> 3) & 0x1F;
        bf_array[i].c = (val >> 8) & 0xFF;
        bf_array[i].d = (val >> 16) & 0xFFFF;
        
        /* Update register variable with memory content */
        asm volatile ("" : "+r" (reg_data) : : "memory");
        reg_data ^= mem_buffer[i % 256];
    }
}

/* Main function with runtime-dependent control flow */
int main(int argc, char *argv[]) {
    struct bitfield_struct bf = {0};
    volatile int trigger_value;
    
    /* Use command-line argument for runtime variability */
    if (argc > 1) {
        trigger_value = atoi(argv[1]);
    } else {
        trigger_value = 42;
    }
    
    /* Initialize memory buffer */
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = i;
    }
    
    /* Run all tests with data-dependent control flow */
    test_bitfield_ops(&bf, trigger_value);
    test_asm_clobber(&trigger_value);
    test_mixed_access(trigger_value % 100);
    test_register_bitfield(trigger_value % 20);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= mem_buffer[i];
    }
    
    /* Include bit-field in checksum */
    checksum ^= *(uint32_t*)&bf;
    
    printf("Checksum: 0x%08X\n", checksum);
    return checksum != 0 ? 0 : 1;
}
