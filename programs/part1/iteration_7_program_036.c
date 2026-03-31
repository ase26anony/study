#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Inline assembly that should generate ZERO_EXTRACT RTL */
    __asm__ volatile (
        /* Writing to specific bits (bits 8-15) of the destination */
        "btsl %1, %0\n\t"           /* Bit test and set */
        : "+m" (*dest)              /* Memory operand that will be modified */
        : "r" (src & 0x1F)          /* Bit position (0-31) */
        : "cc", "memory"
    );
    
    /* Another pattern: writing to a specific bit-field */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0xFF00, %%eax\n\t"   /* Extract bits 8-15 */
        "orl %%eax, %0\n\t"
        : "+m" (*dest)
        : "r" (src)
        : "%eax", "cc", "memory"
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *array, uint32_t idx) {
    uint16_t temp;
    
    /* Assembly that modifies only part of a register */
    __asm__ volatile (
        "movw (%1, %2, 2), %%ax\n\t"  /* Load array[idx] */
        "addw $0x1234, %%ax\n\t"      /* Modify only low 16 bits */
        "movw %%ax, %0\n\t"           /* Store back */
        : "=m" (array[idx])           /* Memory output */
        : "r" (array), "r" (idx)      /* Base and index */
        : "%ax", "cc", "memory"
    );
    
    /* Another pattern with early-clobber to force complex RTL */
    __asm__ volatile (
        "movl $0xABCD, %%eax\n\t"
        "movw %%ax, %0\n\t"           /* Only writes low 16 bits of eax */
        : "=m" (temp)
        :
        : "%eax", "memory"
    );
    
    array[idx % 8] = temp;
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint64_t *big_var) {
    /* Cast to access sub-region of memory */
    volatile uint16_t *as_short = (volatile uint16_t *)big_var;
    volatile uint8_t *as_byte = (volatile uint8_t *)big_var;
    
    /* Write to sub-region of memory through pointer casting */
    __asm__ volatile (
        "movw $0xDEAD, %0\n\t"
        : "=m" (as_short[1])         /* Write to bytes 2-3 of the 64-bit var */
        :
        : "memory"
    );
    
    /* Another pattern with offset */
    __asm__ volatile (
        "movb $0x42, %0\n\t"
        : "=m" (as_byte[5])          /* Write to byte 5 */
        :
        : "memory"
    );
    
    /* Complex pattern with multiple sub-register accesses */
    __asm__ volatile (
        "movq %1, %%rax\n\t"
        "movw %%ax, %0\n\t"          /* Write low 16 bits to memory */
        : "=m" (as_short[0])
        : "m" (*big_var)
        : "%rax", "memory"
    );
}

/* Additional test for bit-field operations that might generate ZERO_EXTRACT */
static void test_bitfield_ops(void) {
    struct bitfield_struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 20;
    } volatile bf;
    
    /* This might generate ZERO_EXTRACT when compiled */
    bf.field2 = 0xAB;
    
    /* Force inline assembly with bit-field like constraints */
    unsigned int temp = 0x12345678;
    __asm__ volatile (
        "rorl $8, %0\n\t"            /* Rotate - might create subreg patterns */
        : "+r" (temp)
        :
        : "cc"
    );
    
    /* Store rotated value into bitfield through memory */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0xFF, %%eax\n\t"      /* Extract low 8 bits */
        "movb %%al, %0\n\t"          /* Store as byte */
        : "=m" (*(volatile uint8_t*)&bf)
        : "r" (temp)
        : "%eax", "memory"
    );
}

/* Test with array indexing and complex addressing modes */
static void test_array_complex(volatile uint32_t *arr, int n) {
    for (int i = 0; i < n; i++) {
        /* Complex addressing that might generate SUBREG MEM patterns */
        __asm__ volatile (
            "imull $0x1234, %1, %%eax\n\t"
            "movw %%ax, %0\n\t"      /* Only write low 16 bits */
            : "=m" (*(volatile uint16_t*)((char*)arr + i * 4 + 1))
            : "r" (i)
            : "%eax", "cc", "memory"
        );
    }
}

int main(void) {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for coverage...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 10);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(global_short_array, 3);
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_long);
    checksum += (uint32_t)(global_long >> 32);
    checksum += (uint32_t)global_long;
    
    /* Test bitfield operations */
    test_bitfield_ops();
    
    /* Test complex array patterns */
    volatile uint32_t local_array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    test_array_complex(local_array, 4);
    for (int i = 0; i < 4; i++) {
        checksum += local_array[i];
    }
    
    /* Add byte array operations */
    for (int i = 0; i < 16; i++) {
        __asm__ volatile (
            "movb $0x%c[val], %0\n\t"
            : "=m" (global_byte_array[i])
            : [val] "i" ((i * 17) & 0xFF)
            : "memory"
        );
        checksum += global_byte_array[i];
    }
    
    /* Final operation that might generate interesting RTL */
    volatile uint32_t final_var = 0;
    __asm__ volatile (
        "movl $0x87654321, %%eax\n\t"
        "xchgl %%eax, %0\n\t"        /* Exchange - creates complex dependency */
        : "+m" (final_var)
        :
        : "%eax", "memory"
    );
    checksum += final_var;
    
    printf("Checksum: 0x%08X\n", checksum);
    return (int)(checksum & 0x7FFFFFFF); /* Return non-negative value */
}
