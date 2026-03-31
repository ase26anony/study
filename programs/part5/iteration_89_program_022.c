/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_resource test_resource.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force runtime values to prevent constant propagation */
static volatile int g_volatile_input = 0;

/* Bit-field structures to generate ZERO_EXTRACT/STRICT_LOW_PART */
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

/* Global with register variable to stress reload */
register uint32_t reg_var asm ("r12");

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent indexing prevents optimization */
        int idx = g_volatile_input + i;
        
        /* Multiple bit-field writes - may generate STRICT_LOW_PART */
        bf[idx % 4].a = (i & 0xF);
        bf[idx % 4].b = (i >> 4) & 0xFF;
        bf[idx % 4].c = (i >> 8) & 0xFFF;
        bf[idx % 4].d = (i >> 20) & 0xFF;
        
        /* Compound assignment on bit-field */
        bf[(idx + 1) % 4].a += 1;
        bf[(idx + 1) % 4].b |= 0x55;
    }
}

/* Test 2: Inline assembly with clobbers to force reload pass */
void test_asm_clobber(void) {
    uint32_t local_var = 0x12345678;
    uint32_t result;
    
    /* Inline asm that ties C variable to hard register */
    asm volatile (
        "mov %[input], %[reg]\n\t"
        "ror $8, %[reg]\n\t"
        "mov %[reg], %[output]\n\t"
        : [output] "=r" (result)
        : [input] "r" (local_var),
          [reg] "r" (reg_var)
        : "cc"
    );
    
    /* Use result to prevent dead code elimination */
    g_volatile_input = result & 1;
}

/* Test 3: Memory accesses with type-punning for SUBREG/MEM */
void test_mem_subreg(volatile union mixed_types *mem, int size) {
    for (int i = 0; i < size; i++) {
        /* Write whole word (MEM) */
        mem[i].word = i * 0x01010101;
        
        /* Write half-word (SUBREG of MEM) */
        mem[i].half[0] = (i * 0x1001) & 0xFFFF;
        
        /* Write byte (another SUBREG of MEM) */
        mem[i].byte[3] = i & 0xFF;
        
        /* Complex addressing mode */
        volatile uint16_t *ptr = &mem[(i + g_volatile_input) % size].half[1];
        *ptr = (i * 7) & 0xFFFF;
    }
}

/* Test 4: Volatile pointer arithmetic for MEM_P patterns */
void test_volatile_mem(volatile uint32_t *mem, int count) {
    volatile uint32_t *ptr = mem;
    
    for (int i = 0; i < count; i++) {
        /* Different addressing modes */
        if (i & 1) {
            ptr[i] = ptr[i - 1] + g_volatile_input;
        } else {
            *(ptr + i) = i * 0xDEADBEEF;
        }
        
        /* Post-increment pattern */
        volatile uint32_t temp = *ptr++;
        *ptr = temp >> 16;
    }
}

/* Test 5: Mixed operations in loop with data-dependent flow */
void test_mixed_operations(struct bitfield_struct *bf, 
                          union mixed_types *mem,
                          volatile uint32_t *vmem,
                          int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent branch */
        if (g_volatile_input & (1 << (i % 8))) {
            /* Path 1: Bit-field operations */
            bf[i % 4].b = (bf[i % 4].b + mem[i % 8].byte[0]) & 0xFF;
            bf[i % 4].c ^= vmem[i % 16];
        } else {
            /* Path 2: Memory/SUBREG operations */
            mem[i % 8].half[1] = bf[i % 4].a | (bf[i % 4].b << 4);
            vmem[i % 16] = mem[i % 8].word;
        }
        
        /* Inline asm barrier to prevent optimization */
        asm volatile ("" : : : "memory");
    }
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize test data */
    struct bitfield_struct bf_array[4] = {0};
    union mixed_types mem_array[8] = {0};
    volatile uint32_t volatile_mem[16] = {0};
    
    /* Initialize register variable */
    reg_var = 0x87654321;
    
    printf("Starting resource tracking tests...\n");
    printf("Iterations: %d\n", iterations);
    
    /* Run tests to generate specific RTL patterns */
    test_bitfield_ops(bf_array, iterations);
    test_asm_clobber();
    test_mem_subreg(mem_array, 8);
    test_volatile_mem(volatile_mem, 16);
    test_mixed_operations(bf_array, mem_array, volatile_mem, iterations);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum ^= *(uint32_t*)&bf_array[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum ^= mem_array[i].word;
    }
    for (int i = 0; i < 16; i++) {
        checksum ^= volatile_mem[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
