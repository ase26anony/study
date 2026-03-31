#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Force ZERO_EXTRACT by writing to specific bit-field */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*dest)  /* Output constraint for bit-field */
        : [src] "r" (src)
        : "memory"
    );
}

/* Alternative ZERO_EXTRACT with explicit bit-field in constraint */
static void test_zero_extract_bitfield(volatile uint32_t *dest) {
    uint32_t temp;
    /* Use constraint modifiers to hint at bit-field access */
    __asm__ volatile (
        "and %0, %0, #0xFF\n\t"  /* Only modify low 8 bits */
        : "=r" (temp)
        : "0" (*dest)
    );
    *dest = temp;
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Force STRICT_LOW_PART by modifying only part of register */
    __asm__ volatile (
        "add %[dest], %[src]\n\t"
        : [dest] "=r" (*dest)  /* Output constraint suggesting partial modification */
        : [src] "r" (src), "[dest]" (*dest)
        : "cc"
    );
}

/* STRICT_LOW_PART with explicit size modifier */
static void test_strict_low_part_byte(volatile uint8_t *dest, uint8_t src) {
    __asm__ volatile (
        "addb %[dest], %[src]\n\t"
        : [dest] "=r" (*dest)
        : [src] "r" (src), "[dest]" (*dest)
        : "cc"
    );
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    /* Access memory through different-sized pointer */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    /* Force SUBREG by accessing sub-register of memory */
    __asm__ volatile (
        "movw %0, #0xABCD\n\t"
        : "=m" (*short_ptr)  /* Memory operand for 16-bit access */
        :
        : "memory"
    );
}

/* More complex SUBREG with early-clobber */
static void test_subreg_mem_complex(volatile uint64_t *long_ptr) {
    volatile uint32_t *int_ptr = (volatile uint32_t *)long_ptr;
    
    __asm__ volatile (
        "mov %0, %1\n\t"
        "add %0, %0, #1\n\t"
        : "=&r" (*int_ptr)    /* Early-clobber to force SUBREG */
        : "r" (*int_ptr)
        : "cc", "memory"
    );
}

/* Test with array access causing SUBREG */
static void test_subreg_array(volatile uint32_t array[], int index) {
    volatile uint16_t *elem = (volatile uint16_t *)&array[index];
    
    __asm__ volatile (
        "strh %1, [%0]\n\t"
        :
        : "r" (elem), "r" ((uint16_t)0x1234)
        : "memory"
    );
}

/* Mixed pattern test */
static void test_mixed_patterns(volatile uint64_t *data) {
    volatile uint32_t *half1 = (volatile uint32_t *)data;
    volatile uint16_t *quarter = (volatile uint16_t *)data;
    
    /* Multiple operations to create complex RTL */
    __asm__ volatile (
        "mov %0, %1\n\t"          /* Potential SUBREG */
        "and %2, %2, #0xFF\n\t"   /* Potential ZERO_EXTRACT */
        : "=r" (*half1), "=r" (*quarter)
        : "1" (*half1), "2" (*quarter)
        : "memory"
    );
}

int main() {
    volatile uint32_t local_int = 0xDEADBEEF;
    volatile uint16_t local_short = 0x1234;
    volatile uint8_t local_byte = 0x42;
    volatile uint64_t local_long = 0x1122334455667788ULL;
    volatile uint32_t local_array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    
    int checksum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    test_zero_extract(&global_int, 0x87654321);
    test_zero_extract_bitfield(&local_int);
    
    /* Test STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    test_strict_low_part(&global_short_array[0], 0x5678);
    test_strict_low_part_byte(&global_byte_array[0], 0xAB);
    
    /* Test SUBREG of MEM patterns */
    printf("Testing SUBREG of MEM patterns...\n");
    test_subreg_mem(&local_int);
    test_subreg_mem_complex(&local_long);
    test_subreg_array(local_array, 1);
    
    /* Test mixed patterns */
    printf("Testing mixed patterns...\n");
    test_mixed_patterns(&global_long);
    
    /* Create checksum to prevent dead code elimination */
    checksum += global_int;
    checksum += global_short_array[0];
    checksum += global_byte_array[0];
    checksum += local_int;
    checksum += local_short;
    checksum += local_byte;
    checksum += (int)(local_long & 0xFFFFFFFF);
    checksum += local_array[0] + local_array[1] + local_array[2] + local_array[3];
    
    printf("Checksum: %d\n", checksum);
    printf("All patterns tested.\n");
    
    return 0;
}
