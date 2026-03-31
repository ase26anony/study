/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o coverage_test coverage_test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_volatile_input = 0;

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

/* Inline assembly to force hard register usage */
#define FORCE_REGISTER(var, reg) \
    register uint32_t var asm(reg)

/* Test 1: Bit-field assignments with volatile control flow */
void test_bitfield_operations(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent assignments to prevent optimization */
        if (g_volatile_input & 1) {
            bf->a = (i & 0xF);           /* ZERO_EXTRACT likely */
            bf->c = (i >> 4) & 0xFFF;
        } else {
            bf->b = (i * 3) & 0xFF;      /* STRICT_LOW_PART possible */
            bf->d = (i + 5) & 0xFF;
        }
        
        /* Volatile memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
}

/* Test 2: Mixed-type memory accesses with SUBREG generation */
void test_mixed_type_access(volatile union mixed_access *mem, int offset) {
    /* Write full word (MEM) */
    mem[offset].full = 0xDEADBEEF;
    
    /* Write half-word (SUBREG of MEM) */
    mem[offset + 1].half[0] = 0x1234;
    
    /* Write byte (another SUBREG) */
    mem[offset + 2].bytes[3] = 0xAB;
    
    /* Complex addressing mode */
    volatile uint16_t *ptr = &mem[offset].half[1];
    *ptr = g_volatile_input & 0xFFFF;
}

/* Test 3: Inline assembly with clobbers to stress reload */
void test_register_clobber(void) {
    FORCE_REGISTER(reg_var, "r12");
    uint32_t local_var;
    
    /* Move between C variable and hard register */
    asm volatile (
        "mov %[input], %[reg]\n\t"
        "add $0x42, %[reg]\n\t"
        "mov %[reg], %[output]"
        : [reg] "=&r" (reg_var), [output] "=m" (local_var)
        : [input] "m" (g_volatile_input)
        : "cc"
    );
    
    /* Use the result in a bit-field */
    struct bitfield_struct bf_local;
    bf_local.b = local_var & 0xFF;  /* Potential STRICT_LOW_PART */
}

/* Test 4: Complex memory addressing with volatile */
void test_complex_mem_addressing(volatile uint32_t *base, int size) {
    for (int i = 0; i < size; i++) {
        /* Data-dependent index prevents optimization */
        int idx = (g_volatile_input + i) % size;
        
        /* Memory write with complex addressing */
        base[idx * 2] = base[idx * 2 + 1] ^ 0xAAAAAAAA;
        
        /* Volatile read to create dependency */
        asm volatile("" : "+m" (*base));
    }
}

int main(int argc, char *argv[]) {
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    
    /* Initialize test structures */
    struct bitfield_struct bf_array[4];
    volatile union mixed_access memory[8];
    volatile uint32_t mem_buffer[32];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 4; i++) {
        bf_array[i].full = 0;
    }
    for (int i = 0; i < 8; i++) {
        memory[i].full = i * 0x11111111;
    }
    for (int i = 0; i < 32; i++) {
        mem_buffer[i] = i;
    }
    
    /* Run tests to trigger uncovered RTL patterns */
    test_bitfield_operations(&bf_array[0], iterations);
    test_mixed_type_access(&memory[0], 1);
    test_register_clobber();
    test_complex_mem_addressing(mem_buffer, 16);
    
    /* Additional loop with mixed operations */
    for (int i = 0; i < iterations; i++) {
        /* Alternate between different operations */
        if (i % 3 == 0) {
            bf_array[i % 4].a = (memory[i % 8].bytes[0] & 0xF);
        } else if (i % 3 == 1) {
            memory[i % 8].half[1] = bf_array[i % 4].b;
        } else {
            mem_buffer[i % 16] = memory[i % 8].full;
        }
    }
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum ^= bf_array[i].full;
    }
    for (int i = 0; i < 8; i++) {
        checksum ^= memory[i].full;
    }
    for (int i = 0; i < 32; i++) {
        checksum ^= mem_buffer[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    return 0;
}
