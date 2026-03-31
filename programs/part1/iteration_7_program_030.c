#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to ensure side effects are preserved */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src)
{
    /* Target: SET_DEST with ZERO_EXTRACT wrapping MEM */
    __asm__ volatile (
        /* Write to specific bits (bits 8-15) of memory location */
        "btsl %1, %0\n\t"
        : "+m" (*dest)
        : "r" (src & 0xFF)
        : "cc", "memory"
    );
}

/* Another ZERO_EXTRACT variant with bit-field operation */
static void test_zero_extract_bitfield(volatile uint64_t *dest, uint32_t pos, uint32_t val)
{
    /* Extract and modify specific bit range */
    __asm__ volatile (
        "bfextu %1{%2, %3}, %0\n\t"
        : "=r" (val)
        : "m" (*dest), "r" (pos), "r" (8U)
        : "cc"
    );
    
    /* Write back modified value to same bit field */
    __asm__ volatile (
        "bfins %1, %0{%2, %3}\n\t"
        : "+m" (*dest)
        : "r" (val), "r" (pos), "r" (8U)
        : "cc", "memory"
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint32_t *dest, uint32_t src)
{
    uint32_t temp;
    
    /* Target: SET_DEST with STRICT_LOW_PART */
    __asm__ volatile (
        /* Operation that only affects low part of register */
        "addw %1, %0\n\t"
        : "=r" (temp)
        : "r" (src), "0" (*dest)
        : "cc"
    );
    
    /* Force early-clobber to complicate register allocation */
    __asm__ volatile (
        "mov {%1, %0|%0, %1}\n\t"
        : "=&r" (temp)
        : "r" (src + 1)
        : "cc"
    );
}

/* STRICT_LOW_PART with byte operation */
static void test_strict_low_part_byte(volatile uint8_t *dest, uint8_t src)
{
    /* Byte operation that implies only low part is modified */
    __asm__ volatile (
        "incb %0\n\t"
        : "+r" (src)
        :
        : "cc"
    );
    
    __asm__ volatile (
        "xchgb %0, %1\n\t"
        : "+r" (src), "+m" (*dest)
        :
        : "cc", "memory"
    );
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr, uint16_t src)
{
    /* Target: SET_DEST with SUBREG wrapping MEM */
    /* Access short within int through pointer casting */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        /* Write to sub-register of memory (16-bit within 32-bit) */
        "movw %1, %0\n\t"
        : "=m" (*short_ptr)
        : "r" (src)
        : "memory"
    );
}

/* More complex SUBREG pattern with array access */
static void test_subreg_mem_complex(volatile uint64_t *array, int index, uint32_t value)
{
    /* Access 32-bit portion of 64-bit memory location */
    volatile uint32_t *sub_ptr = (volatile uint32_t *)((char *)array + index * sizeof(uint64_t) + 2);
    
    __asm__ volatile (
        /* Unaligned write to create interesting SUBREG pattern */
        "movl %1, %0\n\t"
        : "=m" (*sub_ptr)
        : "r" (value)
        : "memory"
    );
}

/* Mixed pattern that might generate multiple interesting RTLs */
static void test_mixed_patterns(volatile uint64_t *dest, uint32_t src1, uint16_t src2)
{
    uint32_t temp1, temp2;
    
    /* Multiple inline asm statements to increase RTL complexity */
    __asm__ volatile (
        /* Potential ZERO_EXTRACT */
        "btcl %1, %0\n\t"
        : "+m" (*dest)
        : "r" (src1 & 0x1F)
        : "cc", "memory"
    );
    
    __asm__ volatile (
        /* Potential STRICT_LOW_PART */
        "subw %1, %0\n\t"
        : "=r" (temp1)
        : "r" (src2), "0" (0xFFFF)
        : "cc"
    );
    
    /* Access sub-register */
    volatile uint16_t *short_dest = (volatile uint16_t *)dest;
    __asm__ volatile (
        /* Potential SUBREG of MEM */
        "orw %1, %0\n\t"
        : "+m" (short_dest[1])
        : "r" (src2)
        : "cc", "memory"
    );
}

/* Main driver function */
int main(void)
{
    volatile uint32_t local_int = 0xDEADBEEF;
    volatile uint16_t local_short_array[4] = {0x1111, 0x2222, 0x3333, 0x4444};
    volatile uint64_t local_long_array[2] = {0xAAAABBBBCCCCDDDDULL, 0x1111222233334444ULL};
    
    uint32_t checksum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    test_zero_extract(&global_int, 0x10);
    test_zero_extract_bitfield(&global_long, 16, 0x55);
    
    /* Test STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    test_strict_low_part(&local_int, 0x1000);
    test_strict_low_part_byte(&global_byte_array[3], 0x42);
    
    /* Test SUBREG of MEM patterns */
    printf("Testing SUBREG of MEM patterns...\n");
    test_subreg_mem(&local_int, 0x8888);
    test_subreg_mem_complex(local_long_array, 0, 0x55555555);
    
    /* Test mixed patterns */
    printf("Testing mixed patterns...\n");
    test_mixed_patterns(&global_long, 0x1F, 0x1234);
    
    /* Calculate checksum to prevent dead code elimination */
    checksum += global_int;
    checksum += global_long & 0xFFFFFFFF;
    checksum += global_long >> 32;
    
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    for (int i = 0; i < 4; i++) {
        checksum += local_short_array[i];
    }
    
    checksum += local_int;
    checksum += local_long_array[0] & 0xFFFFFFFF;
    checksum += local_long_array[0] >> 32;
    checksum += local_long_array[1] & 0xFFFFFFFF;
    checksum += local_long_array[1] >> 32;
    
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    
    return (int)(checksum & 0xFF);
}
