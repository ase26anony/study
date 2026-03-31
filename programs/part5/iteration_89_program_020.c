/* test_resource_cc.c
 * 
 * This program is designed to trigger the uncovered lines in resource.cc
 * (lines 282-290) by generating specific RTL patterns during compilation.
 * It focuses on bit-field assignments, sub-register accesses, and memory
 * operations that should produce ZERO_EXTRACT, STRICT_LOW_PART, SUBREG,
 * and MEM expressions in the RTL.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
static volatile int g_volatile_input = 0;

/* ==================== Bit-field Structures ==================== */

/* Struct with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
};

/* Packed struct to discourage optimization */
struct __attribute__((packed)) packed_bitfield {
    unsigned char x : 2;
    unsigned char y : 3;
    unsigned char z : 3;
};

/* ==================== Union for Type-Punning ==================== */

union type_pun {
    uint32_t word;
    uint16_t half[2];
    uint8_t  byte[4];
};

/* ==================== Test Functions ==================== */

/* Function 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int idx) {
    /* Data-dependent index prevents optimization */
    volatile int i = idx & 3;
    
    /* Multiple bit-field writes - may generate STRICT_LOW_PART */
    bf[i].a = (i * 7) & 0x7;
    bf[i].b = (i * 13) & 0x1F;
    bf[i].c = (i * 31) & 0xFF;
    bf[i].d = (i * 127) & 0xFFFF;
    
    /* Chain of assignments to same bit-field */
    struct packed_bitfield pb;
    pb.x = 1;
    pb.y = 3;
    pb.z = 5;
    
    /* Use the result to prevent dead code elimination */
    g_volatile_input = bf[i].a + pb.x;
}

/* Function 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(void) {
    int input = g_volatile_input;
    int output1, output2;
    
    /* Inline asm that uses specific registers and clobbers */
    asm volatile (
        "mov %2, %%r12\n\t"          /* Use r12 register */
        "add $1, %%r12\n\t"
        "mov %%r12, %0\n\t"
        "mov %2, %%r13\n\t"          /* Use r13 register */
        "sub $1, %%r13\n\t"
        "mov %%r13, %1\n\t"
        : "=r" (output1), "=r" (output2)
        : "r" (input)
        : "%r12", "%r13", "cc"
    );
    
    /* Use outputs to prevent optimization */
    g_volatile_input = output1 + output2;
}

/* Function 3: Sub-register accesses via unions and casts */
void test_subreg_access(volatile uint32_t *mem) {
    union type_pun pun;
    
    /* Initialize with volatile input */
    pun.word = g_volatile_input;
    
    /* Access sub-parts - may generate SUBREG RTL */
    pun.half[0] = pun.half[1] ^ 0x55AA;
    pun.byte[2] = pun.byte[0] + pun.byte[3];
    
    /* Write back to memory */
    *mem = pun.word;
    
    /* Cast between different pointer types */
    volatile uint16_t *half_ptr = (volatile uint16_t *)mem;
    half_ptr[1] = half_ptr[0] + 0x100;  /* May generate SUBREG of MEM */
}

/* Function 4: Complex memory addressing modes */
void test_mem_addressing(volatile int *arr, int size) {
    int i;
    /* Loop with data-dependent index */
    for (i = g_volatile_input & 0xF; i < size && i < 16; i++) {
        /* Complex addressing: arr[i + (i>>1)] */
        int idx = i + (i >> 1);
        if (idx < size) {
            /* Memory write - should generate MEM RTL */
            arr[idx] = arr[i] * 3 + 1;
        }
        
        /* Bit-field within loop */
        struct bitfield_struct tmp;
        tmp.a = i & 0x7;
        tmp.b = (i * 2) & 0x1F;
        arr[i] = tmp.a + tmp.b;
    }
}

/* Function 5: Mixed size accesses with volatile */
void test_mixed_sizes(void) {
    static volatile uint32_t buffer[8];
    
    /* Access as different sizes */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)buffer;
    volatile uint16_t *half_ptr = (volatile uint16_t *)buffer;
    
    /* Pattern of mixed accesses */
    byte_ptr[4] = g_volatile_input & 0xFF;
    half_ptr[3] = half_ptr[1] + 0x1000;
    buffer[2] = buffer[0] ^ buffer[1];
    
    /* SUBREG may appear from byte/halfword access to word memory */
    byte_ptr[10] = byte_ptr[2];  /* Crosses word boundary */
}

/* ==================== Main Function ==================== */

int main(int argc, char *argv[]) {
    /* Initialize with command-line argument for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    g_volatile_input = seed;
    
    /* Allocate and initialize test structures */
    struct bitfield_struct bf_array[4] = {0};
    volatile uint32_t memory_buffer[16] = {0};
    volatile int int_array[32] = {0};
    
    printf("Starting resource.cc coverage test (seed=%d)\n", seed);
    
    /* Run test functions multiple times with different parameters */
    for (int i = 0; i < 3; i++) {
        test_bitfield_ops(bf_array, i + seed);
        test_asm_clobber();
        test_subreg_access(&memory_buffer[i % 8]);
        test_mem_addressing(int_array, 32);
        test_mixed_sizes();
        
        /* Modify seed for next iteration */
        g_volatile_input = (g_volatile_input * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Compute checksum of results */
    uint32_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum ^= (bf_array[i].a + bf_array[i].b + 
                    bf_array[i].c + bf_array[i].d);
    }
    for (int i = 0; i < 16; i++) {
        checksum ^= memory_buffer[i];
    }
    for (int i = 0; i < 32; i++) {
        checksum ^= int_array[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    return (checksum == 0) ? 0 : 1;
}
