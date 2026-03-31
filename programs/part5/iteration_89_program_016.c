/* test_resource_cc.c
 * 
 * This program is designed to trigger the uncovered lines 282-290 in resource.cc
 * by generating specific RTL patterns during GCC compilation.
 * 
 * The uncovered code handles:
 * 1. ZERO_EXTRACT or STRICT_LOW_PART in SET_DEST
 * 2. SUBREG in SET_DEST  
 * 3. MEM in SET_DEST
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* ========== BIT-FIELD STRUCTURES ========== */
/* For ZERO_EXTRACT and STRICT_LOW_PART RTL generation */

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
        unsigned int mode : 2;
        unsigned int value : 6;
    } byte2;
} __attribute__((packed));

/* ========== VOLATILE MEMORY BUFFERS ========== */
/* For MEM RTL generation with complex addressing */

volatile uint32_t mem_buffer[256];
volatile uint16_t short_buffer[512];
volatile uint8_t byte_buffer[1024];

/* ========== TEST FUNCTIONS ========== */

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, volatile int trigger) {
    /* Loop with data-dependent index prevents constant propagation */
    for (int i = 0; i < 4; i++) {
        if (trigger & (1 << i)) {
            /* These assignments should generate ZERO_EXTRACT or STRICT_LOW_PART */
            bf->a = (trigger >> (i * 2)) & 0x7;
            bf->b = (trigger >> (i * 3)) & 0x1F;
            bf->c = (trigger >> (i * 4)) & 0xFF;
            bf->d = (trigger >> (i * 1)) & 0xFFFF;
        }
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobbers(volatile int *data) {
    int temp;
    register int reg_var asm("r12") = *data;
    
    /* Inline assembly that forces register allocation */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (temp)
        : "r" (reg_var)
        : "eax", "cc"
    );
    
    /* Use the result in a bit-field to combine patterns */
    struct nested_bitfields nb;
    nb.byte1.low = temp & 0xF;
    nb.byte2.mode = (temp >> 4) & 0x3;
    
    *data = temp + nb.byte1.low;
}

/* Test 3: Mixed-type accesses for SUBREG generation */
void test_mixed_type_access(volatile uint32_t *base, int offset) {
    /* Type punning through union for SUBREG */
    union {
        uint32_t word;
        uint16_t halves[2];
        uint8_t bytes[4];
    } converter;
    
    /* Complex addressing with volatile */
    converter.word = base[offset];
    
    /* Sub-register accesses - should generate SUBREG RTL */
    converter.halves[1] = converter.bytes[0] + converter.bytes[1];
    converter.bytes[2] = converter.halves[0] & 0xFF;
    
    /* Write back through different pointer type */
    uint16_t *short_ptr = (uint16_t *)&base[offset];
    short_ptr[1] = converter.halves[1];  /* SUBREG of MEM */
    
    /* Another SUBREG pattern */
    uint8_t *byte_ptr = (uint8_t *)&base[offset + 1];
    byte_ptr[0] = converter.bytes[2];  /* SUBREG of MEM */
}

/* Test 4: Memory operations with complex addressing for MEM RTL */
void test_mem_operations(int iterations, volatile int seed) {
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent array index prevents optimization */
        int idx = (seed + i * 37) & 0xFF;
        
        /* MEM RTL with complex addressing */
        uint32_t temp = mem_buffer[idx];
        
        /* Bit-field operation on loaded value */
        struct bitfield_struct bf;
        bf.a = (temp >> 5) & 0x7;
        bf.b = (temp >> 10) & 0x1F;
        
        /* Store with different type - SUBREG of MEM */
        uint16_t *ptr = (uint16_t *)&mem_buffer[idx];
        ptr[0] = (bf.a << 5) | bf.b;  /* SUBREG then MEM */
        
        /* Another MEM access pattern */
        byte_buffer[idx * 2] = (temp >> 16) & 0xFF;
        byte_buffer[idx * 2 + 1] = (temp >> 24) & 0xFF;
    }
}

/* Test 5: Combined patterns in loop with volatile control */
void test_combined_patterns(struct bitfield_struct *bf_array, 
                           volatile int *control,
                           int size) {
    for (int i = 0; i < size; i++) {
        volatile int ctrl = control[i];
        
        /* ZERO_EXTRACT/STRICT_LOW_PART from bit-field */
        bf_array[i].c = ctrl & 0xFF;
        
        /* Inline assembly to create register pressure */
        int temp;
        asm volatile (
            "movl %1, %%ebx\n\t"
            "rorl $8, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (temp)
            : "r" (ctrl)
            : "ebx", "cc"
        );
        
        /* SUBREG access to memory */
        uint8_t *byte_ptr = (uint8_t *)&short_buffer[i];
        byte_ptr[0] = temp & 0xFF;  /* SUBREG of MEM */
        byte_ptr[1] = (temp >> 8) & 0xFF;  /* Another SUBREG */
        
        /* MEM access with complex addressing */
        mem_buffer[temp & 0xFF] = bf_array[i].d;
    }
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    /* Use command-line argument for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    volatile int volatile_seed = seed;  /* Prevent constant propagation */
    
    /* Initialize test structures */
    struct bitfield_struct bf_data[10];
    volatile int control_data[10];
    
    for (int i = 0; i < 10; i++) {
        bf_data[i].a = 0;
        bf_data[i].b = 0;
        bf_data[i].c = 0;
        bf_data[i].d = 0;
        control_data[i] = volatile_seed + i * 7919;  /* Prime multiplier */
        mem_buffer[i] = volatile_seed + i * 9973;    /* Another prime */
    }
    
    printf("Starting resource pattern tests...\n");
    printf("Seed value: %d\n", seed);
    
    /* Run all test patterns */
    test_bitfield_ops(&bf_data[0], volatile_seed);
    test_asm_clobbers(&control_data[0]);
    test_mixed_type_access(&mem_buffer[0], volatile_seed & 0xF);
    test_mem_operations(50, volatile_seed);
    test_combined_patterns(bf_data, control_data, 5);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += bf_data[i].a + bf_data[i].b + bf_data[i].c + bf_data[i].d;
        checksum += control_data[i];
        checksum += mem_buffer[i] & 0xFFFF;
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Tests completed.\n");
    
    return 0;
}
