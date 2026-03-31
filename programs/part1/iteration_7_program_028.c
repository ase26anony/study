#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *ptr, int bitpos, int bitsize)
{
    /* Use inline assembly with bit-field constraints */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*ptr)
        : [src] "r" (0xABCD),
          "0" (*ptr)
        : "memory"
    );
    
    /* Another attempt with explicit bit-field in constraints */
    uint32_t temp;
    __asm__ volatile (
        "bfi %0, %1, %2, %3"
        : "=r" (temp)
        : "r" (0x1234), "I" (bitpos), "I" (bitsize)
        : "cc"
    );
    *ptr = temp;
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *ptr)
{
    uint64_t val = *ptr;
    
    /* Assembly that modifies only low part of register */
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        : "=&r" (val)      /* Early clobber to force specific register allocation */
        : "r" (val), "r" (0x1000)
        : "cc"
    );
    
    /* Another pattern with explicit low-part constraint */
    uint32_t low_part;
    __asm__ volatile (
        "addw %0, %1, %2"
        : "=r" (low_part)
        : "r" ((uint32_t)val), "r" (0x5555)
        : "cc"
    );
    
    *ptr = ((uint64_t)low_part) | (val & 0xFFFFFFFF00000000ULL);
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *mem)
{
    /* Access memory through different-sized views */
    volatile uint16_t *short_ptr = (volatile uint16_t *)mem;
    volatile uint32_t *int_ptr = (volatile uint32_t *)mem;
    
    /* Write to sub-register of memory location */
    __asm__ volatile (
        "strh %1, [%0]"
        : 
        : "r" (short_ptr), "r" ((uint16_t)0xDEAD)
        : "memory"
    );
    
    /* Another pattern with memory constraint and cast */
    __asm__ volatile (
        "mov %0, %1"
        : "=m" (*(volatile uint8_t *)mem)
        : "r" ((uint8_t)0x42)
        : "memory"
    );
    
    /* Complex pattern with multiple sub-register accesses */
    uint32_t temp;
    __asm__ volatile (
        "ldr %0, [%1]\n\t"
        "add %0, %0, #1\n\t"
        "strh %0, [%1, #2]"
        : "=&r" (temp)
        : "r" (int_ptr)
        : "memory"
    );
}

/* Function to test bit-field extraction with memory destination */
static void test_memory_bitfield(volatile uint32_t *ptr)
{
    /* Try to create ZERO_EXTRACT with MEM destination */
    uint32_t mask = 0x00000FFF;
    uint32_t value = 0x555;
    
    __asm__ volatile (
        "bfi %0, %1, #4, #8"
        : "+m" (*ptr)
        : "r" (value)
        : "cc", "memory"
    );
}

/* Function to test strict low part with memory operand */
static void test_strict_low_part_mem(volatile uint64_t *ptr)
{
    /* Attempt to generate STRICT_LOW_PART with memory */
    __asm__ volatile (
        "add %0, %1, %2"
        : "=m" (*(volatile uint32_t *)ptr)  /* Cast to access only part */
        : "m" (*(volatile uint32_t *)ptr), "r" (0x1000)
        : "cc", "memory"
    );
}

/* Main driver function */
int main(void)
{
    uint32_t checksum = 0;
    
    printf("Testing ZERO_EXTRACT patterns...\n");
    test_zero_extract(&global_int, 8, 16);
    checksum += global_int;
    
    printf("Testing STRICT_LOW_PART patterns...\n");
    test_strict_low_part(&global_long);
    checksum += (uint32_t)global_long;
    checksum += (uint32_t)(global_long >> 32);
    
    printf("Testing SUBREG of MEM patterns...\n");
    test_subreg_mem(global_short_array);
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    printf("Testing memory bit-field patterns...\n");
    test_memory_bitfield(&global_int);
    checksum += global_int;
    
    printf("Testing strict low part memory patterns...\n");
    test_strict_low_part_mem(&global_long);
    checksum += (uint32_t)global_long;
    checksum += (uint32_t)(global_long >> 32);
    
    /* Additional tests with different data types */
    volatile struct {
        uint32_t a;
        uint16_t b;
        uint8_t c;
    } packed_struct = {0};
    
    test_subreg_mem(&packed_struct);
    checksum += packed_struct.a + packed_struct.b + packed_struct.c;
    
    /* Test with array element access */
    volatile uint32_t array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    test_zero_extract(&array[1], 4, 12);
    for (int i = 0; i < 4; i++) {
        checksum += array[i];
    }
    
    printf("Final checksum: 0x%08X\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);  /* Return non-negative value */
}
