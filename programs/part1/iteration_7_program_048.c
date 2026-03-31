#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint64_t global_64 = 0x123456789ABCDEF0ULL;
volatile uint32_t global_32 = 0xDEADBEEF;
volatile uint16_t global_16 = 0xCAFE;
volatile uint8_t global_8 = 0x42;
volatile uint32_t mem_array[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *ptr, int bitpos, int bitsize)
{
    /* Write to specific bit-field using inline assembly */
    __asm__ volatile (
        "mov %[val], %[src]\n\t"
        : [val] "=r" (*ptr)
        : [src] "r" (0x55AA55AA),
          "0" (*ptr)
        : "memory"
    );
    
    /* Another attempt with explicit bit-field constraint */
    uint32_t temp;
    __asm__ volatile (
        "bfi %0, %1, %2, %3"
        : "=r" (temp)
        : "r" (0x1234), "I" (bitpos), "I" (bitsize), "0" (*ptr)
    );
    *ptr = temp;
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *ptr)
{
    uint64_t result;
    
    /* Assembly that modifies only low part of register */
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        : "=&r" (result)      /* & for early clobber */
        : "r" (*ptr), "r" (0x1000)
        : "cc"
    );
    
    /* Another pattern with explicit low-part modification */
    __asm__ volatile (
        "and %0, %1, #0xFFFF"
        : "=r" (result)
        : "r" (*ptr)
    );
    
    *ptr = result;
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *base_ptr, int offset)
{
    /* Access sub-register of memory through pointer casting */
    volatile uint16_t *short_ptr = (volatile uint16_t *)base_ptr;
    
    /* Write to 16-bit sub-register of 32-bit memory location */
    __asm__ volatile (
        "strh %1, [%0]"
        : 
        : "r" (&short_ptr[offset]), "r" ((uint16_t)0x8888)
        : "memory"
    );
    
    /* Another pattern using memory constraint with modifier */
    uint16_t value = 0x9999;
    __asm__ volatile (
        "movw %0, %1"
        : "=m" (short_ptr[offset + 1])
        : "r" (value)
        : "memory"
    );
}

/* Additional test for complex SUBREG patterns */
static void test_complex_subreg(volatile void *mem)
{
    /* Access byte within word */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)mem;
    
    __asm__ volatile (
        "strb %1, [%0, #2]"
        :
        : "r" (byte_ptr), "r" ((uint8_t)0x77)
        : "memory"
    );
    
    /* Mixed-size access pattern */
    uint32_t word;
    __asm__ volatile (
        "ldrb %0, [%1, #1]\n\t"
        "strb %0, [%1, #3]"
        : "=&r" (word)
        : "r" (byte_ptr)
        : "memory"
    );
}

/* Test function combining multiple patterns */
static void test_combined_patterns(void)
{
    volatile uint32_t combined = 0;
    
    /* Attempt to create ZERO_EXTRACT with SUBREG */
    __asm__ volatile (
        "bfi %0, %1, #4, #8"
        : "+r" (combined)
        : "r" ((uint8_t)0xAA)
        : "cc"
    );
    
    /* Follow with memory store of partial result */
    volatile uint16_t *half_ptr = (volatile uint16_t *)&combined;
    __asm__ volatile (
        "strh %1, [%0]"
        :
        : "r" (half_ptr), "r" ((uint16_t)0x1234)
        : "memory"
    );
}

int main(void)
{
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_32, 8, 16);
    checksum += global_32;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_64);
    checksum += (global_64 & 0xFFFFFFFF);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&mem_array[0], 2);
    for (int i = 0; i < 4; i++) {
        checksum += mem_array[i];
    }
    
    /* Test complex SUBREG patterns */
    test_complex_subreg(&mem_array[4]);
    for (int i = 4; i < 8; i++) {
        checksum += mem_array[i];
    }
    
    /* Test combined patterns */
    test_combined_patterns();
    
    /* Additional tests with different data types */
    volatile int16_t short_var = 0;
    volatile int8_t byte_var = 0;
    
    /* Test with byte operations that might generate SUBREG */
    __asm__ volatile (
        "mov %0, #255"
        : "=r" (byte_var)
        :
    );
    checksum += byte_var;
    
    /* Test with array element access */
    volatile uint32_t local_array[4] = {1, 2, 3, 4};
    test_subreg_mem(local_array, 1);
    for (int i = 0; i < 4; i++) {
        checksum += local_array[i];
    }
    
    printf("Final checksum: %u\n", checksum);
    printf("Global 64-bit: 0x%016llX\n", (unsigned long long)global_64);
    printf("Global 32-bit: 0x%08X\n", global_32);
    
    return 0;
}
