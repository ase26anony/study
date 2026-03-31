#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src)
{
    /* Use bit-field constraints to generate ZERO_EXTRACT RTL */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=r" (*dest)  /* Output constraint for register */
        : "r" (src)
        : "memory"
    );
    
    /* Another pattern with explicit bit-field extraction */
    uint32_t temp;
    __asm__ volatile (
        "bfextu %0, %1, #8, #8\n\t"  /* Extract bits 8-15 */
        : "=r" (temp)
        : "r" (src)
    );
    
    /* Write to specific bits using bit-field assignment */
    __asm__ volatile (
        "bfi %0, %1, #4, #4\n\t"  /* Insert 4 bits at position 4 */
        : "+r" (*dest)
        : "r" (src & 0xF)
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint32_t src)
{
    /* Constraint that suggests only low part is modified */
    __asm__ volatile (
        "addw %0, %1, %2\n\t"  /* Add word (16-bit) operation */
        : "=r" (*dest)
        : "r" (*dest), "r" (src)
        : "cc"
    );
    
    /* Using early-clobber to force complex register allocation */
    uint32_t temp1, temp2;
    __asm__ volatile (
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        : "=&r" (temp1), "=&r" (temp2)  /* Early-clobber constraints */
        : "r" (src), "1" (*dest)
        : "cc"
    );
    
    /* Pattern that modifies only low 8 bits */
    __asm__ volatile (
        "incb %0\n\t"  /* Increment byte */
        : "+r" (*dest)
        :
        : "cc"
    );
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr, volatile uint16_t *short_ptr)
{
    /* Access memory through different-sized pointers */
    __asm__ volatile (
        "movw %0, %1\n\t"  /* Move word (16-bit) */
        : "=m" (*(volatile uint16_t *)int_ptr)  /* Cast to different size */
        : "r" (*short_ptr)
        : "memory"
    );
    
    /* Access sub-register of larger memory object */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)int_ptr;
    __asm__ volatile (
        "movb %0, %1\n\t"
        : "=m" (byte_ptr[1])  /* Access byte within int */
        : "r" ((uint8_t)(*short_ptr))
        : "memory"
    );
    
    /* Complex pattern with multiple memory accesses */
    uint32_t temp;
    __asm__ volatile (
        "movl %1, %0\n\t"
        "movw %2, %w0\n\t"  /* %w0 accesses lower 16 bits */
        : "=&r" (temp), "=m" (*int_ptr)
        : "m" (*(volatile uint16_t *)&int_ptr[1]), "0" (*short_ptr)
        : "memory"
    );
}

/* Additional test with array access */
static void test_array_subreg(volatile uint64_t *array)
{
    /* Access 32-bit portion of 64-bit array element */
    __asm__ volatile (
        "movl %0, %1\n\t"
        : "=m" (*(volatile uint32_t *)&array[0])  /* SUBREG of MEM */
        : "r" (0xDEADBEEF)
        : "memory"
    );
    
    /* Access 16-bit portion with offset */
    __asm__ volatile (
        "movw %0, %1\n\t"
        : "=m" (*(volatile uint16_t *)&array[1])
        : "r" ((uint16_t)0xCAFE)
        : "memory"
    );
}

/* Test with mixed types and complex constraints */
static void test_mixed_patterns(void)
{
    volatile struct {
        uint32_t a;
        uint16_t b;
        uint8_t c;
    } s = {0};
    
    /* Write to bit-field in structure (potential ZERO_EXTRACT) */
    __asm__ volatile (
        "bfi %0, %1, #0, #8\n\t"
        : "+m" (s)
        : "r" (0xAB)
        : "memory"
    );
    
    /* Access partial register with strict constraint */
    uint32_t reg;
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=r" (reg)
        : "m" (s.a)
    );
    
    /* Modify only part of memory through pointer */
    volatile uint8_t *ptr = (volatile uint8_t *)&s;
    __asm__ volatile (
        "incb %0\n\t"
        : "+m" (ptr[2])
        :
        : "cc", "memory"
    );
}

int main(void)
{
    uint32_t checksum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x87654321);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[0], 0x1001);
    checksum += global_short_array[0];
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int, &global_short_array[1]);
    checksum += global_int & 0xFFFF;
    
    /* Test array access patterns */
    test_array_subreg(&global_long);
    checksum += (global_long >> 32) + (global_long & 0xFFFFFFFF);
    
    /* Test mixed patterns */
    test_mixed_patterns();
    
    /* Access all global arrays to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    printf("Checksum: %u\n", checksum);
    return 0;
}
