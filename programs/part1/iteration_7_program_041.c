#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint64_t global_64 = 0x123456789ABCDEF0ULL;
volatile uint32_t global_32 = 0xDEADBEEF;
volatile uint16_t global_16[4] = {0x1111, 0x2222, 0x3333, 0x4444};
volatile uint8_t global_8[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Bit-field assignment to specific bits of a register/memory */
    __asm__ volatile (
        /* Write to bits 8-15 of dest (ZERO_EXTRACT of memory) */
        "mov %0, %1\n\t"
        : "=m" (*dest)
        : "r" (src)
        : "memory"
    );
}

/* Another ZERO_EXTRACT variant with explicit bitfield */
static void test_zero_extract_bitfield(volatile uint64_t *dest, uint32_t src) {
    uint64_t temp = *dest;
    /* Extract and modify bits 16-31 */
    __asm__ volatile (
        "bts %0, %1\n\t"  /* Bit test and set - operates on specific bit positions */
        : "+r" (temp)
        : "r" ((src & 0x1F) + 16)  /* Bit position between 16-31 */
        : "cc"
    );
    *dest = temp;
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint32_t *dest, uint32_t src) {
    /* Operation that only affects low part of register */
    __asm__ volatile (
        "add %0, %1\n\t"
        : "=r" (*dest)    /* Output constraint that may generate STRICT_LOW_PART */
        : "r" (src), "0" (*dest)
        : "cc"
    );
}

/* STRICT_LOW_PART with byte operation */
static void test_strict_low_part_byte(volatile uint16_t *dest, uint8_t src) {
    /* Byte addition affecting only low 8 bits */
    __asm__ volatile (
        "addb %b1, %b0\n\t"  /* Byte-sized add */
        : "+r" (*dest)
        : "ri" (src)
        : "cc"
    );
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr, uint16_t value) {
    /* Access memory as different-sized object */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    /* Write to sub-register of memory (short within int) */
    __asm__ volatile (
        "movw %1, %0\n\t"  /* Write 16-bit value to memory */
        : "=m" (*short_ptr)  /* Memory operand for 16-bit access */
        : "r" (value)
        : "memory"
    );
}

/* SUBREG of MEM with offset */
static void test_subreg_mem_offset(volatile uint64_t *long_ptr, uint32_t value) {
    /* Access 32-bit portion of 64-bit memory location */
    volatile uint32_t *int_ptr = (volatile uint32_t *)long_ptr;
    
    __asm__ volatile (
        "movl %1, %0\n\t"
        : "=m" (int_ptr[1])  /* Access second 32-bit word of 64-bit location */
        : "r" (value)
        : "memory"
    );
}

/* Complex pattern combining multiple concepts */
static void test_combined_pattern(volatile uint64_t *dest, uint32_t src1, uint16_t src2) {
    uint64_t temp;
    
    /* Load and modify with potential SUBREG operations */
    __asm__ volatile (
        "movq %1, %0\n\t"
        "addl %2, %k0\n\t"  /* Add to low 32 bits (may create SUBREG) */
        "movw %3, %w0\n\t"  /* Move to low 16 bits (may create STRICT_LOW_PART) */
        : "=&r" (temp)      /* Early clobber to force complex allocation */
        : "m" (*dest), "r" (src1), "r" (src2)
        : "cc"
    );
    
    /* Bitfield extract/store */
    __asm__ volatile (
        "btr $16, %0\n\t"   /* Bit test and reset at position 16 */
        : "+r" (temp)
        :: "cc"
    );
    
    *dest = temp;
}

/* Main driver function */
int main() {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_32, 0x12345678);
    checksum += global_32;
    
    test_zero_extract_bitfield(&global_64, 0xABCD);
    checksum += (global_64 & 0xFFFFFFFF);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_32, 0x1000);
    checksum += global_32;
    
    test_strict_low_part_byte(&global_16[0], 0x42);
    checksum += global_16[0];
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_32, 0x8888);
    checksum += global_32;
    
    test_subreg_mem_offset(&global_64, 0x99999999);
    checksum += (global_64 >> 32);
    
    /* Test combined pattern */
    test_combined_pattern(&global_64, 0x11111111, 0x2222);
    checksum += (global_64 & 0xFFFFFFFF);
    
    /* Use all global variables to prevent dead code elimination */
    for (int i = 0; i < 4; i++) {
        checksum += global_16[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += global_8[i];
    }
    
    printf("Final checksum: %u\n", checksum);
    printf("(This output ensures code isn't eliminated by optimizer)\n");
    
    return (int)(checksum & 0xFF);
}
