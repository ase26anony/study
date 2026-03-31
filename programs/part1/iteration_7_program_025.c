#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_bytes[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Assembly that should generate ZERO_EXTRACT RTL for bitfield assignment */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*dest)
        : [src] "r" (src)
        : "memory"
    );
}

/* Alternative ZERO_EXTRACT with explicit bitfield constraints */
static void test_zero_extract_bitfield(volatile uint32_t *dest) {
    uint32_t temp;
    /* Force bitfield extraction pattern */
    __asm__ volatile (
        "bfext %0, %1, #8, #8\n\t"
        : "=r" (temp)
        : "r" (*dest)
    );
    *dest = temp;
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Assembly that modifies only low part of register */
    __asm__ volatile (
        "addw %0, %1, #1\n\t"
        : "=r" (*dest)
        : "r" (src)
        : "cc"
    );
}

/* STRICT_LOW_PART with 8-bit operation */
static void test_strict_low_part_byte(volatile uint8_t *dest) {
    __asm__ volatile (
        "addb %0, %0, #1\n\t"
        : "+r" (*dest)
        :
        : "cc"
    );
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    /* Access sub-register of memory through pointer casting */
    __asm__ volatile (
        "movw %0, %1\n\t"
        : "=m" (*short_ptr)
        : "r" ((uint16_t)0xABCD)
        : "memory"
    );
}

/* More complex SUBREG pattern with early-clobber */
static void test_subreg_mem_complex(volatile uint64_t *long_ptr) {
    volatile uint32_t *int_ptr = (volatile uint32_t *)long_ptr;
    
    __asm__ volatile (
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        : "=&r" (*int_ptr)
        : "r" ((uint32_t)0x1234), "r" ((uint32_t)0x5678)
        : "cc", "memory"
    );
}

/* Test accessing different parts of a larger memory object */
static void test_memory_subreg_access(void) {
    volatile uint32_t buffer[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    
    /* Access 16-bit subreg of 32-bit memory */
    __asm__ volatile (
        "strh %1, [%0, #2]\n\t"
        :
        : "r" (buffer), "r" ((uint16_t)0xAAAA)
        : "memory"
    );
    
    /* Access 8-bit subreg */
    __asm__ volatile (
        "strb %1, [%0, #1]\n\t"
        :
        : "r" (buffer), "r" ((uint8_t)0xBB)
        : "memory"
    );
}

/* Combined test with multiple patterns */
static void test_combined_patterns(void) {
    volatile uint32_t combined = 0;
    volatile uint16_t *short_view = (volatile uint16_t *)&combined;
    
    /* This should generate complex RTL with multiple transformations */
    __asm__ volatile (
        "mov %0, %1\n\t"
        "and %0, %0, #0xFF00FF00\n\t"
        : "=r" (combined)
        : "r" (0xDEADBEEF)
        : "cc"
    );
    
    /* Follow with SUBREG memory access */
    __asm__ volatile (
        "movw %0, %1\n\t"
        : "=m" (*short_view)
        : "r" ((uint16_t)0x1234)
        : "memory"
    );
}

/* Main driver function */
int main(void) {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x87654321);
    checksum += global_int;
    
    test_zero_extract_bitfield(&global_int);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[0], 0x1234);
    checksum += global_short_array[0];
    
    test_strict_low_part_byte(&global_bytes[0]);
    checksum += global_bytes[0];
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    checksum += global_int;
    
    test_subreg_mem_complex(&global_long);
    checksum += (uint32_t)global_long;
    
    /* Test memory subreg access */
    test_memory_subreg_access();
    
    /* Test combined patterns */
    test_combined_patterns();
    checksum += global_int;
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: 0x%08X\n", checksum);
    
    return 0;
}
