/* 
 * Test program targeting uncovered lines in GCC's resource.cc
 * Specifically aims to trigger mark_set_resources paths for:
 * - ZERO_EXTRACT / STRICT_LOW_PART (bit-field assignments)
 * - SUBREG of MEM (subword memory operations)
 * - MEM_P (memory writes with complex addressing)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
static volatile int g_volatile_input = 0;

/* ========== Bit-field structures for ZERO_EXTRACT/STRICT_LOW_PART ========== */

struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

struct nested_bitfields {
    struct {
        unsigned int low : 4;
        unsigned int high : 4;
    } byte1;
    struct {
        unsigned int low : 6;
        unsigned int high : 10;
    } word1;
} __attribute__((packed));

/* ========== Union for type-punning and SUBREG generation ========== */

union type_punner {
    uint32_t full;
    uint16_t half[2];
    uint8_t byte[4];
    struct {
        uint16_t lo;
        uint16_t hi;
    } parts;
};

/* ========== Test functions targeting specific RTL patterns ========== */

/* 
 * Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART
 * Mixed with volatile to prevent optimization
 */
void test_bitfield_operations(struct bitfield_struct *bf, int iterations) {
    volatile int i;
    for (i = 0; i < iterations; i++) {
        /* These assignments often generate ZERO_EXTRACT or STRICT_LOW_PART */
        bf->a = (i & 0x7);                    /* 3-bit field */
        bf->b = ((i >> 3) & 0x1F);            /* 5-bit field */
        bf->c = ((i >> 8) & 0xFF);            /* 8-bit field */
        bf->d = ((i >> 16) & 0xFFFF);         /* 16-bit field */
        
        /* Force dependency between fields */
        bf->a = bf->b & 0x7;
        bf->c = bf->d & 0xFF;
    }
}

/*
 * Test 2: Inline assembly with clobbers to stress reload pass
 * Ties C variables to specific hardware registers
 */
void test_asm_clobbers(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    struct nested_bitfields nb;
    
    /* Inline asm that clobbers registers, forcing reload to manage resources */
    asm volatile (
        "mov %[input], %[temp]\n\t"
        "and $0xF, %[temp]\n\t"
        : [temp] "=r" (reg_var)
        : [input] "r" (reg_var)
        : "cc" /* Clobber condition codes */
    );
    
    /* Bit-field assignment after asm - may generate STRICT_LOW_PART */
    nb.byte1.low = reg_var & 0xF;
    nb.byte1.high = (reg_var >> 4) & 0xF;
    
    /* Another asm with different clobber */
    asm volatile (
        "ror $8, %[val]\n\t"
        : [val] "+r" (reg_var)
        :
        : "cc"
    );
    
    nb.word1.low = reg_var & 0x3F;
    nb.word1.high = (reg_var >> 6) & 0x3FF;
}

/*
 * Test 3: Subword memory operations for SUBREG of MEM
 * Uses volatile pointers and type-punning
 */
void test_subword_mem_access(volatile uint32_t *mem_base, int offset) {
    volatile uint16_t *half_ptr;
    volatile uint8_t *byte_ptr;
    union type_punner pun;
    
    /* Write full word - generates MEM */
    *mem_base = 0xDEADBEEF;
    
    /* Write half-word - may generate SUBREG of MEM */
    half_ptr = (volatile uint16_t *)(mem_base + offset);
    *half_ptr = 0xCAFE;
    
    /* Write byte - another SUBREG of MEM */
    byte_ptr = (volatile uint8_t *)(mem_base + offset + 1);
    *byte_ptr = 0x42;
    
    /* Type-punning through union */
    pun.full = *mem_base;
    pun.half[0] = pun.half[1];  /* SUBREG access */
    pun.byte[2] = pun.byte[0];  /* Another SUBREG */
    
    /* Write back through union */
    *mem_base = pun.full;
}

/*
 * Test 4: Complex addressing modes for MEM_P with addressing computation
 * Array access with data-dependent index
 */
void test_complex_addressing(volatile uint32_t *array, int size) {
    int i, idx;
    uint32_t temp;
    
    for (i = 0; i < size; i++) {
        /* Data-dependent index prevents optimization */
        idx = (i + g_volatile_input) % size;
        
        /* Complex addressing: array[idx + 1] = array[idx] + array[idx-1] */
        temp = array[idx];
        if (idx > 0) {
            temp += array[idx - 1];
        }
        
        /* This generates MEM with address computation (XEXP(x, 0)) */
        array[idx + 1] = temp;
        
        /* Bit-field in the middle to mix patterns */
        struct bitfield_struct *bf_ptr = (struct bitfield_struct *)&array[idx];
        bf_ptr->b = (temp >> 3) & 0x1F;
    }
}

/*
 * Test 5: Mixed operations in loop - combines all patterns
 */
void test_mixed_operations(void) {
    static volatile uint32_t buffer[64];
    struct bitfield_struct bf;
    union type_punner pun;
    int i;
    
    for (i = 0; i < 64; i++) {
        /* 1. Bit-field assignment (ZERO_EXTRACT/STRICT_LOW_PART) */
        bf.a = (i & 0x7);
        bf.b = ((i >> 3) & 0x1F);
        
        /* 2. Memory write (MEM_P) */
        buffer[i] = i * 0x01010101;
        
        /* 3. Subword access (SUBREG of MEM) */
        uint16_t *half = (uint16_t *)&buffer[i];
        half[0] = half[1] ^ 0xFFFF;
        
        /* 4. Type-punning (more SUBREG) */
        pun.full = buffer[i];
        pun.parts.lo = pun.parts.hi;
        buffer[i] = pun.full;
        
        /* 5. Inline asm occasionally */
        if (i % 8 == 0) {
            register uint32_t r asm("eax") = buffer[i];
            asm volatile ("bswap %0" : "+r" (r));
            buffer[i] = r;
        }
    }
}

/* ========== Main test driver ========== */

int main(int argc, char *argv[]) {
    struct bitfield_struct bf_test = {0};
    volatile uint32_t memory_buffer[128] = {0};
    int iterations = 10;
    
    /* Use command-line argument for runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0 || iterations > 100) {
            iterations = 10;
        }
    }
    
    printf("Starting RTL pattern tests (targeting resource.cc lines 282-290)\n");
    printf("Using %d iterations\n\n", iterations);
    
    /* Run all tests */
    test_bitfield_operations(&bf_test, iterations);
    printf("Test 1 (bit-fields) completed\n");
    
    test_asm_clobbers();
    printf("Test 2 (asm clobbers) completed\n");
    
    test_subword_mem_access(memory_buffer, 16);
    printf("Test 3 (subword memory) completed\n");
    
    test_complex_addressing(memory_buffer, 64);
    printf("Test 4 (complex addressing) completed\n");
    
    test_mixed_operations();
    printf("Test 5 (mixed operations) completed\n\n");
    
    /* Compute checksum to ensure actual execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 128; i++) {
        checksum ^= memory_buffer[i];
    }
    checksum ^= bf_test.a ^ bf_test.b ^ bf_test.c ^ bf_test.d;
    
    printf("Final checksum: 0x%08X\n", checksum);
    printf("All tests completed successfully\n");
    
    return 0;
}
