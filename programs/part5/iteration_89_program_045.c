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

/* ==================== Bit-field structures ==================== */

/* Struct with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG accesses */
union mixed_access {
    uint32_t full;
    uint16_t half[2];
    uint8_t  byte[4];
};

/* ==================== Test functions ==================== */

/* Test 1: Bit-field assignments in a loop with data-dependent index.
 * Should generate ZERO_EXTRACT or STRICT_LOW_PART in SET_DEST.
 */
void test_bitfield_assignments(void) {
    struct bitfield_struct bf[4];
    int i;
    
    /* Initialize with volatile to prevent compile-time optimization */
    for (i = 0; i < 4; i++) {
        bf[i].a = (g_volatile_input + i) & 0x7;
        bf[i].b = (g_volatile_input * i) & 0x1F;
        bf[i].c = (g_volatile_input + i * 2) & 0xFF;
        bf[i].d = (g_volatile_input + i * 100) & 0xFFFF;
    }
    
    /* Data-dependent updates to bit-fields */
    for (i = 0; i < 4; i++) {
        if (i % 2 == 0) {
            bf[i].a = bf[i].b & 0x3;           /* ZERO_EXTRACT likely */
            bf[i].c = bf[i].d >> 8;            /* Another bit-field op */
        } else {
            bf[i].b = bf[i].a | 0x10;          /* More bit-field writes */
        }
    }
    
    /* Compute checksum */
    unsigned int sum = 0;
    for (i = 0; i < 4; i++) {
        sum += bf[i].a + bf[i].b + bf[i].c + bf[i].d;
    }
    printf("Bit-field checksum: %u\n", sum);
}

/* Test 2: Inline assembly with clobbers to stress reload pass.
 * Uses register variables tied to specific hard registers.
 */
void test_inline_asm_clobbers(void) {
    register uint32_t reg_var asm ("r12") = g_volatile_input;
    struct bitfield_struct bf;
    
    /* Initialize bit-field */
    bf.a = 2;
    bf.b = 7;
    bf.c = 100;
    bf.d = 50000;
    
    /* Inline asm that clobbers hard registers, forcing reload to manage resources */
    asm volatile (
        "mov %[reg], %[val]\n\t"
        "add %[reg], #1\n\t"
        : [reg] "+r" (reg_var)
        : [val] "r" (bf.d)
        : "cc", "memory"
    );
    
    /* Use the register variable in a bit-field assignment */
    bf.c = (reg_var >> 4) & 0xFF;  /* Could generate STRICT_LOW_PART */
    
    printf("Inline asm result: bf.c = %u, reg_var = %u\n", bf.c, reg_var);
}

/* Test 3: Volatile memory accesses with type-punning via union.
 * Should generate SUBREG of MEM RTL patterns.
 */
void test_volatile_mem_subreg(void) {
    volatile union mixed_access mem;
    int i;
    
    mem.full = 0x12345678;
    
    /* Access sub-parts via different types (SUBREG generation) */
    for (i = 0; i < 2; i++) {
        mem.half[i] = (uint16_t)(g_volatile_input + i * 0x100);
    }
    
    /* Byte access through volatile pointer (another SUBREG possibility) */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)&mem;
    for (i = 0; i < 4; i++) {
        byte_ptr[i] = (uint8_t)(g_volatile_input + i * 0x10);
    }
    
    printf("Memory union value: 0x%08x\n", mem.full);
}

/* Test 4: Complex addressing mode with volatile pointer.
 * Should generate MEM RTL with non-trivial address expression.
 */
void test_complex_addressing(void) {
    volatile uint32_t buffer[16];
    int i, idx;
    
    /* Initialize buffer */
    for (i = 0; i < 16; i++) {
        buffer[i] = i * 100;
    }
    
    /* Data-dependent index prevents optimization */
    idx = g_volatile_input % 16;
    
    /* Write with complex addressing: base + scaled index */
    buffer[idx * 2] = buffer[idx] + buffer[idx + 1];
    
    /* Pointer arithmetic with volatile */
    volatile uint32_t *ptr = &buffer[0];
    ptr += idx;
    *ptr = 0xDEADBEEF;  /* MEM write */
    
    /* Compute checksum */
    uint32_t sum = 0;
    for (i = 0; i < 16; i++) {
        sum += buffer[i];
    }
    printf("Complex addressing checksum: %u\n", sum);
}

/* Test 5: Mixed-size accesses via casts (SUBREG of MEM) */
void test_mixed_size_access(void) {
    volatile uint32_t data = 0x87654321;
    volatile uint16_t *short_ptr = (volatile uint16_t *)&data;
    volatile uint8_t *char_ptr = (volatile uint8_t *)&data;
    
    /* Write a short to the first half (SUBREG of MEM) */
    *short_ptr = (uint16_t)g_volatile_input;
    
    /* Write a char to the third byte (another SUBREG) */
    char_ptr[2] = (uint8_t)(g_volatile_input >> 8);
    
    printf("Mixed-size result: 0x%08x\n", data);
}

/* ==================== Main driver ==================== */
int main(int argc, char **argv) {
    /* Use command-line argument to vary behavior at runtime */
    if (argc > 1) {
        g_volatile_input = atoi(argv[1]);
    } else {
        g_volatile_input = 42;  /* Default seed */
    }
    
    printf("Testing with volatile input = %d\n", g_volatile_input);
    
    /* Run all tests to expose different RTL patterns */
    test_bitfield_assignments();
    test_inline_asm_clobbers();
    test_volatile_mem_subreg();
    test_complex_addressing();
    test_mixed_size_access();
    
    return 0;
}
