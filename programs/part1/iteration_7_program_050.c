#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Using bit-field constraints to generate ZERO_EXTRACT RTL */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=r" (*dest)  /* Output constraint for bit-field */
        : "r" (src)
        : "memory"
    );
}

/* Alternative ZERO_EXTRACT with explicit bit-range */
static void test_zero_extract_bitfield(volatile uint32_t *dest) {
    uint32_t temp;
    /* Force ZERO_EXTRACT by writing to specific bits */
    __asm__ volatile (
        "bfi %0, %1, #8, #8\n\t"  /* Bit field insert: bits 8-15 */
        : "+r" (temp)
        : "r" (0xFF)
        :
    );
    *dest = temp;
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Using '=r' constraint with early-clobber to force partial register update */
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        : "=&r" (*dest)  /* Early-clobber & partial register */
        : "r" (src), "r" (0x100)
        : "cc"
    );
}

/* STRICT_LOW_PART with byte operation */
static void test_strict_low_part_byte(volatile uint8_t *dest) {
    /* Operation that only affects low part of register */
    __asm__ volatile (
        "incb %0\n\t"
        : "+r" (*dest)
        :
        : "cc"
    );
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    /* Access memory through cast pointer to create SUBREG(MEM) */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        "movw %0, %1\n\t"
        : "=m" (*short_ptr)  /* Memory operand for sub-register access */
        : "r" (0xABCD)
        : "memory"
    );
}

/* Alternative SUBREG(MEM) with different size access */
static void test_subreg_mem_mixed(volatile uint64_t *long_ptr) {
    /* Access 32-bit portion of 64-bit memory location */
    volatile uint32_t *int_ptr = (volatile uint32_t *)long_ptr;
    
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=m" (*int_ptr)
        : "r" (0xDEADBEEF)
        : "memory"
    );
}

/* Complex case: SUBREG -> MEM -> address computation */
static void test_subreg_mem_complex(volatile uint8_t *array) {
    /* Access array element with type punning */
    volatile uint32_t *as_int = (volatile uint32_t *)(array + 1);
    
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        : "=m" (*as_int)
        : "r" (0x1000), "r" (0x2000)
        : "memory"
    );
}

/* Driver function that calls all patterns */
int main() {
    uint32_t checksum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    test_zero_extract(&global_int, 0x87654321);
    test_zero_extract_bitfield(&global_int);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    test_strict_low_part(&global_short_array[0], 0x1234);
    test_strict_low_part_byte(&global_byte_array[0]);
    checksum += global_short_array[0];
    checksum += global_byte_array[0];
    
    /* Test SUBREG of MEM patterns */
    printf("Testing SUBREG of MEM patterns...\n");
    test_subreg_mem(&global_int);
    test_subreg_mem_mixed(&global_long);
    test_subreg_mem_complex(global_byte_array);
    
    /* Calculate checksum to prevent dead code elimination */
    checksum += global_int;
    checksum += (uint32_t)(global_long & 0xFFFFFFFF);
    checksum += (uint32_t)(global_long >> 32);
    
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    printf("Final checksum: 0x%08X\n", checksum);
    
    /* Additional test with local variables to increase register pressure */
    {
        volatile uint32_t local_vars[10];
        volatile uint16_t *short_ptrs[5];
        
        for (int i = 0; i < 10; i++) {
            local_vars[i] = i * 0x11111111;
        }
        
        /* Create complex memory access patterns */
        for (int i = 0; i < 5; i++) {
            short_ptrs[i] = (volatile uint16_t *)&local_vars[i];
            test_strict_low_part(short_ptrs[i], i * 0x100);
        }
        
        /* Force ZERO_EXTRACT with bit operations */
        for (int i = 0; i < 10; i += 2) {
            test_zero_extract(&local_vars[i], local_vars[i + 1]);
        }
    }
    
    return 0;
}
