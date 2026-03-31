#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint64_t global_64 = 0x123456789ABCDEF0ULL;
volatile uint32_t global_32 = 0xDEADBEEF;
volatile uint16_t global_16[4] = {0x1111, 0x2222, 0x3333, 0x4444};
volatile uint8_t global_8[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *mem, int bitpos, int bitsize)
{
    /* Use inline assembly to write to a specific bit-field */
    __asm__ volatile (
        "mov %[val], %[src]\n\t"
        : [val] "=r" (*mem)
        : [src] "ri" (0x5A5A5A5A),
          "0" (*mem)
        : "memory"
    );
    
    /* Another attempt with explicit bit-field constraint */
    uint32_t temp;
    __asm__ volatile (
        "bfi %0, %1, %2, %3"
        : "=r" (temp)
        : "r" (0x12345678), "I" (bitpos), "I" (bitsize)
        : "cc"
    );
    *mem = temp;
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *val)
{
    uint64_t result;
    
    /* Assembly that modifies only low part of register */
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        : "=&r" (result)      /* Early clobber to force specific reg allocation */
        : "r" (*val), "ri" (0x1000)
        : "cc"
    );
    
    /* Another pattern with explicit low-part modification */
    uint32_t low_part;
    __asm__ volatile (
        "and %0, %1, %2"
        : "=r" (low_part)
        : "r" ((uint32_t)*val), "ri" (0x0000FFFF)
        : "cc"
    );
    
    *val = ((uint64_t)low_part << 32) | low_part;
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *mem)
{
    /* Access different-sized subregions of memory */
    volatile uint16_t *as_short = (volatile uint16_t *)mem;
    volatile uint8_t *as_byte = (volatile uint8_t *)mem;
    
    /* Write to sub-region of memory using inline assembly */
    __asm__ volatile (
        "strh %1, [%0]"
        : 
        : "r" (as_short), "r" ((uint16_t)0x8888)
        : "memory"
    );
    
    /* Another access with byte operation */
    __asm__ volatile (
        "strb %1, [%0, #2]"
        : 
        : "r" (as_byte), "r" ((uint8_t)0x99)
        : "memory"
    );
    
    /* Complex pattern with pointer arithmetic */
    uint32_t temp;
    __asm__ volatile (
        "ldr %0, [%1]\n\t"
        "add %0, %0, #1\n\t"
        "str %0, [%1]"
        : "=&r" (temp)
        : "r" (mem)
        : "memory"
    );
}

/* Additional test for mixed patterns */
static void test_mixed_patterns(void)
{
    volatile uint32_t array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    
    /* Test ZERO_EXTRACT on array element */
    test_zero_extract(&array[1], 8, 16);
    
    /* Test STRICT_LOW_PART on 64-bit value */
    uint64_t combined = ((uint64_t)array[2] << 32) | array[3];
    test_strict_low_part(&combined);
    
    /* Test SUBREG access */
    test_subreg_mem(&array[0]);
}

int main(void)
{
    uint64_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test 1: ZERO_EXTRACT patterns */
    test_zero_extract(&global_32, 4, 12);
    checksum += global_32;
    
    /* Test 2: STRICT_LOW_PART patterns */
    test_strict_low_part(&global_64);
    checksum += global_64;
    
    /* Test 3: SUBREG of MEM patterns */
    test_subreg_mem(&global_16[0]);
    for (int i = 0; i < 4; i++) {
        checksum += global_16[i];
    }
    
    /* Test 4: Mixed patterns */
    test_mixed_patterns();
    
    /* Test 5: More complex ZERO_EXTRACT with memory operand */
    volatile struct {
        uint32_t field1 : 10;
        uint32_t field2 : 12;
        uint32_t field3 : 10;
    } bitfield = {0};
    
    uint32_t *as_int = (uint32_t *)&bitfield;
    test_zero_extract(as_int, 10, 12);
    checksum += *as_int;
    
    /* Test 6: SUBREG with different access sizes */
    volatile uint32_t int_var = 0x87654321;
    volatile uint16_t *short_ptr = (volatile uint16_t *)&int_var;
    
    __asm__ volatile (
        "strh %1, [%0]"
        :
        : "r" (short_ptr), "r" ((uint16_t)0x5555)
        : "memory"
    );
    checksum += int_var;
    
    /* Test 7: Early clobber to force SUBREG patterns */
    {
        volatile uint64_t data = 0x1122334455667788ULL;
        uint32_t low, high;
        
        __asm__ volatile (
            "mov %0, %2\n\t"
            "mov %1, %3"
            : "=&r" (low), "=&r" (high)  /* Early clobber on both */
            : "r" ((uint32_t)data), "r" ((uint32_t)(data >> 32))
            : "cc"
        );
        
        data = ((uint64_t)high << 32) | low;
        checksum += data;
    }
    
    printf("Final checksum: 0x%016llX\n", (unsigned long long)checksum);
    
    return 0;
}
