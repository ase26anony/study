/* test_resource_cc.c
 * 
 * This program is designed to trigger the uncovered lines in resource.cc
 * (lines 282-290) by generating specific RTL patterns during compilation.
 * It focuses on bit-field assignments, sub-register memory operations,
 * and volatile memory accesses to produce ZERO_EXTRACT, STRICT_LOW_PART,
 * SUBREG, and MEM RTL expressions.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Use volatile input to prevent constant propagation */
static volatile int g_volatile_input = 0;

/* ========== Bit-field structures ========== */

/* Bit-field struct that may generate ZERO_EXTRACT/STRICT_LOW_PART */
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

/* ========== Union for type-punning ========== */

union type_punner {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* ========== Test functions ========== */

/* Function 1: Bit-field assignments
 * Should generate ZERO_EXTRACT or STRICT_LOW_PART in SET_DEST */
void test_bitfield_assignments(void) {
    struct bitfield_struct bf = {0};
    struct packed_bitfield pbf = {0};
    
    /* Use volatile input to make assignments data-dependent */
    int idx = g_volatile_input & 0x7;
    
    /* Multiple bit-field writes - may generate STRICT_LOW_PART */
    bf.a = idx;
    bf.b = idx + 1;
    bf.c = idx + 2;
    bf.d = idx + 3;
    
    /* Packed bit-field with volatile source */
    volatile unsigned char src = g_volatile_input;
    pbf.x = src & 0x3;
    pbf.y = (src >> 2) & 0x7;
    pbf.z = (src >> 5) & 0x7;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(bf), "r"(pbf) : "memory");
}

/* Function 2: Inline assembly with register variables
 * Forces reload pass to handle hard register clobbers */
void test_asm_register_clobber(void) {
    /* Register variable tied to specific register */
    register uint32_t reg_var asm("r12") = g_volatile_input;
    
    /* Bit-field within register context */
    struct {
        uint32_t low : 12;
        uint32_t high : 20;
    } bit_reg;
    
    /* Inline assembly that clobbers the register */
    asm volatile (
        "mov %[reg], %[val]\n\t"
        "and %[reg], %[mask]\n\t"
        : [reg] "+r" (reg_var)
        : [val] "r" (g_volatile_input + 0x100),
          [mask] "r" (0x0FFF)
        : "cc"
    );
    
    /* Assignment that may use STRICT_LOW_PART */
    bit_reg.low = reg_var & 0xFFF;
    bit_reg.high = (reg_var >> 12) & 0xFFFFF;
    
    /* Use the result */
    asm volatile("" : : "r"(bit_reg) : "memory");
}

/* Function 3: Sub-register memory accesses
 * Should generate SUBREG of MEM RTL patterns */
void test_subreg_mem_access(void) {
    union type_punner mem_area;
    volatile uint8_t *volatile_ptr;
    
    /* Initialize with volatile input */
    mem_area.word = g_volatile_input;
    
    /* Type-punning through different sized accesses */
    volatile_ptr = (volatile uint8_t *)&mem_area;
    
    /* Write through byte pointer (may generate SUBREG) */
    volatile_ptr[1] = (g_volatile_input >> 8) & 0xFF;
    
    /* Write half-word through cast (may generate SUBREG) */
    *((volatile uint16_t *)&mem_area.half[0]) = 
        (uint16_t)(g_volatile_input & 0xFFFF);
    
    /* Complex addressing mode */
    int offset = g_volatile_input & 0x3;
    volatile_ptr[offset] = offset * 0x55;
    
    /* Ensure memory is referenced */
    asm volatile("" : : "m"(mem_area) : "memory");
}

/* Function 4: Volatile memory operations in loop
 * Generates MEM RTL patterns with complex addressing */
void test_volatile_mem_loop(void) {
    volatile uint32_t buffer[16];
    volatile uint16_t *short_ptr;
    volatile uint8_t *byte_ptr;
    
    /* Initialize buffer */
    for (int i = 0; i < 16; i++) {
        buffer[i] = i + g_volatile_input;
    }
    
    /* Mixed-type volatile accesses in loop */
    for (int i = 0; i < 8; i++) {
        /* Access as different types - may generate SUBREG */
        short_ptr = (volatile uint16_t *)&buffer[i];
        byte_ptr = (volatile uint8_t *)&buffer[i];
        
        /* Write through different sized views */
        short_ptr[0] = (short_ptr[0] & 0xFF00) | (byte_ptr[1]);
        byte_ptr[2] = (i * 17) & 0xFF;
        
        /* Data-dependent index */
        int idx = (g_volatile_input + i) & 0xF;
        buffer[idx] = buffer[idx] ^ 0x00FF00FF;
    }
    
    /* Force memory barrier */
    asm volatile("" : : "m"(buffer[0]), "m"(buffer[15]) : "memory");
}

/* Function 5: Combined test with all patterns
 * Maximizes chance of hitting uncovered lines */
void test_combined_patterns(void) {
    /* Local struct with bit-fields */
    struct {
        unsigned int field1 : 4;
        unsigned int field2 : 12;
        unsigned int field3 : 16;
    } local_bf;
    
    /* Volatile memory region */
    volatile uint32_t mem[4];
    volatile uint8_t *byte_view = (volatile uint8_t *)mem;
    
    /* Initialize with data-dependent values */
    int base = g_volatile_input;
    
    /* Bit-field assignment (ZERO_EXTRACT/STRICT_LOW_PART) */
    local_bf.field1 = base & 0xF;
    local_bf.field2 = (base >> 4) & 0xFFF;
    local_bf.field3 = (base >> 16) & 0xFFFF;
    
    /* Memory write with complex addressing (MEM) */
    for (int i = 0; i < 4; i++) {
        mem[i] = base + i * 0x100;
    }
    
    /* Sub-register access (SUBREG) */
    byte_view[5] = local_bf.field1;
    *((volatile uint16_t *)&byte_view[8]) = local_bf.field2;
    
    /* Inline assembly that may force reload */
    register uint32_t tmp asm("r11") = mem[0];
    asm volatile (
        "ror %[tmp], #8\n\t"
        : [tmp] "+r" (tmp)
        :
        : "cc"
    );
    
    /* Use all results to prevent optimization */
    asm volatile("" 
        : 
        : "r"(local_bf), "m"(mem[0]), "m"(mem[3]), "r"(tmp)
        : "memory");
}

/* ========== Main function ========== */

int main(int argc, char *argv[]) {
    /* Use command-line argument for runtime variability */
    if (argc > 1) {
        g_volatile_input = atoi(argv[1]);
    } else {
        g_volatile_input = 0x12345678;
    }
    
    printf("Testing resource.cc uncovered lines with input: %d (0x%08x)\n",
           g_volatile_input, g_volatile_input);
    
    /* Execute all test patterns */
    test_bitfield_assignments();
    printf("  test_bitfield_assignments completed\n");
    
    test_asm_register_clobber();
    printf("  test_asm_register_clobber completed\n");
    
    test_subreg_mem_access();
    printf("  test_subreg_mem_access completed\n");
    
    test_volatile_mem_loop();
    printf("  test_volatile_mem_loop completed\n");
    
    test_combined_patterns();
    printf("  test_combined_patterns completed\n");
    
    printf("All tests completed successfully.\n");
    return 0;
}
