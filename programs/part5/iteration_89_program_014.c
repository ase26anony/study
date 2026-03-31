/* 
 * Test program targeting uncovered lines in resource.cc (lines 282-290)
 * Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_rtl test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
static volatile int g_volatile_input = 0;

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

/* 
 * Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART 
 * Multiple assignments with volatile index prevent optimization
 */
void test_bitfield_operations(void) {
    struct bitfield_struct bf[4];
    int i;
    
    /* Initialize with volatile to prevent constant folding */
    int idx = g_volatile_input & 3;
    
    /* Multiple bit-field writes - may generate STRICT_LOW_PART */
    bf[idx].a = 0x5;
    bf[idx].b = 0xAB;
    bf[idx].c = 0xDEF;
    bf[idx].d = 0x42;
    
    /* Chain assignments to create complex RTL patterns */
    bf[(idx + 1) & 3].a = bf[idx].b & 0xF;
    bf[(idx + 2) & 3].c = bf[idx].a | (bf[idx].b << 4);
}

/*
 * Test 2: Inline assembly with register variables and clobbers
 * Forces reload pass to analyze SET_DEST patterns
 */
void test_asm_register_clobber(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    struct bitfield_struct bf;
    
    /* Inline asm that clobbers hard register */
    asm volatile (
        "mov %1, %0\n\t"
        "add $0x1111, %0"
        : "=r" (reg_var)
        : "r" (reg_var)
        : "cc"
    );
    
    /* Use the register variable in bit-field assignment */
    bf.b = (reg_var >> 4) & 0xFF;
    bf.c = reg_var & 0xFFF;
    
    /* More asm to force reload analysis */
    asm volatile (
        "and $0xF, %0"
        : "+r" (reg_var)
        :
        : "cc"
    );
    
    bf.a = reg_var & 0xF;
}

/*
 * Test 3: Memory operations with type-punning for SUBREG generation
 * Accesses memory through different sized types
 */
void test_memory_subreg(void) {
    union mixed_access buffer[8];
    volatile int i = g_volatile_input & 7;
    
    /* Write whole word (MEM) */
    buffer[i].word = 0xDEADBEEF;
    
    /* Write half-word (SUBREG of MEM) */
    buffer[(i + 1) & 7].half[0] = 0xCAFE;
    
    /* Write byte (SUBREG of MEM) */
    buffer[(i + 2) & 7].byte[3] = 0x42;
    
    /* Complex addressing mode */
    uint32_t *ptr = &buffer[(i + 3) & 7].word;
    *ptr = (*ptr & 0xFFFF0000) | 0xBABE;
}

/*
 * Test 4: Volatile memory accesses in loop
 * Generates multiple MEM RTL patterns
 */
void test_volatile_mem_loop(void) {
    volatile uint32_t mem_buffer[16];
    union mixed_access *alias_ptr = (union mixed_access *)mem_buffer;
    int i, limit;
    
    /* Volatile limit prevents loop unrolling */
    limit = (g_volatile_input & 0xF) + 1;
    
    for (i = 0; i < limit; i++) {
        /* MEM access with volatile */
        mem_buffer[i] = i * 0x1001;
        
        /* SUBREG access through union */
        alias_ptr[i].half[1] = i * 0x22;
        
        /* Bit-field-like operation using masking */
        mem_buffer[(i + 1) & 0xF] = (mem_buffer[i] & 0xFF00FF00) | 0x00FF00FF;
    }
}

/*
 * Test 5: Complex addressing modes and pointer arithmetic
 * Forces MEM_P(x) path with non-trivial XEXP(x, 0)
 */
void test_complex_addressing(void) {
    struct bitfield_struct *bf_array = malloc(sizeof(struct bitfield_struct) * 8);
    volatile int idx = g_volatile_input;
    
    if (bf_array) {
        /* Array access with complex index calculation */
        bf_array[(idx * 3 + 1) & 7].a = idx & 0xF;
        bf_array[(idx * 5 + 2) & 7].b = (idx >> 4) & 0xFF;
        bf_array[(idx * 7 + 3) & 7].c = idx & 0xFFF;
        
        /* Pointer arithmetic with different types */
        uint8_t *byte_ptr = (uint8_t *)bf_array;
        byte_ptr[idx & 0x1F] = ~byte_ptr[(idx + 1) & 0x1F];
        
        free(bf_array);
    }
}

/* ==================== MAIN EXECUTION ==================== */

int main(int argc, char *argv[]) {
    int i, iterations;
    
    /* Use command-line argument for variability */
    iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 1) iterations = 5;
    if (iterations > 100) iterations = 100;
    
    printf("Testing RTL pattern generation for resource tracking\n");
    printf("Iterations: %d\n\n", iterations);
    
    for (i = 0; i < iterations; i++) {
        /* Update volatile input each iteration */
        g_volatile_input = i;
        
        /* Execute all test patterns */
        test_bitfield_operations();
        test_asm_register_clobber();
        test_memory_subreg();
        test_volatile_mem_loop();
        test_complex_addressing();
        
        /* Prevent loop optimization */
        asm volatile ("" : : : "memory");
    }
    
    printf("Test completed successfully.\n");
    printf("Compile with -O2 -fdump-rtl-all to see RTL patterns.\n");
    
    return 0;
}
