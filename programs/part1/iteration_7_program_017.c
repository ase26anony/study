#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_bytes[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *ptr, int bitpos, int bitsize)
{
    /* Force ZERO_EXTRACT by writing to specific bitfield */
    uint32_t temp;
    __asm__ volatile (
        /* Write to specific bits of memory using bitfield constraints */
        "bfi %0, %1, %2, %3"
        : "=r" (temp)
        : "r" (0xAA55), "I" (bitpos), "I" (bitsize)
        : "memory"
    );
    
    /* Alternative: Direct memory bitfield operation */
    __asm__ volatile (
        "str %1, [%0, #0]"
        : 
        : "r" (ptr), "r" (temp)
        : "memory"
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *ptr)
{
    uint32_t reg32;
    uint16_t reg16;
    
    /* Force STRICT_LOW_PART by modifying only part of a register */
    __asm__ volatile (
        /* Operation that only affects low 16 bits */
        "addw %0, %1, %2"
        : "=r" (reg32)
        : "r" (reg32), "r" (0x1234)
        : "cc"
    );
    
    /* Another pattern with explicit low-part constraint */
    __asm__ volatile (
        "mov %w0, %w1"
        : "=r" (reg16)
        : "r" (0xABCD)
        :
    );
    
    *ptr = reg16;
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr)
{
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    /* Access memory through SUBREG by casting pointer types */
    __asm__ volatile (
        /* Write to 16-bit sub-register of 32-bit memory location */
        "strh %1, [%0]"
        : 
        : "r" (short_ptr), "r" (0xDEAD)
        : "memory"
    );
    
    /* Alternative with explicit memory constraint */
    uint16_t value = 0xBEEF;
    __asm__ volatile (
        "mov %0, %1"
        : "=m" (*short_ptr)
        : "r" (value)
        : "memory"
    );
}

/* More complex SUBREG pattern with early-clobber */
static void test_subreg_complex(volatile uint64_t *ptr)
{
    uint32_t temp32;
    uint16_t temp16;
    
    /* Force SUBREG creation through register pressure */
    __asm__ volatile (
        "ldr %0, [%2]\n\t"
        "add %1, %0, %0\n\t"
        "strh %1, [%2, #2]"
        : "=&r" (temp32), "=&r" (temp16)
        : "r" (ptr)
        : "memory"
    );
}

/* Test ZERO_EXTRACT with memory destination */
static void test_zero_extract_mem(volatile uint8_t *mem)
{
    /* Try to create ZERO_EXTRACT of MEM */
    uint32_t bitfield;
    
    __asm__ volatile (
        /* Extract bits from memory and write back to different bits */
        "ubfx %0, %1, #4, #8\n\t"
        "bfi %1, %0, #12, #8"
        : "=&r" (bitfield), "+r" (*((volatile uint32_t*)mem))
        :
        : "memory"
    );
}

/* Main driver function */
int main(void)
{
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for coverage...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 8, 16);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[0]);
    checksum += global_short_array[0];
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    checksum += global_int & 0xFFFF;
    
    /* Test complex SUBREG patterns */
    test_subreg_complex(&global_long);
    checksum += (global_long >> 32) & 0xFFFF;
    
    /* Test ZERO_EXTRACT with memory */
    test_zero_extract_mem(global_bytes);
    for (int i = 0; i < 4; i++) {
        checksum += global_bytes[i];
    }
    
    /* Additional tests with different alignments */
    volatile uint32_t misaligned_int __attribute__((aligned(2)));
    volatile uint16_t misaligned_short __attribute__((aligned(1)));
    
    test_zero_extract(&misaligned_int, 4, 12);
    test_subreg_mem(&misaligned_int);
    
    checksum += misaligned_int;
    checksum += misaligned_short;
    
    printf("Checksum: %u\n", checksum);
    printf("All pattern tests completed.\n");
    
    return (int)(checksum & 0xFF);
}
