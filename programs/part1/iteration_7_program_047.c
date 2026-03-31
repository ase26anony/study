#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Use inline assembly with bit-field constraints */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=r" (*dest)  /* Output operand with bit-field constraint */
        : "r" (src)
        : "memory"
    );
}

/* Alternative ZERO_EXTRACT with explicit bit-field */
static void test_zero_extract_bitfield(volatile uint32_t *dest) {
    uint32_t temp;
    __asm__ volatile (
        "bfi %0, %1, #8, #8\n\t"  /* Bit-field insert - may generate ZERO_EXTRACT */
        : "=r" (temp)
        : "r" (0xFF)
        : 
    );
    *dest = temp;
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    uint32_t wide_reg;
    
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        : "=&r" (wide_reg)  /* Early clobber to force specific register allocation */
        : "r" ((uint32_t)src), "r" ((uint32_t)0x100)
        : 
    );
    
    /* Force partial register write */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=r" (*dest)
        : "r" ((uint16_t)wide_reg)
        : 
    );
}

/* STRICT_LOW_PART with arithmetic operation */
static void test_strict_low_part_arith(volatile uint8_t *dest) {
    uint16_t result;
    __asm__ volatile (
        "inc %0\n\t"
        : "=r" (result)  /* Only low part modified */
        : "0" ((uint16_t)*dest)
        : 
    );
    *dest = (uint8_t)result;
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    /* Access memory through different-sized pointer */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        "movw %0, %1\n\t"  /* Write 16-bit to what might be SUBREG of MEM */
        : "=m" (*short_ptr)  /* Memory operand with sub-register access */
        : "r" ((uint16_t)0xABCD)
        : "memory"
    );
}

/* More complex SUBREG of MEM with array access */
static void test_subreg_mem_array(volatile uint64_t *array) {
    /* Access 32-bit portion of 64-bit memory location */
    volatile uint32_t *sub_ptr = (volatile uint32_t *)((char *)array + 2);
    
    __asm__ volatile (
        "str %1, [%0]\n\t"
        : 
        : "r" (sub_ptr), "r" (0xDEADBEEF)
        : "memory"
    );
}

/* Test with pointer casting and offset */
static void test_subreg_mem_offset(volatile void *base) {
    /* Access misaligned 16-bit value within larger memory block */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)base;
    volatile uint16_t *misaligned_short = (volatile uint16_t *)(byte_ptr + 1);
    
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=m" (*misaligned_short)
        : "r" ((uint16_t)0x1234)
        : "memory"
    );
}

/* Combined test with multiple patterns */
static void test_combined(volatile uint32_t *mem) {
    uint32_t temp;
    
    /* First operation that might create SUBREG */
    __asm__ volatile (
        "ldrh %0, [%1]\n\t"  /* Load halfword - creates SUBREG */
        : "=r" (temp)
        : "r" ((volatile uint16_t *)mem)
        : "memory"
    );
    
    /* Operation that might create STRICT_LOW_PART */
    __asm__ volatile (
        "and %0, %1, %2\n\t"
        : "=r" (temp)
        : "r" (temp), "r" (0xFF)
        : 
    );
    
    /* Store back with possible ZERO_EXTRACT */
    __asm__ volatile (
        "strb %0, [%1]\n\t"
        : 
        : "r" ((uint8_t)temp), "r" ((volatile uint8_t *)mem)
        : "memory"
    );
}

int main() {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x87654321);
    test_zero_extract_bitfield(&global_int);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[0], 0x1234);
    test_strict_low_part_arith(&global_byte_array[0]);
    checksum += global_short_array[0] + global_byte_array[0];
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    test_subreg_mem_array(&global_long);
    test_subreg_mem_offset(global_byte_array);
    
    checksum += global_int;
    checksum += (uint32_t)(global_long & 0xFFFFFFFF);
    checksum += global_byte_array[1] + (global_byte_array[2] << 8);
    
    /* Test combined patterns */
    test_combined(&global_int);
    checksum += global_int;
    
    /* Add up all global values to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("All pattern tests completed\n");
    
    return (int)(checksum & 0xFF);
}
