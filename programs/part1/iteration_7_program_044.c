#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT RTL pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Use bit-field assignment through inline assembly.
       The 'H' modifier suggests high part access */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=Q"(*dest)  /* 'Q' constraint for register a,b,c,d - often used with bit ops */
        : "r"(src)
        : "memory"
    );
}

/* Another ZERO_EXTRACT variant with explicit bit range */
static void test_zero_extract_bitfield(volatile uint32_t *dest, uint32_t src) {
    /* Try to force ZERO_EXTRACT by writing to specific bits */
    __asm__ volatile (
        "bts %0, %1\n\t"
        : "+r"(*dest)
        : "r"(src & 0x1F)  /* Limit to 5 bits for bit position */
        : "cc"
    );
}

/* Function to generate STRICT_LOW_PART RTL pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* The '=r' constraint with early clobber and specific register
       may generate STRICT_LOW_PART when only modifying part of register */
    __asm__ volatile (
        "addw %0, %1\n\t"
        : "=r"(*dest)
        : "r"(src), "0"(*dest)
        : "cc"
    );
}

/* STRICT_LOW_PART variant with byte operation */
static void test_strict_low_part_byte(volatile uint8_t *dest, uint8_t src) {
    /* Byte operation that should only affect low part */
    __asm__ volatile (
        "addb %1, %0\n\t"
        : "+q"(*dest)  /* 'q' constraint for byte-addressable registers */
        : "qi"(src)
        : "cc"
    );
}

/* Function to generate SUBREG of MEM RTL pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr, uint16_t value) {
    /* Access memory through different-sized pointer to force SUBREG */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        "movw %1, %0\n\t"
        : "=m"(*short_ptr)  /* Memory operand with different size than register */
        : "r"(value)
        : "memory"
    );
}

/* SUBREG variant with explicit casting in constraints */
static void test_subreg_mem_cast(volatile void *ptr, uint8_t value) {
    /* Force SUBREG by accessing part of larger memory object */
    __asm__ volatile (
        "movb %1, (%0)\n\t"
        : 
        : "r"(ptr), "r"(value)
        : "memory"
    );
}

/* Complex case: SUBREG of MEM with offset */
static void test_subreg_mem_offset(volatile uint64_t *array, int index, uint32_t value) {
    /* Access 32-bit portion of 64-bit memory location */
    volatile uint32_t *subptr = (volatile uint32_t *)((volatile char *)array + index * sizeof(uint64_t) + 2);
    
    __asm__ volatile (
        "movl %1, %0\n\t"
        : "=m"(*subptr)
        : "r"(value)
        : "memory"
    );
}

/* Combined test that might trigger multiple patterns */
static void test_combined(volatile uint32_t *data) {
    uint32_t temp;
    
    /* Read-modify-write that could generate complex RTL */
    __asm__ volatile (
        "movl (%1), %0\n\t"
        "andl $0xFFFF, %0\n\t"  /* STRICT_LOW_PART pattern */
        "movl %0, (%1)\n\t"
        : "=&r"(temp)  /* Early clobber to force separate register */
        : "r"(data)
        : "memory"
    );
}

/* Main driver function */
int main() {
    volatile uint32_t local_int = 0xABCDEF01;
    volatile uint16_t local_short = 0x1234;
    volatile uint8_t local_byte = 0x42;
    volatile uint64_t local_long_array[4] = {0};
    
    printf("Starting coverage test...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x87654321);
    test_zero_extract_bitfield(&local_int, 0x10);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[0], 0x1111);
    test_strict_low_part_byte(&global_byte_array[0], 0x22);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&local_int, 0x5678);
    test_subreg_mem_cast(&global_int, 0x99);
    test_subreg_mem_offset(local_long_array, 1, 0xDEADBEEF);
    
    /* Test combined pattern */
    test_combined(&local_int);
    
    /* Create artificial dependencies to prevent dead code elimination */
    uint32_t checksum = 0;
    checksum += global_int;
    checksum += global_short_array[0];
    checksum += global_byte_array[0];
    checksum += local_int;
    checksum += local_short;
    checksum += local_byte;
    
    for (int i = 0; i < 4; i++) {
        checksum += (uint32_t)local_long_array[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
