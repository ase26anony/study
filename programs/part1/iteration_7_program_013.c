#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_bytes[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *ptr, int bitpos, int bitsize)
{
    /* Use inline assembly with bit-field constraints */
    __asm__ volatile (
        "mov %[bitfield], %[value]\n\t"
        : [bitfield] "=r" (*ptr)
        : [value] "r" (0x55),
          "0" (*ptr)
        : "memory"
    );
    
    /* Another pattern using explicit bit operations */
    uint32_t mask = ((1 << bitsize) - 1) << bitpos;
    uint32_t cleared = *ptr & ~mask;
    uint32_t new_bits = (0xAA & ((1 << bitsize) - 1)) << bitpos;
    
    __asm__ volatile (
        "or %[dest], %[src]\n\t"
        : [dest] "+r" (cleared)
        : [src] "r" (new_bits)
        : "cc"
    );
    
    *ptr = cleared;
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *ptr)
{
    uint64_t temp;
    
    /* Assembly that only modifies low part of register */
    __asm__ volatile (
        "add %[low], %[inc]\n\t"
        : [low] "+&r" (temp)  /* Early clobber to force specific register allocation */
        : [inc] "ri" (0x10001)
        : "cc"
    );
    
    /* Another pattern with explicit low-part constraint */
    uint32_t low_part;
    __asm__ volatile (
        "mov %[out], %[in]\n\t"
        : [out] "=r" (low_part)
        : [in] "r" (temp & 0xFFFFFFFF)
        : 
    );
    
    *ptr = (temp & 0xFFFFFFFF) | ((*ptr) & 0xFFFFFFFF00000000ULL);
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *mem)
{
    /* Access different-sized subregions of memory */
    volatile uint16_t *as_short = (volatile uint16_t *)mem;
    volatile uint8_t *as_byte = (volatile uint8_t *)mem;
    
    /* Write to sub-region of larger memory object */
    __asm__ volatile (
        "movw %[val], (%[ptr])\n\t"
        : 
        : [val] "ri" (0xABCD), [ptr] "r" (as_short)
        : "memory"
    );
    
    /* Another access with offset */
    __asm__ volatile (
        "movb %[val], 2(%[ptr])\n\t"
        : 
        : [val] "ri" (0xEF), [ptr] "r" (as_byte)
        : "memory"
    );
}

/* Complex pattern combining multiple effects */
static void test_combined_pattern(volatile uint32_t *arr)
{
    /* Create a situation with register pressure and complex addressing */
    register uint32_t r1 asm ("r10") = arr[0];
    register uint32_t r2 asm ("r11") = arr[1];
    register uint32_t r3 asm ("r12") = arr[2];
    
    /* Multiple operations to create register pressure */
    __asm__ volatile (
        "add %[a], %[b]\n\t"
        "and %[c], %[mask]\n\t"
        "or %[d], %[e]\n\t"
        : [a] "+&r" (r1), [c] "+&r" (r2), [d] "+&r" (r3)
        : [b] "ri" (0x100), [mask] "ri" (0xFF00FF00), [e] "ri" (0x00FF00FF)
        : "cc"
    );
    
    /* Write back through potentially complex addressing mode */
    __asm__ volatile (
        "mov %[val1], (%[ptr1])\n\t"
        "mov %[val2], 4(%[ptr1])\n\t"
        "mov %[val3], 8(%[ptr1])\n\t"
        : 
        : [val1] "r" (r1), [val2] "r" (r2), [val3] "r" (r3),
          [ptr1] "r" (arr)
        : "memory"
    );
}

/* Test with bit-field structures */
struct bitfield_struct {
    uint32_t a : 4;
    uint32_t b : 8;
    uint32_t c : 12;
    uint32_t d : 8;
};

static void test_bitfield_struct(volatile struct bitfield_struct *bf)
{
    /* Operations on bit-fields often generate ZERO_EXTRACT */
    bf->b = 0xAA;
    bf->c = 0xBCD;
    
    /* Force memory access with inline assembly */
    __asm__ volatile (
        ""
        : "+m" (*bf)
        :
        : 
    );
}

int main(void)
{
    volatile uint32_t local_int = 0xDEADBEEF;
    volatile uint32_t local_array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    volatile struct bitfield_struct bf = {0};
    
    printf("Starting coverage test...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 8, 8);
    test_zero_extract(&local_int, 16, 12);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_long);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    test_subreg_mem(global_short_array);
    test_subreg_mem(global_bytes);
    
    /* Test combined patterns */
    test_combined_pattern(local_array);
    
    /* Test bit-field structures */
    test_bitfield_struct(&bf);
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    checksum += global_int;
    checksum += global_long & 0xFFFFFFFF;
    checksum += global_long >> 32;
    
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    for (int i = 0; i < 4; i++) {
        checksum += local_array[i];
    }
    
    checksum += *(volatile uint32_t*)&bf;
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    printf("Test completed.\n");
    
    return 0;
}
