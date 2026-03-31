#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_bytes[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Use bit-field constraints to generate ZERO_EXTRACT RTL */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=r" (*dest)  /* Output constraint for bit-field */
        : "r" (src)
        : "memory"
    );
}

/* Another ZERO_EXTRACT variant with explicit bit-range */
static void test_zero_extract_bitfield(volatile uint32_t *dest) {
    uint32_t temp;
    /* Force extraction of specific bits */
    __asm__ volatile (
        "and %0, %1, #0xFF\n\t"
        : "=r" (temp)
        : "r" (*dest)
        : "cc"
    );
    *dest = temp;
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Early-clobber constraint to force STRICT_LOW_PART */
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        : "=&r" (*dest)  /* Early-clobber to complicate allocation */
        : "r" (*dest), "r" (src)
        : "cc"
    );
}

/* STRICT_LOW_PART with 8-bit operation */
static void test_strict_low_part_byte(volatile uint8_t *dest, uint8_t src) {
    __asm__ volatile (
        "addb %0, %1\n\t"
        : "+r" (*dest)  /* Read-write operand for partial modification */
        : "r" (src)
        : "cc"
    );
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    /* Access memory through cast pointer to create SUBREG */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        "movw %0, #0xABCD\n\t"
        : "=m" (*short_ptr)  /* Memory operand for sub-register access */
        :
        : "memory"
    );
}

/* Complex SUBREG pattern with array access */
static void test_subreg_mem_complex(volatile uint64_t *array) {
    /* Access different sized elements within the array */
    volatile uint32_t *as_int = (volatile uint32_t *)array;
    volatile uint16_t *as_short = (volatile uint16_t *)array;
    
    __asm__ volatile (
        "str %1, [%0]\n\t"
        "strh %2, [%0, #4]\n\t"
        :
        : "r" (as_int), "r" (0xDEADBEEF), "r" ((uint16_t)0xCAFE)
        : "memory"
    );
    
    /* Another access pattern */
    __asm__ volatile (
        "ldrh %0, [%1]\n\t"
        : "=r" (as_short[1])
        : "r" (&as_short[0])
        : "memory"
    );
}

/* Combined pattern: SUBREG -> MEM -> ZERO_EXTRACT */
static void test_combined_pattern(volatile uint64_t *dest) {
    volatile uint32_t *part = (volatile uint32_t *)dest;
    
    /* This may generate complex nested RTL */
    __asm__ volatile (
        "ldr %0, [%1]\n\t"
        "and %0, %0, #0xFFFF\n\t"
        "strh %0, [%1, #2]\n\t"
        :
        : "r" (part[0]), "r" (&part[0])
        : "memory", "cc"
    );
}

/* Main driver function */
int main() {
    volatile uint32_t local_int = 0x87654321;
    volatile uint16_t local_short = 0x1234;
    volatile uint8_t local_byte = 0x42;
    volatile uint64_t local_long_array[4] = {0};
    
    uint32_t checksum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x89ABCDEF);
    test_zero_extract_bitfield(&local_int);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[0], 0x1111);
    test_strict_low_part_byte(&global_bytes[0], 0x22);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    test_subreg_mem_complex(&global_long);
    
    /* Test combined pattern */
    test_combined_pattern(&global_long);
    
    /* Additional calls with different arguments */
    for (int i = 0; i < 4; i++) {
        test_strict_low_part(&global_short_array[i], i * 0x100);
        test_subreg_mem((volatile uint32_t *)&local_long_array[i]);
    }
    
    /* Calculate checksum to prevent dead code elimination */
    checksum += global_int;
    checksum += global_long & 0xFFFFFFFF;
    checksum += global_long >> 32;
    
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += global_bytes[i];
    }
    
    checksum += local_int;
    checksum += local_short;
    checksum += local_byte;
    
    printf("Checksum: 0x%08X\n", checksum);
    
    return (int)checksum;
}
