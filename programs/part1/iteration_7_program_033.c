#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint64_t global_64 = 0x123456789ABCDEF0ULL;
volatile uint32_t global_32 = 0xDEADBEEF;
volatile uint16_t global_16[4] = {0x1111, 0x2222, 0x3333, 0x4444};
volatile uint8_t global_8[8] = {1, 2, 3, 4, 5, 6, 7, 8};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *mem) {
    /* Bit-field assignment to specific bits of memory */
    __asm__ volatile (
        "mov %[val], %[src]\n\t"
        : [val] "=m" (*mem)  /* Memory destination */
        : [src] "r" (0xAA55AA55U)
        : "memory"
    );
    
    /* More complex: writing to specific bits using bit-field constraints */
    uint32_t temp;
    __asm__ volatile (
        "bfi %0, %1, #8, #8\n\t"  /* Bit-field insert: insert 8 bits at position 8 */
        : "=r" (temp)
        : "r" (0xFFU), "0" (*mem)
        : 
    );
    *mem = temp;
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *mem) {
    uint64_t val = *mem;
    
    /* Operation that only modifies low part of register */
    __asm__ volatile (
        "add %0, %0, %1\n\t"
        : "+&r" (val)        /* Early-clobber + register constraint */
        : "r" (0x1000ULL)
        : "cc"
    );
    
    /* Another pattern with explicit low-part modification */
    __asm__ volatile (
        "and %0, %0, %1\n\t"
        : "+&r" (val)
        : "r" (0x0000FFFFUL)  /* Only affects low 16 bits */
        : "cc"
    );
    
    *mem = val;
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *mem) {
    /* Access different-sized subregions of memory */
    volatile uint16_t *as_short = (volatile uint16_t *)mem;
    volatile uint8_t *as_byte = (volatile uint8_t *)mem;
    
    /* Write to 16-bit subregion of 32-bit memory */
    __asm__ volatile (
        "movw %0, %1\n\t"
        : "=m" (*as_short)
        : "r" ((uint16_t)0x1234)
        : "memory"
    );
    
    /* Write to 8-bit subregion with offset */
    __asm__ volatile (
        "movb %0, %1\n\t"
        : "=m" (as_byte[2])
        : "r" ((uint8_t)0xAB)
        : "memory"
    );
}

/* Additional test with array access and pointer casting */
static void test_complex_subreg(void) {
    /* Access array elements as different types */
    volatile uint32_t array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    
    /* Cast to different pointer types to force SUBREG MEM */
    volatile uint16_t *ptr16 = (volatile uint16_t *)&array[1];
    
    __asm__ volatile (
        "strh %1, [%0]\n\t"
        : 
        : "r" (ptr16), "r" ((uint16_t)0xAAAA)
        : "memory"
    );
    
    /* Access with offset */
    __asm__ volatile (
        "strb %1, [%0, #2]\n\t"
        : 
        : "r" (ptr16), "r" ((uint8_t)0xBB)
        : "memory"
    );
}

/* Test with packed structures to force bitfield operations */
struct __attribute__((packed)) packed_struct {
    uint32_t a;
    uint16_t b;
    uint8_t c;
};

static void test_packed_bitfield(void) {
    volatile struct packed_struct ps = {0x11223344, 0x5566, 0x77};
    
    /* Access bitfield within packed struct */
    uint32_t *ptr = (uint32_t *)&ps.b;  /* Misaligned access */
    
    __asm__ volatile (
        "ldr %0, [%1]\n\t"
        "bic %0, %0, #0xFF00\n\t"  /* Clear bits 8-15 */
        "str %0, [%1]\n\t"
        : "=&r" (*(volatile uint32_t *)ptr)
        : "r" (ptr)
        : "memory"
    );
}

int main(void) {
    uint64_t checksum = 0;
    
    printf("Testing ZERO_EXTRACT patterns...\n");
    test_zero_extract(&global_32);
    checksum += global_32;
    
    printf("Testing STRICT_LOW_PART patterns...\n");
    test_strict_low_part(&global_64);
    checksum += global_64;
    
    printf("Testing SUBREG of MEM patterns...\n");
    test_subreg_mem(&global_32);
    checksum += global_32;
    
    for (int i = 0; i < 4; i++) {
        checksum += global_16[i];
    }
    
    printf("Testing complex SUBREG patterns...\n");
    test_complex_subreg();
    
    printf("Testing packed bitfield patterns...\n");
    test_packed_bitfield();
    
    /* Use all variables to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        checksum += global_8[i];
    }
    
    printf("Checksum: 0x%016llX\n", (unsigned long long)checksum);
    
    return 0;
}
