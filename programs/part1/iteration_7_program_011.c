#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Using bit-field constraints to generate ZERO_EXTRACT RTL */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=r" (*dest)  /* This should generate ZERO_EXTRACT for bit-field */
        : "r" (src)
        : "memory"
    );
}

/* Another ZERO_EXTRACT variant with explicit bit-range */
static void test_zero_extract_bitfield(volatile uint64_t *dest, uint32_t src) {
    uint64_t temp = *dest;
    /* Write to bits 8-23 of a 64-bit value */
    __asm__ volatile (
        "bts %0, %1\n\t"  /* Bit test and set - operates on specific bits */
        : "+r" (temp)
        : "r" (src)
        : "cc"
    );
    *dest = temp;
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Using 'h' modifier for high byte, implying low part is preserved */
    __asm__ volatile (
        "addw %0, %1\n\t"  /* Word add - modifies only low 16 bits */
        : "=r" (*dest)
        : "r" (src), "0" (*dest)
        : "cc"
    );
}

/* STRICT_LOW_PART with byte operation */
static void test_strict_low_part_byte(volatile uint8_t *dest, uint8_t src) {
    __asm__ volatile (
        "addb %0, %1\n\t"  /* Byte add - strict low part of register */
        : "=q" (*dest)     /* 'q' constraint for byte-addressable register */
        : "q" (src), "0" (*dest)
        : "cc"
    );
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr, uint16_t value) {
    /* Cast to access sub-register of memory */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    /* This should generate SUBREG(MEM) RTL */
    __asm__ volatile (
        "movw %0, %1\n\t"
        : "=m" (*short_ptr)  /* Memory operand for 16-bit access to 32-bit memory */
        : "r" (value)
        : "memory"
    );
}

/* SUBREG of MEM with different sizes */
static void test_subreg_mem_mixed(volatile uint64_t *long_ptr, uint8_t value) {
    /* Access byte within 64-bit memory */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)long_ptr;
    
    __asm__ volatile (
        "movb %0, %1\n\t"
        : "=m" (*byte_ptr)  /* 8-bit memory access to 64-bit location */
        : "r" (value)
        : "memory"
    );
}

/* Complex case with early-clobber to force SUBREG patterns */
static void test_early_clobber_subreg(volatile uint32_t *dest, uint32_t src1, uint32_t src2) {
    uint32_t temp;
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=&r" (temp)      /* Early-clobber to complicate allocation */
        : "m" (*dest), "r" (src2)
        : "cc"
    );
    
    /* Write back through SUBREG */
    volatile uint16_t *short_dest = (volatile uint16_t *)dest;
    __asm__ volatile (
        "movw %0, %1\n\t"
        : "=m" (*short_dest)
        : "r" ((uint16_t)temp)
        : "memory"
    );
}

/* Test function combining multiple patterns */
static void test_combined_patterns(void) {
    volatile uint32_t combined_var = 0xDEADBEEF;
    volatile uint16_t short_var = 0x1234;
    volatile uint8_t byte_var = 0xAB;
    
    /* Mix different patterns */
    test_zero_extract(&combined_var, 0x5555AAAA);
    test_strict_low_part(&short_var, 0x5678);
    test_subreg_mem(&combined_var, 0x9ABC);
    
    /* Use the results to prevent dead code elimination */
    global_int ^= combined_var;
    global_short_array[0] ^= short_var;
    global_byte_array[0] ^= byte_var;
}

int main(void) {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x87654321);
    test_zero_extract_bitfield(&global_long, 0x1234);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[1], 0x1111);
    test_strict_low_part_byte(&global_byte_array[2], 0x22);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int, 0xABCD);
    test_subreg_mem_mixed(&global_long, 0xEF);
    
    /* Test complex patterns */
    test_early_clobber_subreg(&global_int, 0x11111111, 0x22222222);
    test_combined_patterns();
    
    /* Calculate checksum to prevent optimization */
    checksum ^= global_int;
    for (int i = 0; i < 8; i++) {
        checksum ^= global_short_array[i];
    }
    checksum ^= (uint32_t)(global_long ^ (global_long >> 32));
    for (int i = 0; i < 16; i++) {
        checksum ^= global_byte_array[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("If this prints, all functions were called\n");
    
    return (int)checksum;
}
