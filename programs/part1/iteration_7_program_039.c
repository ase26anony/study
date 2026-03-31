#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint64_t global_64 = 0x123456789ABCDEF0ULL;
volatile uint32_t global_32 = 0xDEADBEEF;
volatile uint16_t global_16[8] = {0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888};
volatile uint8_t global_8[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *var) {
    /* Write to specific bit-field (bits 8-15) using inline assembly */
    __asm__ volatile (
        "mov %[out], %[in]\n\t"
        : [out] "=r" (*var)
        : [in] "r" (0x00FF0000U)
        : "memory"
    );
    
    /* Another pattern: bit-field assignment with specific constraints */
    uint32_t temp = 0x12345678;
    __asm__ volatile (
        "bts %0, %1\n\t"
        : "+r" (temp)
        : "i" (16)  /* bit position 16 */
        : "cc"
    );
    
    /* Complex bit-field manipulation */
    __asm__ volatile (
        "bfi %0, %1, #8, #8\n\t"  /* Insert bits 8-15 from source */
        : "+r" (*var)
        : "r" (0xAA55AA55U)
        : "cc"
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *var) {
    uint64_t input = 0xFFFFFFFF00000000ULL;
    
    /* Operation that only modifies low 32 bits */
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        : "=&r" (*var)  /* Early clobber to force specific register allocation */
        : "r" (input), "r" (0x00000000FFFFFFFFULL)
        : "cc"
    );
    
    /* Another pattern: operation on low part only */
    __asm__ volatile (
        "and %0, %1, %2\n\t"
        : "=r" (*var)
        : "r" (0x123456789ABCDEF0ULL), "r" (0x00000000FFFFFFFFULL)
        : "cc"
    );
    
    /* Using specific register constraints for partial modification */
    register uint32_t low_part asm("eax");
    low_part = 0xDEADBEEF;
    __asm__ volatile (
        "or %0, %1\n\t"
        : "+&r" (low_part)  /* Early clobber on partial register */
        : "r" (0x00FF00FFU)
        : "cc"
    );
    *var = low_part;
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *mem) {
    /* Write to 16-bit sub-register of 32-bit memory location */
    __asm__ volatile (
        "movw %0, %1\n\t"
        : "=m" (*(volatile uint16_t *)mem)
        : "r" ((uint16_t)0xABCD)
        : "memory"
    );
    
    /* Access different sized sub-parts of memory */
    volatile uint32_t *ptr32 = (volatile uint32_t *)mem;
    __asm__ volatile (
        "strh %1, [%0, #2]\n\t"  /* Store halfword at offset 2 */
        : 
        : "r" (ptr32), "r" ((uint16_t)0x1234)
        : "memory"
    );
    
    /* Complex memory access with pointer arithmetic */
    volatile uint8_t *ptr8 = (volatile uint8_t *)mem;
    __asm__ volatile (
        "strb %1, [%0, #3]\n\t"
        :
        : "r" (ptr8), "r" ((uint8_t)0x99)
        : "memory"
    );
}

/* Function combining multiple patterns */
static void test_combined_patterns(void) {
    volatile uint32_t combined = 0;
    
    /* Create SUBREG -> MEM pattern */
    __asm__ volatile (
        "ldrh %0, [%1]\n\t"
        : "=r" (combined)
        : "r" (&global_16[2])
        : "memory"
    );
    
    /* Follow with ZERO_EXTRACT operation */
    __asm__ volatile (
        "ubfx %0, %1, #4, #12\n\t"  /* Extract bits 4-15 */
        : "=r" (combined)
        : "r" (combined)
        : "cc"
    );
}

/* Function with memory operand that should generate SUBREG(MEM) */
static void test_memory_subreg(void) {
    /* Cast to access sub-register of larger type */
    volatile uint16_t *as_short = (volatile uint16_t *)&global_32;
    
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=m" (*as_short)  /* 16-bit memory access to 32-bit location */
        : "r" ((uint16_t)0xCAFE)
        : "memory"
    );
    
    /* Another pattern with offset */
    __asm__ volatile (
        "strh %1, [%0]\n\t"
        :
        : "r" (&global_32), "r" ((uint16_t)0xBEEF)
        : "memory"
    );
}

/* Function to test bit-field operations on memory */
static void test_bitfield_memory(void) {
    volatile struct {
        uint32_t field1 : 8;
        uint32_t field2 : 16;
        uint32_t field3 : 8;
    } bitfield = {0};
    
    /* This should generate ZERO_EXTRACT on MEM */
    __asm__ volatile (
        "bfi %0, %1, #8, #16\n\t"  /* Insert into bits 8-23 */
        : "+m" (bitfield)
        : "r" (0x1234U)
        : "cc", "memory"
    );
}

int main(void) {
    uint64_t checksum = 0;
    
    printf("Testing RTL pattern generation for coverage...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_32);
    checksum += global_32;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_64);
    checksum += global_64;
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_32);
    checksum += global_32;
    
    test_memory_subreg();
    checksum += global_32;
    
    /* Test combined patterns */
    test_combined_patterns();
    checksum += global_32;
    
    /* Test bitfield memory operations */
    test_bitfield_memory();
    
    /* Access array elements to generate various memory patterns */
    for (int i = 0; i < 8; i++) {
        volatile uint16_t *elem = &global_16[i];
        __asm__ volatile (
            "add %0, %1, %2\n\t"
            : "=m" (*elem)
            : "r" ((uint16_t)i), "i" (0x1000)
            : "cc", "memory"
        );
        checksum += *elem;
    }
    
    /* Final memory pattern with complex addressing */
    volatile uint32_t local_var = 0;
    __asm__ volatile (
        "stm %0, {%1, %2}\n\t"  /* Store multiple registers */
        :
        : "r" (&local_var), "r" (0x11111111U), "r" (0x22222222U)
        : "memory"
    );
    checksum += local_var;
    
    printf("Checksum: 0x%016llX\n", (unsigned long long)checksum);
    printf("All pattern tests completed.\n");
    
    return 0;
}
