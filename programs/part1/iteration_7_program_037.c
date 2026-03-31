#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
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

/* Another ZERO_EXTRACT variant with bit-field operation */
static void test_zero_extract_bitfield(volatile uint64_t *dest) {
    uint64_t mask = 0x000000FF00000000ULL;
    uint64_t value = 0x0000005500000000ULL;
    
    __asm__ volatile (
        /* Clear and set specific bit range in memory */
        "andq %1, %0\n\t"
        "orq  %2, %0\n\t"
        : "+m" (*dest)
        : "r" (~mask), "r" (value)
        : "cc", "memory"
    );
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint32_t *dest, uint32_t src) {
    uint32_t temp;
    
    __asm__ volatile (
        /* Operation that only affects low 16 bits */
        "addw %w2, %w0\n\t"
        /* Use '&' for early clobber to force complex register allocation */
        : "=&r" (temp)
        : "0" (*dest), "r" (src)
        : "cc"
    );
    
    *dest = temp;
}

/* STRICT_LOW_PART with byte operation */
static void test_strict_low_part_byte(volatile uint8_t *dest) {
    uint8_t result;
    
    __asm__ volatile (
        /* Byte operation - compiler may use STRICT_LOW_PART */
        "incb %b0\n\t"
        : "=r" (result)
        : "0" (*dest)
        : "cc"
    );
    
    *dest = result;
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    /* Access short within int through pointer casting */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        /* Write to sub-register of memory (short within int) */
        "movw %1, %0\n\t"
        : "=m" (*short_ptr)
        : "r" ((uint16_t)0xABCD)
        : "memory"
    );
}

/* SUBREG of MEM with different size access */
static void test_subreg_mem_mixed(volatile uint64_t *long_ptr) {
    /* Access 32-bit portion of 64-bit memory */
    volatile uint32_t *int_ptr = (volatile uint32_t *)long_ptr;
    
    __asm__ volatile (
        /* Write to 32-bit sub-register of 64-bit memory */
        "movl %1, %0\n\t"
        : "=m" (int_ptr[1])  /* Access second 32-bit portion */
        : "r" (0xDEADBEEF)
        : "memory"
    );
}

/* Complex case combining multiple patterns */
static void test_combined_pattern(volatile uint32_t *mem) {
    uint32_t temp;
    volatile uint16_t *short_ptr = (volatile uint16_t *)mem;
    
    /* First do a STRICT_LOW_PART operation */
    __asm__ volatile (
        "andl $0xFFFF, %0\n\t"
        : "=r" (temp)
        : "0" (*mem)
        : "cc"
    );
    
    /* Then write to SUBREG of MEM */
    __asm__ volatile (
        "movw %w1, %0\n\t"
        : "=m" (*short_ptr)
        : "r" (temp)
        : "memory"
    );
    
    /* Finally a ZERO_EXTRACT-like operation */
    __asm__ volatile (
        "btrl $16, %0\n\t"
        : "+m" (*mem)
        :
        : "cc", "memory"
    );
}

/* Main driver function */
int main() {
    volatile uint32_t local_int = 0x87654321;
    volatile uint64_t local_long = 0x1122334455667788ULL;
    volatile uint32_t array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    
    printf("Starting coverage test...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x10);
    test_zero_extract_bitfield(&global_long);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&local_int, 0x1000);
    test_strict_low_part_byte(&global_bytes[0]);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&local_int);
    test_subreg_mem_mixed(&local_long);
    
    /* Test combined patterns */
    test_combined_pattern(&array[0]);
    
    /* Additional tests with different alignments */
    test_zero_extract((volatile uint32_t*)&global_short_array[1], 0x5);
    test_subreg_mem((volatile uint32_t*)&global_bytes[2]);
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    checksum += global_int;
    checksum += global_long;
    checksum += local_int;
    checksum += local_long;
    
    for (int i = 0; i < 4; i++) {
        checksum += array[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += global_bytes[i];
    }
    
    printf("Checksum: 0x%016llX\n", (unsigned long long)checksum);
    printf("Test completed.\n");
    
    return 0;
}
