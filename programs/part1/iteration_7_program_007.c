#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_bytes[16] = {0};

/* Structure for bit-field testing */
struct bitfield_struct {
    volatile uint32_t field1;
    volatile uint32_t field2;
};

static struct bitfield_struct bitfields = {0xAAAAAAAA, 0x55555555};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Attempt to generate ZERO_EXTRACT by writing to specific bits */
    __asm__ volatile (
        /* Try to write to bits 8-15 of dest (8-bit field starting at bit 8) */
        "btsl %1, %0\n\t"          /* Bit test and set - may generate ZERO_EXTRACT */
        : "+m" (*dest)             /* Memory operand that might be extracted */
        : "r" (src & 0xFF)         /* Source value constrained to low 8 bits */
        : "cc", "memory"
    );
    
    /* Another attempt with explicit bit-field operation */
    uint32_t mask = 0xFF00;  /* Bits 8-15 */
    __asm__ volatile (
        "andl %1, %0\n\t"          /* Clear other bits */
        "orl %2, %0\n\t"           /* Set specific bits */
        : "+m" (*dest)
        : "r" (~mask), "r" ((src << 8) & mask)
        : "cc", "memory"
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint32_t *dest, uint32_t src) {
    /* Use constraints that suggest only low part is modified */
    uint32_t temp;
    
    __asm__ volatile (
        /* 'q' constraint for byte-addressable register, hinting at partial modification */
        "mov{l} {%1, %0 | %0, %1}\n\t"
        : "=q" (temp)              /* Byte register constraint */
        : "r" (src)
        : /* No clobbers - trying to get STRICT_LOW_PART */
    );
    
    /* Force write-back with early clobber to complicate allocation */
    __asm__ volatile (
        "add{l} {%1, %0 | %0, %1}\n\t"
        : "+&r" (temp)             /* Early clobber on register */
        : "r" (src & 0xFFFF)       /* Only low 16 bits matter */
        : "cc"
    );
    
    *dest = temp;
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *mem) {
    /* Access memory through different-sized views */
    volatile uint32_t *as_int = (volatile uint32_t *)mem;
    volatile uint16_t *as_short = (volatile uint16_t *)mem;
    volatile uint8_t *as_byte = (volatile uint8_t *)mem;
    
    /* Write to sub-register of memory location */
    __asm__ volatile (
        "mov{w} {%1, %0 | %0, %1}\n\t"
        : "=m" (*as_short)         /* 16-bit memory access */
        : "r" ((uint16_t)0xDEAD)
        : "memory"
    );
    
    /* Access different offset within the same memory object */
    __asm__ volatile (
        "mov{b} {%1, %0 | %0, %1}\n\t"
        : "=m" (as_byte[2])        /* Byte at offset 2 */
        : "r" ((uint8_t)0xBE)
        : "memory"
    );
    
    /* Complex pattern with pointer arithmetic */
    uint32_t offset = 4;
    __asm__ volatile (
        "mov{l} {%1, %0(%2) | %0(%2), %1}\n\t"
        : "=m" (*(volatile uint32_t *)((char *)mem + offset))
        : "r" (0xCAFEBABE), "r" (mem)
        : "memory"
    );
}

/* Function that combines multiple patterns */
static void test_combined_patterns(void) {
    volatile uint32_t local_var = 0;
    volatile uint64_t local_long = 0;
    
    /* Mix ZERO_EXTRACT and SUBREG patterns */
    __asm__ volatile (
        /* Complex multi-operation sequence */
        "mov{l} {%1, %0\n\t"
        "shr{l} $16, %0\n\t"
        "mov{w} %w0, %2\n\t"
        : "=&r" (local_var),        /* Early clobber register */
          "+m" (global_int)         /* Memory operand */
        : "m" (global_short_array[0]) /* Another memory operand */
        : "cc", "memory"
    );
    
    /* Access part of a larger variable */
    __asm__ volatile (
        "lea{q} %1, %%rax\n\t"
        "mov{w} $0x1234, (%%rax)\n\t"  /* Write to low 16 bits */
        : "=m" (local_long)          /* Whole variable as memory */
        : "m" (local_long)           /* Input also */
        : "rax", "cc", "memory"
    );
}

/* Main driver function */
int main(void) {
    uint32_t checksum = 0;
    
    printf("Starting coverage test for resource.cc lines 282-290\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x89ABCDEF);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_int, 0x87654321);
    checksum += global_int;
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_long);
    checksum += (uint32_t)global_long;
    checksum += (uint32_t)(global_long >> 32);
    
    /* Test with array */
    test_subreg_mem(global_bytes);
    for (int i = 0; i < 16; i++) {
        checksum += global_bytes[i];
    }
    
    /* Test combined patterns */
    test_combined_patterns();
    checksum += global_int;
    checksum += global_short_array[0];
    
    /* Test bitfield structure */
    test_zero_extract(&bitfields.field1, 0xF0F0F0F0);
    test_subreg_mem(&bitfields.field2);
    checksum += bitfields.field1;
    checksum += bitfields.field2;
    
    /* Force use of all global variables */
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed - compile with flags to trigger RTL generation\n");
    
    return (int)(checksum & 0x7FFFFFFF); /* Return non-negative value */
}
