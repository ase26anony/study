#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Bit-field assignment: write to specific bits of a register/memory */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=r" (*dest)  /* Output constraint for register */
        : "r" (src)     /* Input constraint */
        : "memory"
    );
}

/* Function to generate ZERO_EXTRACT with memory destination */
static void test_zero_extract_mem(volatile uint32_t *dest, uint32_t src) {
    /* Write to specific bits of a memory location */
    __asm__ volatile (
        "and %0, %1, #0xFF\n\t"  /* Extract low 8 bits */
        : "=m" (*dest)           /* Memory output constraint */
        : "r" (src)              /* Input register */
        : "memory"
    );
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Only modify low part of register */
    __asm__ volatile (
        "add %0, %1, #1\n\t"
        : "=r" (*dest)  /* Output constraint implying partial modification */
        : "r" (src)     /* Input constraint */
        : "cc"          /* Clobber condition codes */
    );
}

/* Function to generate STRICT_LOW_PART with early clobber */
static void test_strict_low_part_earlyclobber(volatile uint32_t *dest, uint32_t src1, uint32_t src2) {
    /* Early clobber to force complex register allocation */
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        : "=&r" (*dest)  /* Early clobber output */
        : "r" (src1), "r" (src2)
        : "cc"
    );
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    /* Access sub-register of memory through pointer casting */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        "movw %0, #0xABCD\n\t"
        : "=m" (*short_ptr)  /* Memory output for 16-bit sub-register */
        :
        : "memory"
    );
}

/* Function to generate SUBREG of MEM with array access */
static void test_subreg_mem_array(volatile uint64_t *array) {
    /* Access 32-bit sub-register of 64-bit memory location */
    volatile uint32_t *sub_ptr = (volatile uint32_t *)&array[1];
    
    __asm__ volatile (
        "mov %0, #0xDEADBEEF\n\t"
        : "=m" (*sub_ptr)  /* 32-bit memory in 64-bit location */
        :
        : "memory"
    );
}

/* Function to generate complex pattern with multiple constraints */
static void test_complex_pattern(volatile uint32_t *dest, volatile uint16_t *src) {
    uint32_t temp;
    
    /* Read from memory, modify, write back with partial update */
    __asm__ volatile (
        "ldrh %1, [%2]\n\t"      /* Load halfword from memory */
        "and %0, %1, #0xFF00\n\t" /* Extract bits 8-15 */
        "orr %0, %0, #0x00FF\n\t" /* Set low bits */
        : "=&r" (temp), "+r" (dest)  /* Early clobber + read-write */
        : "r" (src)
        : "memory", "cc"
    );
    
    /* Write result back through different pointer type */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)dest;
    __asm__ volatile (
        "strb %1, [%0]\n\t"
        : 
        : "r" (byte_ptr), "r" ((uint8_t)(temp & 0xFF))
        : "memory"
    );
}

/* Driver function */
int main() {
    volatile uint32_t local_int = 0x87654321;
    volatile uint16_t local_short = 0x1234;
    volatile uint64_t local_long_array[4] = {0};
    volatile uint32_t checksum = 0;
    
    printf("Starting RTL pattern generation tests...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&local_int, 0x5555AAAA);
    checksum += local_int;
    
    test_zero_extract_mem(&global_int, 0x88776655);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&local_short, 0x1111);
    checksum += local_short;
    
    test_strict_low_part_earlyclobber(&local_int, 0x1000, 0x2000);
    checksum += local_int;
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    checksum += global_int;
    
    test_subreg_mem_array(local_long_array);
    checksum += (uint32_t)local_long_array[1];
    
    /* Test complex pattern */
    test_complex_pattern(&local_int, &local_short);
    checksum += local_int;
    
    /* Access all global variables to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    checksum += (uint32_t)global_long;
    
    printf("Final checksum: 0x%08X\n", checksum);
    printf("(This value varies based on architecture and compiler)\n");
    
    return (int)(checksum & 0xFF);  /* Return non-zero to indicate execution */
}
