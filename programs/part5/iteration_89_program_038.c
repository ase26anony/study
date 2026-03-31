/* Compile with: gcc -O2 -fdump-rtl-all -fdump-tree-all -o coverage_test coverage_test.c */
/* Alternative: gcc -O3 -fno-strict-aliasing -frename-registers -o coverage_test coverage_test.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_volatile_input = 0;

/* ==================== BIT-FIELD STRUCTURES ==================== */
/* These should generate ZERO_EXTRACT/STRICT_LOW_PART RTL patterns */

struct bitfield_packed {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

struct bitfield_mixed {
    volatile unsigned int low : 4;   /* volatile forces memory access */
    unsigned int high : 12;
    unsigned int pad : 16;
};

/* ==================== TEST FUNCTIONS ==================== */

/* Test 1: Bit-field assignments that may generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_packed *bp, struct bitfield_mixed *bm, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Complex bit-field assignment with runtime-dependent values */
        bp->a = (i & 0x7);                    /* 3-bit field */
        bp->b = ((i + g_volatile_input) & 0x1F); /* 5-bit field */
        bp->c = ((i * 3) & 0xFF);             /* 8-bit field */
        bp->d = ((i << 8) & 0xFFFF);          /* 16-bit field */
        
        /* Mixed volatile/non-volatile bit-field access */
        bm->low = (i & 0xF);                  /* volatile bit-field */
        bm->high = ((i + bp->a) & 0xFFF);
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobbers(void) {
    int input = g_volatile_input;
    int output1, output2;
    
    /* Register variable tied to specific register */
    register int reg_var asm ("r12") = input;
    
    /* Inline asm that clobbers registers, forcing reload to manage resources */
    asm volatile (
        "mov %[in], %[out1]\n\t"
        "add $1, %[out1]\n\t"
        "mov %[out1], %[out2]\n\t"
        : [out1] "=r" (output1), [out2] "=r" (output2)
        : [in] "r" (reg_var)
        : "r11", "r13"  /* Clobber registers to stress reload */
    );
    
    /* Use the outputs to prevent dead code elimination */
    ((volatile int*)&g_volatile_input)[0] = output1 + output2;
}

/* Test 3: SUBREG and MEM patterns through type-punning and volatile */
void test_subreg_mem(void) {
    volatile uint32_t mem_buffer[4] = {0, 0, 0, 0};
    volatile uint16_t *half_ptr;
    volatile uint8_t *byte_ptr;
    
    /* Type-punning through unions for SUBREG generation */
    union {
        uint32_t word;
        uint16_t half[2];
        uint8_t byte[4];
    } punner;
    
    /* Initialize with volatile input */
    punner.word = g_volatile_input;
    
    /* Access sub-parts (should generate SUBREG RTL) */
    punner.half[1] = punner.half[0] + 1;  /* SUBREG of REG */
    punner.byte[3] = punner.byte[1] * 2;
    
    /* Write to volatile memory with different sized accesses */
    half_ptr = (volatile uint16_t*)&mem_buffer[0];
    byte_ptr = (volatile uint8_t*)&mem_buffer[1];
    
    /* Mixed-size memory accesses (MEM with SUBREG) */
    for (int i = 0; i < 3; i++) {
        half_ptr[i] = (uint16_t)(punner.half[0] + i);  /* MEM:HI */
        byte_ptr[i] = (uint8_t)(punner.byte[i] ^ 0x55); /* MEM:QI */
    }
    
    /* Complex addressing mode for MEM */
    uint32_t idx = g_volatile_input & 0x3;
    mem_buffer[idx] = punner.word + mem_buffer[idx ^ 1];
}

/* Test 4: STRICT_LOW_PART through register variables and bit-fields */
void test_strict_low_part(void) {
    /* Register variable combined with bit-field manipulation */
    register uint32_t reg asm ("r10") = g_volatile_input;
    struct bitfield_mixed local_bm;
    
    /* Force partial register update */
    for (int i = 0; i < 4; i++) {
        /* This may generate STRICT_LOW_PART for bit-field in register */
        local_bm.low = (reg >> (i * 2)) & 0xF;
        local_bm.high = (reg >> (i * 2 + 4)) & 0xFFF;
        
        /* Use inline asm to ensure register is live */
        asm volatile ("" : "+r" (reg) : : "memory");
        
        /* Update register based on bit-field */
        reg = (reg << 1) | (local_bm.low & 0x1);
    }
    
    /* Store result to prevent elimination */
    ((volatile uint32_t*)&g_volatile_input)[0] = reg;
}

/* ==================== MAIN EXECUTION ==================== */
int main(int argc, char *argv[]) {
    int iterations = 10;
    
    /* Use command-line argument for runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    
    /* Initialize test structures */
    struct bitfield_packed bp = {0, 0, 0, 0};
    struct bitfield_mixed bm = {0, 0, 0};
    
    printf("Starting coverage test with %d iterations...\n", iterations);
    
    /* Run all tests to trigger different RTL patterns */
    test_bitfield_ops(&bp, &bm, iterations);
    test_asm_clobbers();
    test_subreg_mem();
    test_strict_low_part();
    
    /* Compute checksum of modified data */
    uint32_t checksum = 0;
    checksum += bp.a + bp.b + bp.c + bp.d;
    checksum += bm.low + bm.high;
    checksum += g_volatile_input;
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
