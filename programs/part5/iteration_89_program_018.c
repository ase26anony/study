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
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* Global volatile memory buffer for MEM RTL */
volatile uint32_t g_mem_buffer[256];

/* Test 1: Bit-field assignments with volatile control flow */
void test_bitfield_operations(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent assignments to prevent optimization */
        if (g_volatile_input & (1 << i)) {
            bf->a = (i & 0xF);          /* Likely ZERO_EXTRACT */
            bf->b = (i * 3) & 0xFF;     /* Bit-field store */
        } else {
            bf->c = (i * 5) & 0xFFF;    /* Another bit-field */
            bf->d = (i * 7) & 0xFF;
        }
        
        /* Force register spill/reload with inline asm */
        asm volatile("" : "+r" (i) : : "memory");
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobbers(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    struct bitfield_struct bf_local;
    
    /* Move between register variable and bit-field */
    for (int i = 0; i < 4; i++) {
        /* Inline asm that clobbers hard registers */
        asm volatile(
            "mov %[val], %[reg]\n\t"
            : [reg] "=r" (reg_var)
            : [val] "r" (i)
            : "r11", "r12", "memory"
        );
        
        /* Bit-field assignment after register clobber - may generate STRICT_LOW_PART */
        bf_local.b = reg_var & 0xFF;
        
        /* Use the value to prevent dead code elimination */
        g_mem_buffer[i] = bf_local.b;
    }
}

/* Test 3: Mixed-type accesses via unions and pointers */
void test_mixed_type_accesses(volatile uint8_t *mem_base, int offset) {
    union mixed_access *u = (union mixed_access *)&mem_base[offset];
    
    /* Write whole word then access sub-parts - may generate SUBREG */
    u->word = 0xDEADBEEF;
    
    /* Access sub-registers */
    u->half[1] = g_volatile_input & 0xFFFF;  /* SUBREG of MEM */
    
    /* Byte access through volatile pointer */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)&u->word;
    for (int i = 0; i < 4; i++) {
        byte_ptr[i] = (g_volatile_input >> (i * 8)) & 0xFF;
    }
}

/* Test 4: Complex memory addressing modes */
void test_complex_mem_addressing(int index) {
    /* Array with volatile elements */
    volatile struct {
        uint32_t data;
        uint16_t tag;
    } complex_array[16];
    
    /* Data-dependent index prevents optimization */
    int idx = (g_volatile_input + index) & 0xF;
    
    /* Complex addressing: array + struct field + bit manipulation */
    uint32_t *ptr = (uint32_t *)&complex_array[idx].data;
    
    /* This should generate MEM RTL with address expression */
    *ptr = (*ptr & 0xFFFF0000) | (g_volatile_input & 0xFFFF);
    
    /* Additional MEM access with offset */
    uint16_t *tag_ptr = (uint16_t *)&complex_array[idx].tag;
    *tag_ptr = (*ptr >> 16) & 0xFFFF;
}

/* Test 5: Loop with volatile memory updates and bit-field manipulation */
void test_combined_operations(void) {
    struct bitfield_struct bf_array[8];
    volatile uint32_t checksum = 0;
    
    for (int i = 0; i < 8; i++) {
        /* Initialize with pattern */
        bf_array[i].a = i;
        bf_array[i].b = i * 2;
        bf_array[i].c = i * 3;
        bf_array[i].d = i * 4;
        
        /* Update based on volatile memory */
        if (g_mem_buffer[i] & 1) {
            /* Modify bit-field - potential ZERO_EXTRACT */
            bf_array[i].b ^= 0x55;
        }
        
        /* Update volatile memory based on bit-field */
        g_mem_buffer[i] = bf_array[i].c;
        
        /* Calculate checksum to ensure execution */
        checksum += bf_array[i].a + bf_array[i].b + bf_array[i].c + bf_array[i].d;
    }
    
    /* Use checksum to prevent optimization */
    asm volatile("" : : "r" (checksum) : "memory");
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for runtime variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    
    /* Initialize test structures */
    struct bitfield_struct bf_main;
    union mixed_access u_main;
    
    printf("Starting coverage test with iterations=%d\n", iterations);
    
    /* Run all test patterns */
    test_bitfield_operations(&bf_main, iterations);
    test_asm_clobbers();
    test_mixed_type_accesses((volatile uint8_t *)g_mem_buffer, 16);
    test_complex_mem_addressing(iterations);
    test_combined_operations();
    
    /* Final memory barrier */
    asm volatile("" : : : "memory");
    
    printf("Test completed successfully\n");
    
    /* Print some results to verify execution */
    printf("Sample values: bf_main.a=%u, g_mem_buffer[0]=%u\n", 
           bf_main.a, (unsigned int)g_mem_buffer[0]);
    
    return 0;
}
