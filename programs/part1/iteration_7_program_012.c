#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_bytes[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src)
{
    /* Write to specific bit-field (bits 8-15) using inline assembly */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*dest)
        : [src] "r" (src)
        : "memory"
    );
    
    /* Another pattern: bit-field insert using constraints */
    uint32_t temp;
    __asm__ volatile (
        "bfi %0, %1, #8, #8\n\t"  /* Bit-field insert: insert src bits 0-7 into dest bits 8-15 */
        : "=r" (temp)
        : "r" (src), "0" (*dest)
        : 
    );
    *dest = temp;
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *dest, uint32_t src)
{
    /* Operation that only modifies low 32 bits */
    __asm__ volatile (
        "add %k[dest], %k[src]\n\t"  /* %k modifier for 32-bit register name */
        : [dest] "+&r" (*dest)       /* Early clobber + register constraint */
        : [src] "r" (src)
        : "cc"
    );
    
    /* Another pattern with explicit low-part constraint */
    uint64_t result;
    __asm__ volatile (
        "mov %[result], %[dest]\n\t"
        "addl %k[result], %k[src]\n\t"  /* 32-bit add to low part */
        : [result] "=&r" (result)
        : [dest] "r" (*dest), [src] "r" (src)
        : "cc"
    );
    *dest = result;
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *mem, uint16_t value)
{
    /* Access memory through different-sized views */
    volatile uint32_t *as_int = (volatile uint32_t *)mem;
    volatile uint16_t *as_short = (volatile uint16_t *)mem;
    volatile uint8_t *as_byte = (volatile uint8_t *)mem;
    
    /* Write to sub-register of memory location */
    __asm__ volatile (
        "movw %[ptr], %[val]\n\t"  /* 16-bit write to memory */
        : [ptr] "=m" (*as_short)
        : [val] "r" (value)
        : "memory"
    );
    
    /* Another pattern: byte access within word */
    __asm__ volatile (
        "movb %b[val], %[ptr]\n\t"  /* 8-bit write */
        : [ptr] "=m" (*as_byte)
        : [val] "r" (value)
        : "memory"
    );
    
    /* Complex pattern with offset */
    __asm__ volatile (
        "movw 2%[base], %[val]\n\t"  /* Write to offset within memory */
        : 
        : [base] "o" (*as_int), [val] "r" (value)
        : "memory"
    );
}

/* Additional test for bit-field extraction with memory */
static void test_memory_bitfield(volatile uint32_t *mem)
{
    uint32_t temp;
    /* Extract bits 16-23 from memory */
    __asm__ volatile (
        "ldr %[temp], [%[mem]]\n\t"
        "ubfx %[temp], %[temp], #16, #8\n\t"  /* Unsigned bit-field extract */
        : [temp] "=&r" (temp)
        : [mem] "r" (mem)
        : "memory"
    );
    
    /* Insert extracted bits back at different position */
    __asm__ volatile (
        "bfi %[temp], %[temp], #0, #8\n\t"
        "str %[temp], [%[mem]]\n\t"
        : [temp] "+&r" (temp)
        : [mem] "r" (mem)
        : "memory"
    );
}

/* Test with array and pointer arithmetic */
static void test_array_subreg(void)
{
    volatile uint32_t array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    
    /* Access array element as different type */
    __asm__ volatile (
        "movw %[elem], #0xAAAA\n\t"
        : [elem] "=m" (*(volatile uint16_t *)&array[1])  /* Cast to 16-bit */
        : 
        : "memory"
    );
    
    /* Access with byte offset */
    __asm__ volatile (
        "movb %[val], 1%[base]\n\t"
        : [val] "=r" (global_bytes[0])
        : [base] "o" (array[0])
        : "memory"
    );
}

int main(void)
{
    uint32_t checksum = 0;
    
    printf("Testing ZERO_EXTRACT patterns...\n");
    test_zero_extract(&global_int, 0xABCDEF01);
    checksum += global_int;
    
    printf("Testing STRICT_LOW_PART patterns...\n");
    test_strict_low_part(&global_long, 0x12345678);
    checksum += (uint32_t)global_long + (uint32_t)(global_long >> 32);
    
    printf("Testing SUBREG of MEM patterns...\n");
    test_subreg_mem(&global_int, 0x1234);
    checksum += global_int;
    
    printf("Testing memory bitfield patterns...\n");
    test_memory_bitfield(&global_int);
    checksum += global_int;
    
    printf("Testing array subreg patterns...\n");
    test_array_subreg();
    
    /* Sum all global arrays */
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += global_bytes[i];
    }
    
    printf("Final checksum: 0x%08X\n", checksum);
    return (int)checksum;
}
