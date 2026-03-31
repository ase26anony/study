#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_bytes[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src)
{
    /* Force ZERO_EXTRACT by writing to specific bit-field */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*dest)
        : [src] "r" (src)
        : "memory"
    );
    
    /* Alternative with explicit bit-field constraint */
    uint32_t temp;
    __asm__ volatile (
        "bfi %0, %1, #8, #8\n\t"  /* Bit-field insert: bits 8-15 */
        : "=r" (temp)
        : "r" (src)
        : "cc"
    );
    *dest = temp;
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *dest, uint32_t src)
{
    /* Force STRICT_LOW_PART by modifying only part of register */
    uint64_t temp = *dest;
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        : "=&r" (temp)      /* Early clobber to force specific reg allocation */
        : "r" (temp), "r" ((uint64_t)src)
        : "cc"
    );
    *dest = temp;
    
    /* Another pattern that might generate STRICT_LOW_PART */
    uint32_t low_part;
    __asm__ volatile (
        "and %0, %1, #0xFF\n\t"  /* Only modify low 8 bits */
        : "=r" (low_part)
        : "r" (src)
        : "cc"
    );
    *dest = (*dest & 0xFFFFFFFFFFFFFF00ULL) | low_part;
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *mem)
{
    /* Access memory through different-sized views */
    volatile uint32_t *as_int = (volatile uint32_t *)mem;
    volatile uint16_t *as_short = (volatile uint16_t *)mem;
    volatile uint8_t *as_byte = (volatile uint8_t *)mem;
    
    /* Write to sub-register of memory location */
    __asm__ volatile (
        "strh %1, [%0]\n\t"      /* Store halfword to memory */
        : 
        : "r" (as_short), "r" ((uint16_t)0xABCD)
        : "memory"
    );
    
    /* Another SUBREG memory access */
    __asm__ volatile (
        "strb %1, [%0, #1]\n\t"  /* Store byte with offset */
        : 
        : "r" (as_byte), "r" ((uint8_t)0xEF)
        : "memory"
    );
    
    /* Complex pattern with multiple sub-register accesses */
    uint32_t combined;
    __asm__ volatile (
        "ldrb %0, [%1]\n\t"      /* Load byte */
        "ldrb %2, [%1, #1]\n\t"  /* Load another byte */
        "orr %0, %0, %2, lsl #8\n\t"  /* Combine into halfword */
        : "=&r" (combined), "+r" (as_byte)
        : "r" (as_byte)
        : "memory", "cc"
    );
}

/* Function to test mixed patterns */
static void test_mixed_patterns(void)
{
    volatile struct {
        uint32_t a;
        uint16_t b;
        uint8_t c;
    } s = {0};
    
    /* This should generate complex MEM access patterns */
    __asm__ volatile (
        "str %1, [%0]\n\t"       /* Store word to s.a */
        "strh %2, [%0, #4]\n\t"  /* Store halfword to s.b */
        "strb %3, [%0, #6]\n\t"  /* Store byte to s.c */
        : 
        : "r" (&s), "r" (0xDEADBEEF), "r" ((uint16_t)0xCAFE), 
          "r" ((uint8_t)0x42)
        : "memory"
    );
}

/* Function to test bit-field extraction with memory */
static void test_bitfield_mem(volatile uint32_t *mem)
{
    uint32_t result;
    
    /* Extract bit-field from memory */
    __asm__ volatile (
        "ldr %0, [%1]\n\t"
        "ubfx %0, %0, #4, #12\n\t"  /* Extract bits 4-15 */
        : "=r" (result)
        : "r" (mem)
        : "memory"
    );
    
    /* Insert bit-field to memory */
    __asm__ volatile (
        "ldr %0, [%1]\n\t"
        "bfi %0, %2, #8, #8\n\t"    /* Insert bits 8-15 */
        "str %0, [%1]\n\t"
        : 
        : "r" (mem), "r" (0xFF)
        : "memory", "r0"
    );
}

int main(void)
{
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for coverage...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x87654321);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_long, 0x12345678);
    checksum += (global_long & 0xFFFFFFFF) + (global_long >> 32);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(global_short_array);
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    /* Test mixed patterns */
    test_mixed_patterns();
    
    /* Test bit-field memory operations */
    test_bitfield_mem(&global_int);
    checksum += global_int;
    
    /* Access all global variables to prevent dead code elimination */
    for (int i = 0; i < 16; i++) {
        checksum += global_bytes[i];
    }
    
    printf("Checksum: %u\n", checksum);
    printf("All pattern tests completed.\n");
    
    return 0;
}
