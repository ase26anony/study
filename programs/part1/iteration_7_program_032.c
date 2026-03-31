#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Use bit-field constraints to generate ZERO_EXTRACT RTL */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=r" (*dest)  /* Output in register that will be extracted */
        : "r" (src)
        : "memory"
    );
    
    /* Another pattern with explicit bitfield operation */
    uint32_t temp;
    __asm__ volatile (
        "bfi %0, %1, #8, #8\n\t"  /* Bit field insert - common for ZERO_EXTRACT */
        : "=r" (temp)
        : "r" (src)
    );
    *dest = temp;
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Operation that only affects low part of register */
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        : "=r" (*dest)  /* Output constraint suggesting partial modification */
        : "r" (*dest), "r" (src)
        : "cc"
    );
    
    /* Using early-clobber to force complex register allocation */
    uint32_t wide_temp;
    __asm__ volatile (
        "and %0, %1, #0xFFFF\n\t"  /* Only keep low 16 bits */
        : "=&r" (wide_temp)        /* Early-clobber */
        : "r" (wide_temp)
    );
    *dest = (uint16_t)wide_temp;
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr, uint16_t value) {
    /* Access memory through different-sized pointer */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    /* Write to sub-region of memory */
    __asm__ volatile (
        "strh %1, [%0]\n\t"        /* Store halfword to memory */
        : 
        : "r" (short_ptr), "r" (value)
        : "memory"
    );
    
    /* Another pattern with memory constraint */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=m" (*short_ptr)        /* Memory output constraint */
        : "r" (value)
    );
}

/* Additional test with array access */
static void test_complex_mem_access(volatile uint8_t *array, int index, uint8_t value) {
    /* Access array element with type punning */
    volatile uint16_t *alias = (volatile uint16_t *)(array + index);
    
    __asm__ volatile (
        "strb %1, [%0]\n\t"
        : 
        : "r" (&array[index]), "r" (value)
        : "memory"
    );
    
    /* Force SUBREG MEM through pointer arithmetic */
    uint32_t *int_alias = (uint32_t *)(array + (index & ~3));
    __asm__ volatile (
        "ldr %0, [%1]\n\t"
        "add %0, %0, #1\n\t"
        "str %0, [%1]\n\t"
        : "=&r" (value)            /* Early-clobber */
        : "r" (int_alias)
        : "memory"
    );
}

/* Test with 64-bit operations generating complex patterns */
static void test_64bit_operations(volatile uint64_t *dest, uint64_t src) {
    /* Operations that might generate SUBREG for 32-bit parts */
    uint32_t low_part = (uint32_t)src;
    uint32_t high_part = (uint32_t)(src >> 32);
    
    __asm__ volatile (
        "adds %0, %0, %2\n\t"      /* Affects low part and flags */
        "adc %1, %1, %3\n\t"       /* Add with carry for high part */
        : "+r" (low_part), "+r" (high_part)
        : "r" (low_part), "r" (high_part)
        : "cc"
    );
    
    *dest = ((uint64_t)high_part << 32) | low_part;
}

int main() {
    uint32_t checksum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    test_zero_extract(&global_int, 0xABCD1234);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    for (int i = 0; i < 4; i++) {
        test_strict_low_part(&global_short_array[i], (uint16_t)(i * 0x1111));
        checksum += global_short_array[i];
    }
    
    /* Test SUBREG MEM patterns */
    printf("Testing SUBREG MEM patterns...\n");
    test_subreg_mem(&global_int, 0x5678);
    checksum += global_int;
    
    /* Test complex memory access */
    printf("Testing complex memory access...\n");
    test_complex_mem_access(global_byte_array, 4, 0x42);
    for (int i = 0; i < 8; i++) {
        checksum += global_byte_array[i];
    }
    
    /* Test 64-bit operations */
    printf("Testing 64-bit operations...\n");
    test_64bit_operations(&global_long, 0x1122334455667788ULL);
    checksum += (uint32_t)global_long;
    checksum += (uint32_t)(global_long >> 32);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: 0x%08X\n", checksum);
    
    return 0;
}
