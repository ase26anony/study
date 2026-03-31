#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint64_t global_64 = 0x123456789ABCDEF0ULL;
volatile uint32_t global_32 = 0xDEADBEEF;
volatile uint16_t global_16 = 0xCAFE;
volatile uint8_t global_8 = 0x42;
volatile uint32_t mem_buffer[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *ptr, int bitpos, int bitsize)
{
    /* Target: SET_DEST with ZERO_EXTRACT wrapping MEM */
    uint32_t temp;
    __asm__ volatile (
        /* Write to specific bits of memory location */
        "mov %[out], %[in]\n\t"
        : [out] "=m" (*ptr)  /* Memory output */
        : [in] "r" (0x55AA55AAU)
        : "memory"
    );
    
    /* Another pattern: bitfield insertion */
    __asm__ volatile (
        "btsl %[pos], %[out]\n\t"
        : [out] "+m" (*ptr)
        : [pos] "Ir" (bitpos)
        : "cc", "memory"
    );
    
    /* Complex constraint for bitfield */
    __asm__ volatile (
        "andl %[mask], %[out]\n\t"
        "orl %[val], %[out]\n\t"
        : [out] "+m" (*ptr)
        : [mask] "ri" (~(((1U << bitsize) - 1) << bitpos)),
          [val] "ri" ((0x37 & ((1U << bitsize) - 1)) << bitpos)
        : "cc", "memory"
    );
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *ptr)
{
    uint64_t result;
    
    /* Target: SET_DEST with STRICT_LOW_PART */
    __asm__ volatile (
        /* Operation that only affects low part */
        "addw $0x1234, %[out]\n\t"
        : [out] "=r" (result)
        : "0" (*ptr)
        : "cc"
    );
    
    /* Using 'q' modifier for byte-accessible register */
    __asm__ volatile (
        "incb %b[out]\n\t"
        : [out] "+r" (result)
        :
        : "cc"
    );
    
    /* Early clobber to force specific register allocation */
    uint32_t low_part;
    __asm__ volatile (
        "movl %[in], %[out]\n\t"
        "andl $0xFFFF, %[out]\n\t"
        : [out] "=&r" (low_part)
        : [in] "r" ((uint32_t)(*ptr))
        : "cc"
    );
    
    *ptr = result;
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *base_ptr, int offset)
{
    /* Target: SET_DEST with SUBREG wrapping MEM */
    volatile uint16_t *short_ptr = (volatile uint16_t *)((char *)base_ptr + offset);
    
    /* Write to sub-register of memory */
    __asm__ volatile (
        "movw %[val], %[mem]\n\t"
        : [mem] "=m" (*short_ptr)
        : [val] "ri" ((uint16_t)0xABCD)
        : "memory"
    );
    
    /* Different size access to same memory */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)((char *)base_ptr + offset + 2);
    __asm__ volatile (
        "movb %[val], %[mem]\n\t"
        : [mem] "=m" (*byte_ptr)
        : [val] "ri" ((uint8_t)0xEF)
        : "memory"
    );
    
    /* Using pointer casting in constraints */
    __asm__ volatile (
        "addw $1, %0\n\t"
        : "=m" (*(volatile uint16_t *)((char *)base_ptr + offset))
        : "m" (*(volatile uint16_t *)((char *)base_ptr + offset))
        : "memory"
    );
}

/* Additional test with array access */
static void test_array_subreg(void)
{
    volatile uint64_t array[4] = {0};
    
    /* Access different sized elements */
    __asm__ volatile (
        "movl $0x87654321, %[elem]\n\t"
        : [elem] "=m" (*(volatile uint32_t *)&array[1])
        :
        : "memory"
    );
    
    /* Access with offset */
    __asm__ volatile (
        "movw $0x1234, %[part]\n\t"
        : [part] "=m" (*(volatile uint16_t *)((char *)array + 6))
        :
        : "memory"
    );
}

/* Mixed pattern test */
static void test_mixed_patterns(void)
{
    volatile struct {
        uint32_t a;
        uint16_t b;
        uint8_t c;
    } s = {0};
    
    /* Multiple patterns in one function */
    __asm__ volatile (
        /* Potential STRICT_LOW_PART */
        "orb $0x0F, %b[val]\n\t"
        /* Followed by memory store with possible SUBREG */
        "movw %w[val], %[mem]\n\t"
        : [val] "+r" (global_16),
          [mem] "=m" (s.b)
        :
        : "cc", "memory"
    );
}

int main(void)
{
    uint32_t checksum = 0;
    
    printf("Testing ZERO_EXTRACT patterns...\n");
    test_zero_extract(&global_32, 8, 8);
    test_zero_extract(&mem_buffer[0], 16, 16);
    checksum += global_32;
    
    printf("Testing STRICT_LOW_PART patterns...\n");
    test_strict_low_part(&global_64);
    checksum += (uint32_t)global_64;
    
    printf("Testing SUBREG of MEM patterns...\n");
    test_subreg_mem(&global_32, 0);
    test_subreg_mem(mem_buffer, 4);
    checksum += global_32;
    
    printf("Testing array patterns...\n");
    test_array_subreg();
    
    printf("Testing mixed patterns...\n");
    test_mixed_patterns();
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 16; i++) {
        checksum += mem_buffer[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    return 0;
}
