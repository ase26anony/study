/* test_resource_cc.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc (lines 282-290) by generating RTL patterns that involve:
 *   - ZERO_EXTRACT / STRICT_LOW_PART (via bit-field assignments)
 *   - SUBREG of MEM (via subword memory operations)
 *   - MEM with complex addressing (via volatile pointer accesses)
 * 
 * Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing test_resource_cc.c -o test_resource
 * 
 * The -O2 enables reload/reorg passes where mark_set_resources is called.
 * Use -fdump-rtl-reload to see the generated RTL.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* ========== Bit-field structures for ZERO_EXTRACT/STRICT_LOW_PART ========== */

struct bitfield_packed {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

struct bitfield_mixed {
    volatile unsigned int low : 4;
    unsigned int high : 12;
    unsigned int fill : 16;
};

/* ========== Union for type-punning (SUBREG generation) ========== */

union punner {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* ========== Global volatile memory for MEM RTL ========== */

volatile uint32_t global_mem[256];
volatile uint8_t global_bytes[1024];

/* ========== Test functions ========== */

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfields(int seed) {
    struct bitfield_packed bp1 = {0};
    struct bitfield_packed bp2 = {0};
    struct bitfield_mixed bm = {0};
    
    /* Loop with data-dependent index to prevent constant propagation */
    for (int i = 0; i < (seed & 0xF); i++) {
        /* These assignments may lower to ZERO_EXTRACT or STRICT_LOW_PART */
        bp1.a = (seed + i) & 0x7;
        bp1.b = (seed * i) & 0x1F;
        bp1.c = (seed ^ i) & 0xFF;
        
        /* Chain assignments to create dependencies */
        bp2.a = bp1.b & 0x7;
        bp2.b = bp1.c & 0x1F;
        bp2.c = bp1.a & 0xFF;
        
        /* Mixed volatile/non-volatile bit-field */
        bm.low = (bp1.a + bp2.b) & 0xF;
        bm.high = (bp1.c ^ bp2.c) & 0xFFF;
    }
    
    /* Use results to prevent dead code elimination */
    global_mem[0] = bp1.c | (bp2.b << 8);
    global_mem[1] = bm.high;
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(int seed) {
    register uint32_t reg_var asm("r12") = seed * 0x12345678;
    uint32_t temp;
    
    /* Inline asm that clobbers hard registers */
    asm volatile (
        "mov %[reg], %[temp]\n\t"
        "ror $8, %[temp]\n\t"
        "mov %[temp], %[reg]"
        : [temp] "=r" (temp), [reg] "+r" (reg_var)
        :
        : "cc"
    );
    
    /* Use the register variable with bit-field-like operation */
    struct bitfield_mixed bm;
    bm.low = reg_var & 0xF;      /* May generate STRICT_LOW_PART */
    bm.high = (reg_var >> 4) & 0xFFF;
    
    global_mem[2] = bm.high | (bm.low << 16);
}

/* Test 3: SUBREG of MEM via type-punning and volatile */
void test_subreg_mem(int seed) {
    union punner pu;
    volatile uint32_t *volatile_mem = &global_mem[16];
    
    /* Initialize with seed */
    pu.word = seed * 0xABCDEF01;
    
    /* Write subword parts to volatile memory - may generate SUBREG(MEM) */
    for (int i = 0; i < 8; i++) {
        /* These accesses often become SUBREG of MEM in RTL */
        global_bytes[i*2] = pu.byte[i % 4];
        global_bytes[i*2 + 1] = pu.half[i % 2] & 0xFF;
        
        /* Volatile pointer with offset - complex addressing mode */
        *(volatile_mem + i) = pu.half[i % 2] | (pu.byte[(i+1)%4] << 16);
    }
    
    /* Pointer casting for SUBREG generation */
    uint16_t *short_ptr = (uint16_t *)&global_mem[32];
    for (int i = 0; i < 4; i++) {
        short_ptr[i] = pu.half[i % 2] + i;  /* MEM:SI -> SUBREG:HI */
    }
}

/* Test 4: Complex MEM addressing with volatile */
void test_mem_addressing(int seed) {
    volatile uint32_t *ptr = &global_mem[64];
    volatile uint8_t *byte_ptr = global_bytes + 128;
    
    /* Loop with data-dependent addressing */
    for (int i = 0; i < (seed & 0x3F); i++) {
        int idx = (i * seed) & 0xFF;
        
        /* Complex addressing: base + index + offset */
        ptr[idx % 64] = seed + i;
        
        /* Volatile byte write - ensures MEM RTL */
        byte_ptr[(idx * 3) % 256] = i & 0xFF;
        
        /* Pointer arithmetic with different types */
        *(volatile uint16_t *)(byte_ptr + idx) = (seed ^ idx) & 0xFFFF;
    }
}

/* Test 5: Mixed operations in a loop to create various RTL patterns */
void test_mixed_operations(int seed) {
    struct bitfield_packed bp = {0};
    union punner pu;
    volatile uint32_t accum = 0;
    
    pu.word = seed;
    
    for (int i = 0; i < 16; i++) {
        /* Bit-field assignment (ZERO_EXTRACT/STRICT_LOW_PART) */
        bp.a = (pu.byte[i % 4] & 0x7);
        bp.b = (pu.half[i % 2] & 0x1F);
        
        /* SUBREG of MEM via union access */
        uint8_t temp_byte = pu.byte[(i + 1) % 4];
        global_bytes[i + 64] = temp_byte;
        
        /* MEM with volatile */
        accum += global_mem[i % 32];
        
        /* Update union with data-dependent values */
        pu.word = (pu.word * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Store result to prevent optimization */
    global_mem[127] = accum | bp.c;
}

/* ========== Main function ========== */

int main(int argc, char *argv[]) {
    /* Use command-line argument or default seed for runtime variability */
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
        if (seed == 0) seed = 12345;
    }
    
    printf("Testing resource.cc coverage with seed=%d\n", seed);
    
    /* Initialize global memory */
    for (int i = 0; i < 256; i++) {
        global_mem[i] = i * 0x01010101;
    }
    for (int i = 0; i < 1024; i++) {
        global_bytes[i] = i & 0xFF;
    }
    
    /* Run all tests */
    test_bitfields(seed);
    test_asm_clobber(seed + 1);
    test_subreg_mem(seed + 2);
    test_mem_addressing(seed + 3);
    test_mixed_operations(seed + 4);
    
    /* Compute checksum to verify execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= global_mem[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    
    return 0;
}
