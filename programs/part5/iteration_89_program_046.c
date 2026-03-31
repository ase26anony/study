/* test_resource_cc.c
 * 
 * This program is designed to trigger the uncovered lines in resource.cc
 * (lines 282-290) by generating specific RTL patterns during compilation.
 * The patterns target SET_DEST expressions that are ZERO_EXTRACT,
 * STRICT_LOW_PART, SUBREG, or MEM.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* ==================== Bit-field Structures ==================== */

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
    uint8_t  byte[4];
};

/* ==================== Test Functions ==================== */

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, volatile int trigger) {
    for (int i = 0; i < 10; i++) {
        /* Data-dependent assignments prevent constant propagation */
        if (trigger & (1 << i)) {
            bf->a = (i & 0xF);          /* 4-bit field */
            bf->b = (i * 7) & 0xFF;     /* 8-bit field */
            bf->c = (i * 13) & 0xFFF;   /* 12-bit field */
        } else {
            bf->d = (i * 3) & 0xFF;     /* 8-bit field */
        }
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(volatile int *mem) {
    register int reg_var asm ("r12") = *mem;  /* Suggest register r12 */
    int tmp;
    
    /* Inline asm that clobbers hard registers, forcing reload to manage resources */
    asm volatile (
        "movl %1, %%r12d\n\t"
        "addl $1, %%r12d\n\t"
        "movl %%r12d, %0\n\t"
        : "=r" (tmp)
        : "r" (reg_var)
        : "r12"  /* Clobber r12 */
    );
    
    *mem = tmp;
}

/* Test 3: Mixed-type accesses via union to generate SUBREG */
void test_mixed_type_access(union mixed_access *u, volatile int idx) {
    /* Access different sized members based on runtime index */
    switch (idx & 3) {
        case 0: u->word = 0xDEADBEEF; break;
        case 1: u->half[0] = 0x1234; break;   /* SUBREG of MEM likely */
        case 2: u->byte[1] = 0xAB; break;     /* Smaller SUBREG */
        case 3: u->half[1] = u->half[0]; break; /* SUBREG load/store */
    }
}

/* Test 4: Volatile pointer dereferences to generate MEM RTL */
void test_volatile_mem(volatile uint32_t *mem, int size, volatile int step) {
    for (int i = 0; i < size; i += step) {
        /* Complex addressing with volatile ensures MEM_P(x) */
        *(mem + i) = *(mem + i + 1) + 1;
    }
}

/* Test 5: Pointer casting for sub-word memory operations */
void test_ptr_cast(volatile char *buf, int offset) {
    /* Cast to different pointer types to generate SUBREG of MEM */
    uint16_t *ptr16 = (uint16_t *)(buf + offset);
    *ptr16 = 0x55AA;  /* Likely SUBREG store */
    
    /* Another volatile MEM access */
    volatile uint32_t *ptr32 = (volatile uint32_t *)(buf + offset + 2);
    *ptr32 = *ptr32 ^ 0xFF00FF00;
}

/* ==================== Main Driver ==================== */
int main(int argc, char *argv[]) {
    /* Use command-line argument for runtime variability */
    volatile int trigger = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize test data */
    struct bitfield_struct bf = {0};
    union mixed_access u = {0};
    
    /* Volatile memory buffer */
    volatile uint32_t mem_buffer[32];
    for (int i = 0; i < 32; i++) {
        mem_buffer[i] = i * 3;
    }
    
    /* Volatile char buffer for pointer casting test */
    volatile char char_buf[64];
    for (int i = 0; i < 64; i++) {
        char_buf[i] = i;
    }
    
    /* Run all tests to generate diverse RTL patterns */
    test_bitfield_ops(&bf, trigger);
    test_asm_clobber((volatile int *)&mem_buffer[0]);
    test_mixed_type_access(&u, trigger);
    test_volatile_mem(mem_buffer, 32, (trigger % 5) + 1);
    test_ptr_cast(char_buf, trigger % 60);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    checksum += bf.a + bf.b + bf.c + bf.d;
    checksum += u.word;
    for (int i = 0; i < 32; i++) {
        checksum ^= mem_buffer[i];
    }
    for (int i = 0; i < 64; i++) {
        checksum += char_buf[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    return 0;
}
