#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to ensure side effects are preserved */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_bytes[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Target: SET_DEST with ZERO_EXTRACT wrapping MEM */
    __asm__ volatile (
        /* Write to specific bits (bits 8-15) of memory location */
        "btsl %1, %0\n\t"
        : "+m" (*dest)
        : "r" (src & 0xFF)
        : "cc", "memory"
    );
}

/* Another ZERO_EXTRACT variant with bitfield in register */
static void test_zero_extract_reg(volatile uint32_t *dest) {
    uint32_t temp;
    __asm__ volatile (
        /* Extract bits 4-11 from memory, modify, write back */
        "movl %1, %0\n\t"
        "andl $0xFF0, %0\n\t"
        "orl $0x550, %0\n\t"
        : "=r" (temp)
        : "m" (*dest)
        : "cc"
    );
    *dest = temp;
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Target: SET_DEST with STRICT_LOW_PART */
    uint32_t temp;
    __asm__ volatile (
        /* Operation that only affects low 16 bits */
        "addw %w2, %w0\n\t"
        : "=r" (temp)
        : "0" (0), "r" (src)
        : "cc"
    );
    *dest = (uint16_t)temp;
}

/* STRICT_LOW_PART with memory operand */
static void test_strict_low_part_mem(volatile uint32_t *dest) {
    __asm__ volatile (
        /* Only modify low 16 bits of memory location */
        "addw $0x1234, %0\n\t"
        : "+m" (*((volatile uint16_t *)dest))
        :
        : "cc", "memory"
    );
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint64_t *dest) {
    /* Target: SET_DEST with SUBREG wrapping MEM */
    uint16_t temp = 0xABCD;
    
    __asm__ volatile (
        /* Write 16-bit value to first part of 64-bit memory */
        "movw %w1, %0\n\t"
        : "=m" (*((volatile uint16_t *)dest))
        : "r" (temp)
        : "memory"
    );
}

/* SUBREG with different size access */
static void test_subreg_mem_mixed(volatile uint32_t *array) {
    uint8_t byte_val = 0x42;
    
    __asm__ volatile (
        /* Access byte within 32-bit memory location */
        "movb %b1, %0\n\t"
        : "=m" (*((volatile uint8_t *)&array[1]))
        : "r" (byte_val)
        : "memory"
    );
}

/* Complex case with early-clobber to force SUBREG patterns */
static void test_early_clobber_subreg(volatile uint32_t *dest, uint32_t src1, uint32_t src2) {
    uint16_t result;
    
    __asm__ volatile (
        /* Early clobber on 16-bit output from 32-bit operations */
        "movl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "movw %%ax, %0\n\t"
        : "=&m" (*((volatile uint16_t *)dest))
        : "0" (0), "r" (src1), "r" (src2)
        : "%eax", "cc", "memory"
    );
}

/* Combined pattern that might trigger multiple paths */
static void test_combined_pattern(volatile uint64_t *mem) {
    uint32_t temp32;
    uint16_t temp16;
    
    /* First create a SUBREG access */
    __asm__ volatile (
        "movw $0xDEAD, %0\n\t"
        : "=m" (*((volatile uint16_t *)mem))
        :
        : "memory"
    );
    
    /* Then a ZERO_EXTRACT-like operation */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0xFFFF0000, %%eax\n\t"
        "orl $0x0000BEEF, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (*((volatile uint32_t *)mem + 1))
        : "m" (*((volatile uint32_t *)mem + 1))
        : "%eax", "cc", "memory"
    );
}

/* Main driver function */
int main() {
    volatile uint32_t local_int = 0x87654321;
    volatile uint64_t local_long = 0x1122334455667788ULL;
    volatile uint32_t local_array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    
    printf("Starting coverage test...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x55);
    test_zero_extract(&local_int, 0xAA);
    test_zero_extract_reg(&global_int);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[0], 0x1234);
    test_strict_low_part(&global_short_array[1], 0x5678);
    test_strict_low_part_mem(&local_int);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_long);
    test_subreg_mem(&local_long);
    test_subreg_mem_mixed(local_array);
    test_early_clobber_subreg(&local_int, 0x1000, 0x2000);
    
    /* Test combined pattern */
    test_combined_pattern(&global_long);
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    checksum += global_int;
    checksum += global_long;
    
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    for (int i = 0; i < 4; i++) {
        checksum += local_array[i];
    }
    
    checksum += local_int;
    checksum += (local_long & 0xFFFFFFFF) + (local_long >> 32);
    
    printf("Checksum: 0x%016llX\n", (unsigned long long)checksum);
    printf("Test completed.\n");
    
    return 0;
}
