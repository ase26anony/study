#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_bytes[16] = {0};

/* Function 1: Generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *ptr, int bitpos, int bitsize)
{
    uint32_t temp;
    
    /* Inline assembly that writes to a specific bit-field */
    __asm__ volatile (
        /* Write to specific bits of memory/register */
        "mov %[out], %[in]\n\t"
        /* The constraint should force ZERO_EXTRACT on the output */
        : [out] "=r" (temp)  /* Output in register */
        : [in] "r" (*ptr)    /* Input from memory */
        : /* No clobbers */
    );
    
    /* Force bit-field operation */
    __asm__ volatile (
        /* Bit-field insert/extract operation */
        "bfi %0, %1, %2, %3"
        : "+r" (temp)
        : "r" (0xAA), "I" (bitpos), "I" (bitsize)
    );
    
    /* Store back through pointer with bit-field constraints */
    __asm__ volatile (
        "str %[val], [%[addr]]"
        : 
        : [val] "r" (temp), [addr] "r" (ptr)
        : "memory"
    );
}

/* Function 2: Generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *ptr)
{
    uint64_t temp;
    
    /* Assembly that modifies only low part of register */
    __asm__ volatile (
        /* Operation that only affects low bits */
        "add %0, %1, %2"
        : "=&r" (temp)      /* Early clobber to force specific reg allocation */
        : "r" (*ptr), "i" (0xFF)
        : "cc"
    );
    
    /* Another pattern with explicit low-part constraint */
    __asm__ volatile (
        "and %0, %1, #0xFFFF"
        : "=r" (temp)
        : "r" (temp)
    );
    
    /* Store with potential STRICT_LOW_PART */
    __asm__ volatile (
        "mov %0, %1"
        : "=r" (*ptr)
        : "r" (temp)
        : "memory"
    );
}

/* Function 3: Generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *mem)
{
    volatile uint16_t *short_ptr = (volatile uint16_t *)mem;
    volatile uint8_t *byte_ptr = (volatile uint8_t *)mem;
    
    /* Access sub-register of memory through casting */
    __asm__ volatile (
        /* Write to 16-bit sub-register of 32-bit memory */
        "strh %[val], [%[addr]]"
        :
        : [val] "r" (0xABCD), [addr] "r" (short_ptr)
        : "memory"
    );
    
    /* Access byte within larger memory object */
    __asm__ volatile (
        /* Write single byte to memory */
        "strb %[val], [%[addr], #1]"
        :
        : [val] "r" (0xEF), [addr] "r" (byte_ptr)
        : "memory"
    );
    
    /* More complex SUBREG pattern with pointer arithmetic */
    __asm__ volatile (
        "ldrh %0, [%1, #2]\n\t"
        "add %0, %0, #1\n\t"
        "strh %0, [%1, #2]"
        : "=&r" (global_int)  /* Early clobber */
        : "r" (short_ptr)
        : "memory"
    );
}

/* Function 4: Combined pattern - SUBREG of MEM with bit operations */
static void test_combined_pattern(volatile uint32_t *array)
{
    volatile uint16_t *as_short = (volatile uint16_t *)array;
    
    /* Complex pattern that might generate multiple interesting RTLs */
    __asm__ volatile (
        /* Load, modify with bit operation, store back */
        "ldr %0, [%1]\n\t"
        "bfi %0, %2, #8, #8\n\t"    /* Bit-field insert */
        "str %0, [%1]"
        : "=&r" (global_long)  /* Early clobber */
        : "r" (array), "r" (0x55)
        : "memory"
    );
    
    /* Access different sized subregisters */
    __asm__ volatile (
        "ldrb %0, [%1, #3]\n\t"
        "strb %0, [%2]"
        : "=&r" (global_int)
        : "r" (array), "r" (&global_bytes[4])
        : "memory"
    );
}

/* Function 5: Test with array and pointer offsets */
static void test_array_patterns(void)
{
    volatile uint32_t local_array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    
    /* Access array elements with different sizes */
    test_subreg_mem(&local_array[1]);
    
    /* Force ZERO_EXTRACT on array element */
    test_zero_extract(&local_array[2], 4, 8);
    
    /* Force STRICT_LOW_PART */
    test_strict_low_part((volatile uint64_t *)&local_array[0]);
}

int main(void)
{
    uint32_t checksum = 0;
    
    printf("Starting coverage test for resource.cc lines 282-290\n");
    
    /* Test 1: ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 8, 8);
    test_zero_extract(&global_int, 16, 4);
    
    /* Test 2: STRICT_LOW_PART patterns */
    test_strict_low_part(&global_long);
    test_strict_low_part((volatile uint64_t *)&global_int);
    
    /* Test 3: SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    test_subreg_mem(global_short_array);
    test_subreg_mem(global_bytes);
    
    /* Test 4: Combined patterns */
    test_combined_pattern(&global_int);
    
    /* Test 5: Array patterns */
    test_array_patterns();
    
    /* Calculate checksum to prevent dead code elimination */
    checksum += global_int;
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    checksum += (global_long & 0xFFFFFFFF) + (global_long >> 32);
    for (int i = 0; i < 16; i++) {
        checksum += global_bytes[i];
    }
    
    printf("Checksum: %u\n", checksum);
    printf("Test completed. Compile with recommended flags to trigger coverage.\n");
    
    return 0;
}
