#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint64_t global_64 = 0x123456789ABCDEF0ULL;
volatile uint32_t global_32 = 0xDEADBEEF;
volatile uint16_t global_16[8] = {0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888};
volatile uint8_t global_8 = 0xAA;

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *mem, int bitpos, int bitsize)
{
    /* Use inline assembly with bit-field constraints to generate ZERO_EXTRACT */
    uint32_t temp;
    __asm__ volatile (
        /* Write to specific bits of memory location */
        "mov %[out], %[in]\n\t"
        : [out] "=m" (*mem)  /* Memory output */
        : [in] "r" (0x55AA55AA)  /* Input register */
        : "memory"
    );
    
    /* Another pattern with explicit bit field in constraint */
    __asm__ volatile (
        "bsf %1, %0\n\t"
        : "=r" (temp)
        : "m" (*mem)
        : "cc"
    );
    
    /* Complex pattern that might generate ZERO_EXTRACT */
    uint32_t mask = (1 << bitsize) - 1;
    __asm__ volatile (
        "and %[val], %[mask]\n\t"
        "ror %[val], %[pos]\n\t"
        : [val] "+r" (temp)
        : [mask] "r" (mask), [pos] "i" (bitpos)
        : "cc"
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *val)
{
    uint32_t low_part;
    uint64_t full_reg;
    
    /* Assembly that modifies only low part of register */
    __asm__ volatile (
        /* '&' for early clobber, 'r' constraint for register */
        "mov %[full], %[input]\n\t"
        "add $0x1001, %k[full]\n\t"  /* %k modifier for 32-bit register name */
        : [full] "=&r" (full_reg)
        : [input] "r" (*val)
        : "cc"
    );
    
    /* Another pattern with explicit low-part modification */
    __asm__ volatile (
        "inc %0\n\t"
        : "=r" (low_part)
        : "0" ((uint32_t)*val)
        : "cc"
    );
    
    /* Pattern that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "xor %%eax, %%eax\n\t"
        "mov %1, %%eax\n\t"
        "not %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r" (low_part)
        : "r" ((uint32_t)*val)
        : "%eax", "cc"
    );
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *ptr)
{
    volatile uint16_t *short_ptr = (volatile uint16_t *)ptr;
    volatile uint8_t *byte_ptr = (volatile uint8_t *)ptr;
    
    /* Write to sub-register of memory (16-bit access to 32-bit memory) */
    __asm__ volatile (
        "movw %[val], %[mem]\n\t"
        : [mem] "=m" (*short_ptr)
        : [val] "r" ((uint16_t)0xF00D)
        : "memory"
    );
    
    /* Access different sub-registers of the same memory */
    __asm__ volatile (
        "movb %[val], %[mem]\n\t"
        : [mem] "=m" (byte_ptr[1])  /* Access byte at offset 1 */
        : [val] "r" ((uint8_t)0x42)
        : "memory"
    );
    
    /* Complex pattern with multiple sub-register accesses */
    uint32_t temp32;
    __asm__ volatile (
        "movl %[src], %%eax\n\t"
        "movw %%ax, %[dst1]\n\t"
        "shrl $16, %%eax\n\t"
        "movb %%al, %[dst2]\n\t"
        : [dst1] "=m" (short_ptr[0]),
          [dst2] "=m" (byte_ptr[2]),
          "=a" (temp32)
        : [src] "r" (0x12345678)
        : "cc", "memory"
    );
}

/* Function combining multiple patterns */
static void test_combined_patterns(void)
{
    volatile uint64_t combined = 0;
    volatile uint32_t parts[4] = {0};
    
    /* Pattern that might generate multiple complex RTL expressions */
    __asm__ volatile (
        /* Multiple outputs with different sizes */
        "movq $0x1122334455667788, %[combined]\n\t"
        "movl $0xAABBCCDD, %[part0]\n\t"
        "movw $0xEEFF, %[part1]\n\t"
        "movb $0x11, %[part2]\n\t"
        : [combined] "=m" (combined),
          [part0] "=m" (parts[0]),
          [part1] "=m" (*(volatile uint16_t *)&parts[1]),
          [part2] "=m" (*(volatile uint8_t *)&parts[2])
        :
        : "memory"
    );
}

int main(void)
{
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for coverage...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_32, 8, 16);
    checksum += global_32;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_64);
    checksum += (uint32_t)global_64;
    checksum += (uint32_t)(global_64 >> 32);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_32);
    checksum += global_32;
    
    /* Test with array */
    test_subreg_mem(&global_16[2]);
    for (int i = 0; i < 8; i++) {
        checksum += global_16[i];
    }
    
    /* Test combined patterns */
    test_combined_patterns();
    
    /* Add global_8 to checksum */
    checksum += global_8;
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: 0x%08X\n", checksum);
    
    return 0;
}
