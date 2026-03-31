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
    uint32_t temp;
    /* Force ZERO_EXTRACT by writing to specific bitfield */
    __asm__ volatile (
        "mov %[temp], %[val]\n\t"
        "bfi %[out], %[temp], %[pos], %[size]"
        : [out] "+r" (*ptr)
        : [temp] "r" (0xAA55AA55), [val] "r" (0x12345678),
          [pos] "I" (bitpos), [size] "I" (bitsize)
        : "cc"
    );
}

/* Alternative ZERO_EXTRACT using bitfield constraints */
static void test_zero_extract_bitfield(volatile uint64_t *ptr)
{
    /* Constraint that suggests writing to specific bits */
    uint64_t mask = 0x00000000FFFFFFFFULL;
    __asm__ volatile (
        "and %0, %0, %1\n\t"
        "orr %0, %0, %2"
        : "+r" (*ptr)
        : "r" (~mask), "r" (0x87654321ULL)
        : "cc"
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint32_t *ptr)
{
    uint32_t input = 0x89ABCDEF;
    /* The 'q' modifier suggests quarter register access */
    __asm__ volatile (
        "add %0, %0, %1\n\t"
        "and %0, %0, #0xFFFF"
        : "+&r" (*ptr)  /* Early clobber to force specific register allocation */
        : "r" (input)
        : "cc"
    );
}

/* STRICT_LOW_PART with byte operation */
static void test_strict_low_part_byte(volatile uint8_t *ptr)
{
    uint8_t val = 0x7F;
    /* Operation that only affects low part */
    __asm__ volatile (
        "add %0, %0, %1\n\t"
        "and %0, %0, #0xFF"
        : "+&l" (*ptr)  /* 'l' for low register */
        : "r" (val)
        : "cc"
    );
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr)
{
    /* Access memory through different-sized pointer */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    volatile uint8_t *byte_ptr = (volatile uint8_t *)int_ptr;
    
    /* Write to sub-region of memory */
    __asm__ volatile (
        "strh %1, [%0, #2]\n\t"    /* Store halfword at offset 2 */
        "strb %2, [%0, #5]"        /* Store byte at offset 5 */
        : 
        : "r" (int_ptr), "r" ((uint16_t)0xAA55), 
          "r" ((uint8_t)0xCC)
        : "memory"
    );
}

/* SUBREG with complex memory addressing */
static void test_subreg_mem_complex(volatile uint32_t *array)
{
    /* Access array elements with type punning */
    uint32_t index = 3;
    __asm__ volatile (
        "ldrb w3, [%1, %2]\n\t"    /* Load byte through SUBREG */
        "add w3, w3, #1\n\t"
        "strb w3, [%1, %2]"
        : 
        : "r" (array), "r" (index)
        : "w3", "memory", "cc"
    );
}

/* Mixed pattern: SUBREG leading to MEM */
static void test_mixed_pattern(volatile uint64_t *ptr)
{
    uint32_t *sub_ptr = (uint32_t *)ptr;
    /* This should generate SUBREG then MEM access */
    __asm__ volatile (
        "ldr w0, [%0]\n\t"
        "add w0, w0, #0x100\n\t"
        "str w0, [%0]"
        : 
        : "r" (sub_ptr)
        : "w0", "memory", "cc"
    );
}

/* Driver function that exercises all patterns */
int main(void)
{
    uint32_t checksum = 0;
    
    printf("Testing ZERO_EXTRACT patterns...\n");
    test_zero_extract(&global_32, 8, 16);
    test_zero_extract_bitfield(&global_64);
    checksum += global_32 + (global_64 & 0xFFFFFFFF);
    
    printf("Testing STRICT_LOW_PART patterns...\n");
    test_strict_low_part(&global_32);
    test_strict_low_part_byte(&global_8);
    checksum += global_32 + global_8;
    
    printf("Testing SUBREG of MEM patterns...\n");
    test_subreg_mem(&global_32);
    test_subreg_mem_complex(mem_array);
    test_mixed_pattern(&global_64);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 16; i++) {
        checksum += mem_array[i];
    }
    checksum += global_32;
    checksum += (global_64 >> 32) + (global_64 & 0xFFFFFFFF);
    
    printf("Final checksum: 0x%08X\n", checksum);
    
    /* Additional test with loop to increase register pressure */
    volatile uint32_t loop_var = 0;
    for (int i = 0; i < 100; i++) {
        test_strict_low_part(&loop_var);
        if (i % 10 == 0) {
            test_subreg_mem(&mem_array[i % 8]);
        }
    }
    checksum += loop_var;
    
    return (int)(checksum & 0x7FFFFFFF);
}
