/* test_resource_cc.c
 * 
 * This program is designed to trigger the uncovered lines 282-290 in resource.cc
 * by generating specific RTL patterns during GCC compilation.
 * 
 * The uncovered code handles:
 * 1. ZERO_EXTRACT and STRICT_LOW_PART patterns (from bit-field assignments)
 * 2. SUBREG patterns (from sub-register accesses)
 * 3. MEM patterns (from memory operations)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* ========== BIT-FIELD STRUCTURES ========== */
/* These generate ZERO_EXTRACT/STRICT_LOW_PART RTL */

struct bitfield_packed {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

struct bitfield_mixed {
    volatile unsigned int low : 4;
    unsigned int high : 12;
    unsigned int full : 16;
};

/* ========== VOLATILE MEMORY BUFFERS ========== */
/* These generate MEM RTL with volatile semantics */

volatile uint32_t mem_buffer[64];
volatile uint16_t short_buffer[128];
volatile uint8_t byte_buffer[256];

/* ========== UNION FOR TYPE PUNNING ========== */
/* Generates SUBREG RTL through mixed-type accesses */

union type_punner {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* ========== TEST FUNCTIONS ========== */

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_packed *bp, struct bitfield_mixed *bm, 
                       volatile int control) {
    for (int i = 0; i < control % 8; i++) {
        /* These assignments often generate STRICT_LOW_PART or ZERO_EXTRACT */
        bp->a = (i & 0x7);                    /* 3-bit field */
        bp->b = (i * 3) & 0x1F;               /* 5-bit field */
        bp->c = (i * 5) & 0xFF;               /* 8-bit field */
        bp->d = (i * 7) & 0xFFFF;             /* 16-bit field */
        
        /* Volatile bit-field with mixed access */
        bm->low = (i & 0xF);                  /* Volatile 4-bit field */
        bm->high = (i * 11) & 0xFFF;          /* 12-bit field */
        bm->full = bm->low | (bm->high << 4); /* Full field access */
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(volatile int *input, int *output) {
    int temp1, temp2;
    
    /* Force register allocation conflicts */
    asm volatile (
        "mov %[in], %[t1]\n\t"
        "add $1, %[t1]\n\t"
        "mov %[t1], %[t2]\n\t"
        "and $0xFF, %[t2]"
        : [t1] "=r" (temp1), [t2] "=r" (temp2)
        : [in] "r" (*input)
        : "cc" /* Clobber condition codes to force resource tracking */
    );
    
    /* Use the results in bit-field operations */
    struct bitfield_mixed bm;
    bm.low = temp1 & 0xF;      /* May generate STRICT_LOW_PART */
    bm.high = temp2 & 0xFFF;
    
    *output = bm.full;
}

/* Test 3: Sub-register accesses via unions and casts (SUBREG generation) */
void test_subreg_access(volatile int index) {
    union type_punner pun;
    volatile uint32_t *mem_ptr = &mem_buffer[0];
    
    for (int i = 0; i < (index % 16); i++) {
        /* Whole word access (MEM) */
        pun.word = mem_buffer[i];
        
        /* Sub-word accesses (may generate SUBREG) */
        short_buffer[i*2] = pun.parts.low;     /* 16-bit access to 32-bit */
        short_buffer[i*2+1] = pun.parts.high;
        
        /* Byte access via cast (SUBREG of MEM) */
        uint8_t *byte_ptr = (uint8_t *)&mem_buffer[i];
        byte_buffer[i*4] = byte_ptr[0];
        byte_buffer[i*4+1] = byte_ptr[1];      /* SUBREG likely here */
        
        /* Pointer arithmetic with different types */
        *(volatile uint16_t *)((uint8_t *)&mem_buffer[i] + 1) = 
            pun.half[0] + pun.half[1];
    }
}

/* Test 4: Complex memory addressing modes (MEM_P patterns) */
void test_complex_mem(volatile int start, volatile int stride) {
    /* Complex addressing with multiple components */
    for (int i = 0; i < 8; i++) {
        int idx = start + i * stride;
        
        /* MEM with index, scale, and offset */
        mem_buffer[idx % 64] = 
            short_buffer[(idx * 2) % 128] + 
            byte_buffer[(idx * 4) % 256];
        
        /* Pointer with multiple levels of indirection */
        volatile uint32_t **ptr_ptr = (volatile uint32_t **)&mem_buffer[0];
        volatile uint32_t *indirect = ptr_ptr[i % 8];
        if (indirect) {
            *indirect = i * 0x12345678;
        }
    }
}

/* Test 5: Register variables with bit-field manipulation */
void test_register_var(volatile int seed) {
    /* Register variable tied to specific register */
    register int reg_var asm("r12") = seed;
    
    struct bitfield_packed bp;
    
    /* Force the register variable into bit-field operations */
    for (int i = 0; i < 4; i++) {
        /* This may create conflicts requiring STRICT_LOW_PART */
        bp.a = (reg_var >> (i * 3)) & 0x7;
        bp.b = (reg_var >> (i * 5)) & 0x1F;
        
        /* Modify register variable */
        asm volatile ("" : "+r" (reg_var)); /* Pretend to modify */
        
        /* Use in memory operation */
        mem_buffer[i] = bp.c | (bp.d << 8);
    }
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    /* Use command-line argument for runtime control */
    volatile int control = (argc > 1) ? atoi(argv[1]) : 5;
    
    /* Initialize test structures */
    struct bitfield_packed bp = {0};
    struct bitfield_mixed bm = {0};
    
    /* Initialize memory buffers */
    for (int i = 0; i < 64; i++) mem_buffer[i] = i;
    for (int i = 0; i < 128; i++) short_buffer[i] = i & 0xFFFF;
    for (int i = 0; i < 256; i++) byte_buffer[i] = i & 0xFF;
    
    printf("Starting resource.cc coverage tests...\n");
    printf("Control value: %d\n", control);
    
    /* Run all tests to generate various RTL patterns */
    test_bitfield_ops(&bp, &bm, control);
    
    int asm_result;
    test_asm_clobber(&control, &asm_result);
    
    test_subreg_access(control);
    test_complex_mem(control, 3);
    test_register_var(control);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 64; i++) checksum ^= mem_buffer[i];
    for (int i = 0; i < 128; i++) checksum ^= short_buffer[i];
    for (int i = 0; i < 256; i++) checksum ^= byte_buffer[i];
    
    checksum ^= bp.a ^ bp.b ^ bp.c ^ bp.d;
    checksum ^= bm.low ^ bm.high ^ bm.full;
    checksum ^= asm_result;
    
    printf("Final checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    return (checksum == 0) ? 0 : 1;
}
