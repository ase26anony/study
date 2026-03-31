#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function 1: Generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Write to specific bits of a 32-bit value using bit-field constraints */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=r" (*dest)  /* Output constraint for bit-field */
        : "r" (src)
        : "memory"
    );
}

/* Function 1b: More explicit ZERO_EXTRACT with bit-field */
static void test_zero_extract_bitfield(volatile uint32_t *dest) {
    uint32_t temp;
    /* Force ZERO_EXTRACT by writing to specific bits */
    __asm__ volatile (
        "bfi %0, %1, #8, #8\n\t"  /* Bit field insert: insert 8 bits at position 8 */
        : "=r" (temp)
        : "r" (0xFF)
        : 
    );
    *dest = temp;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Operation that only affects low part of register */
    __asm__ volatile (
        "addw %0, %1, %2\n\t"  /* Word-sized add (32-bit) */
        : "=r" (*dest)         /* Should generate STRICT_LOW_PART */
        : "r" (*dest), "r" (src)
        : "cc"
    );
}

/* Function 2b: Alternative STRICT_LOW_PART with byte operation */
static void test_strict_low_part_byte(volatile uint8_t *dest) {
    /* Byte operation that implies only low part is modified */
    __asm__ volatile (
        "incb %0\n\t"
        : "+r" (*dest)
        : 
        : "cc"
    );
}

/* Function 3: Generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    /* Access memory through different-sized pointer */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    /* Write to sub-register of memory location */
    __asm__ volatile (
        "movw %0, %1\n\t"
        : "=m" (*short_ptr)    /* Memory operand for 16-bit access */
        : "r" ((uint16_t)0xABCD)
        : "memory"
    );
}

/* Function 3b: More complex SUBREG of MEM with early-clobber */
static void test_subreg_mem_complex(volatile uint64_t *long_ptr) {
    volatile uint32_t *int_ptr = (volatile uint32_t *)long_ptr;
    
    /* Early-clobber to force SUBREG pattern */
    uint32_t temp;
    __asm__ volatile (
        "ldr %0, [%1]\n\t"
        "add %0, %0, #1\n\t"
        "str %0, [%1]\n\t"
        : "=&r" (temp), "+r" (int_ptr)
        : 
        : "memory"
    );
}

/* Function 3c: SUBREG of MEM with array access */
static void test_subreg_mem_array(volatile uint8_t *array) {
    /* Access 32-bit value as two 16-bit halves */
    volatile uint16_t *half_ptr = (volatile uint16_t *)(array + 2);
    
    __asm__ volatile (
        "strh %1, [%0]\n\t"
        : 
        : "r" (half_ptr), "r" ((uint16_t)0x1234)
        : "memory"
    );
}

/* Function 4: Combined pattern - ZERO_EXTRACT of MEM */
static void test_zero_extract_mem(volatile uint32_t *mem) {
    /* Try to create ZERO_EXTRACT of memory directly */
    __asm__ volatile (
        "bfi %0, %1, #16, #8\n\t"
        : "+m" (*mem)
        : "r" ((uint8_t)0xAA)
        : "memory"
    );
}

/* Driver function that calls all patterns */
int main() {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x87654321);
    checksum += global_int;
    
    test_zero_extract_bitfield(&global_int);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    for (int i = 0; i < 4; i++) {
        test_strict_low_part(&global_short_array[i], 0x100 * i);
        checksum += global_short_array[i];
    }
    
    test_strict_low_part_byte(&global_byte_array[0]);
    checksum += global_byte_array[0];
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    checksum += global_int;
    
    test_subreg_mem_complex(&global_long);
    checksum += (global_long & 0xFFFFFFFF) + (global_long >> 32);
    
    test_subreg_mem_array(global_byte_array);
    for (int i = 0; i < 4; i++) {
        checksum += global_byte_array[i];
    }
    
    /* Test combined pattern */
    test_zero_extract_mem(&global_int);
    checksum += global_int;
    
    /* Additional tests with local variables */
    volatile uint32_t local_var = 0xDEADBEEF;
    volatile uint64_t local_long = 0x123456789ABCDEF0ULL;
    
    test_zero_extract(&local_var, 0xCAFEBABE);
    checksum += local_var;
    
    test_subreg_mem(&local_var);
    checksum += local_var;
    
    test_strict_low_part((volatile uint16_t *)&local_var, 0x5555);
    checksum += local_var;
    
    printf("Final checksum: 0x%08X\n", checksum);
    printf("(This ensures code isn't optimized away)\n");
    
    return 0;
}
