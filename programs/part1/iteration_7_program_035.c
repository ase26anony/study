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
        /* Write to specific bits 8-15 of the destination */
        "btsl %1, %0\n\t"
        : "+m" (*dest)  /* Memory operand that may become ZERO_EXTRACT */
        : "r" (src & 0x07)  /* Bit position */
        : "cc", "memory"
    );
}

/* Another ZERO_EXTRACT variant with bit-field in register */
static void test_zero_extract_reg(volatile uint64_t *dest, uint64_t mask) {
    uint64_t temp;
    __asm__ volatile (
        /* Extract bits 16-31 from mask and store in bits 32-47 of dest */
        "movq %2, %0\n\t"
        "andq $0xFFFF0000, %0\n\t"
        "shlq $16, %0\n\t"
        "orq %0, %1\n\t"
        : "=&r" (temp), "+m" (*dest)
        : "r" (mask)
        : "cc"
    );
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint32_t *dest, uint32_t src) {
    uint32_t temp;
    /* Target: SET_DEST with STRICT_LOW_PART */
    __asm__ volatile (
        /* Operation that only affects low 16 bits */
        "addw %w2, %w0\n\t"  /* 'w' modifier for word (16-bit) operation */
        : "=r" (temp)
        : "0" (*dest), "r" (src)
        : "cc"
    );
    *dest = temp;
}

/* STRICT_LOW_PART with byte operation */
static void test_strict_low_part_byte(volatile uint8_t *dest, uint8_t src) {
    uint8_t temp;
    __asm__ volatile (
        "addb %b2, %b0\n\t"  /* 'b' modifier for byte operation */
        : "=r" (temp)
        : "0" (*dest), "r" (src)
        : "cc"
    );
    *dest = temp;
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr, uint16_t value) {
    /* Target: SET_DEST with SUBREG wrapping MEM */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        /* Write to the low 16 bits of the 32-bit memory location */
        "movw %w1, %0\n\t"
        : "=m" (*short_ptr)  /* Memory operand as 16-bit access to 32-bit location */
        : "r" (value)
        : "memory"
    );
}

/* Complex SUBREG pattern with array access */
static void test_subreg_mem_array(volatile uint64_t *array, int index, uint32_t value) {
    /* Access 32-bit portion of 64-bit array element */
    volatile uint32_t *subptr = (volatile uint32_t *)((char *)array + index * sizeof(uint64_t));
    
    __asm__ volatile (
        "movl %1, %0\n\t"
        : "=m" (*subptr)
        : "r" (value)
        : "memory"
    );
}

/* Mixed pattern that might generate multiple target RTL constructs */
static void test_mixed_pattern(volatile uint64_t *dest, uint64_t src1, uint64_t src2) {
    uint64_t temp1, temp2;
    
    __asm__ volatile (
        /* Multiple operations that could generate various patterns */
        "movq %2, %0\n\t"
        "andq $0x0000FFFF, %0\n\t"      /* ZERO_EXTRACT-like */
        "addw %w3, %w0\n\t"             /* STRICT_LOW_PART-like */
        "movw %w0, (%1)\n\t"            /* SUBREG of MEM */
        : "=&r" (temp1), "+r" (temp2)
        : "r" (src1), "r" (src2), "m" (*dest)
        : "cc", "memory"
    );
}

/* Main driver function */
int main() {
    uint32_t checksum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x4);
    test_zero_extract_reg(&global_long, 0x12345678);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_int, 0x100);
    test_strict_low_part_byte(&global_bytes[0], 0x42);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int, 0xABCD);
    test_subreg_mem_array(&global_long, 0, 0xDEADBEEF);
    
    /* Test mixed pattern */
    test_mixed_pattern(&global_long, 0x11112222, 0x3333);
    
    /* Calculate checksum to prevent dead code elimination */
    checksum += global_int;
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    checksum += (uint32_t)(global_long & 0xFFFFFFFF);
    checksum += (uint32_t)(global_long >> 32);
    for (int i = 0; i < 16; i++) {
        checksum += global_bytes[i];
    }
    
    printf("Checksum: %u\n", checksum);
    return 0;
}
