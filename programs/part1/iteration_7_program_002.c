#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_bytes[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *ptr, int bitpos, int bitsize)
{
    /* Force ZERO_EXTRACT by writing to specific bitfield */
    uint32_t temp;
    __asm__ volatile (
        /* Write to specific bits of memory using bitfield constraints */
        "mov %[out], %[in]\n\t"
        : [out] "=m" (*ptr)
        : [in] "r" ((uint32_t)(0xFF << bitpos))
        : "memory"
    );
    
    /* Another pattern that might generate ZERO_EXTRACT */
    __asm__ volatile (
        "bts %[bitpos], %[var]\n\t"
        : [var] "+m" (*ptr)
        : [bitpos] "Ir" (bitpos)
        : "cc", "memory"
    );
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *ptr)
{
    uint32_t reg_val;
    
    /* Force STRICT_LOW_PART by modifying only part of a register */
    __asm__ volatile (
        /* The '=q' constraint suggests QImode register, might generate STRICT_LOW_PART */
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        : "=q" (reg_val)
        : "0" (reg_val), "r" ((uint32_t)0x10001)
        : "cc"
    );
    
    /* Write result back */
    *ptr = (uint16_t)reg_val;
    
    /* Another pattern with explicit low-part modification */
    __asm__ volatile (
        "addw %0, %1\n\t"
        : "+r" (reg_val)
        : "ri" ((uint32_t)0xFFFF)
        : "cc"
    );
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr)
{
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    /* Access memory through SUBREG by writing to part of larger object */
    __asm__ volatile (
        /* Write to 16-bit sub-register of 32-bit memory location */
        "movw %0, %1\n\t"
        : "=m" (*short_ptr)
        : "r" ((uint16_t)0xABCD)
        : "memory"
    );
    
    /* Another pattern with early-clobber to force SUBREG */
    uint32_t temp;
    __asm__ volatile (
        "movl %1, %0\n\t"
        "movw %2, %0\n\t"
        : "=&r" (temp), "+m" (*int_ptr)
        : "r" ((uint16_t)0x1234)
        : "memory"
    );
}

/* Test with array access to generate complex MEM patterns */
static void test_array_subreg(volatile uint64_t *array)
{
    /* Access 32-bit portion of 64-bit array element */
    volatile uint32_t *subptr = (volatile uint32_t *)((char *)array + 2);
    
    __asm__ volatile (
        /* Unaligned access to force SUBREG MEM */
        "movl %0, %1\n\t"
        : "=m" (*subptr)
        : "r" ((uint32_t)0xDEADBEEF)
        : "memory"
    );
}

/* Function combining multiple patterns */
static void test_combined(volatile uint32_t *mem)
{
    uint32_t temp;
    
    /* Complex pattern that might generate multiple interesting RTLs */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "btrl $8, %0\n\t"
        "movw %0, %2\n\t"
        : "=&r" (temp), "+m" (*mem)
        : "m" (*(volatile uint16_t *)mem)
        : "cc", "memory"
    );
}

int main(void)
{
    volatile uint32_t local_int = 0x87654321;
    volatile uint16_t local_short_array[4] = {0x1111, 0x2222, 0x3333, 0x4444};
    volatile uint64_t local_long_array[2] = {0};
    
    printf("Starting coverage test...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 8, 8);
    test_zero_extract(&local_int, 16, 8);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[0]);
    test_strict_low_part(&local_short_array[1]);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    test_subreg_mem(&local_int);
    
    /* Test array-based SUBREG patterns */
    test_array_subreg(&global_long);
    test_array_subreg(local_long_array);
    
    /* Test combined patterns */
    test_combined(&global_int);
    test_combined(&local_int);
    
    /* Create checksum to prevent dead code elimination */
    uint32_t checksum = 0;
    checksum += global_int;
    for (int i = 0; i < 8; i++) checksum += global_short_array[i];
    checksum += (uint32_t)(global_long >> 32) + (uint32_t)global_long;
    for (int i = 0; i < 16; i++) checksum += global_bytes[i];
    checksum += local_int;
    for (int i = 0; i < 4; i++) checksum += local_short_array[i];
    for (int i = 0; i < 2; i++) checksum += (uint32_t)(local_long_array[i] >> 32) + (uint32_t)local_long_array[i];
    
    printf("Checksum: %u\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
